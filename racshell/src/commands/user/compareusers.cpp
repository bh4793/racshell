#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

#include "sear/sear.h"
#include "include/racshell/command_helper.hpp"
#include "racshell/output_formatter.hpp"

namespace
{

    struct RawUserResponse
    {
        std::string userid;
        std::string result_json;
    };

    struct ExtractedUser
    {
        std::string userid;
        nlohmann::json base;
        nlohmann::json profile;
    };

    nlohmann::json get_object_value(const nlohmann::json &object, const char *key);

    void add_difference(nlohmann::json &differences,
                        const char *label,
                        const nlohmann::json &left,
                        const nlohmann::json &right);

    void add_list_difference(nlohmann::json &differences,
                             const char *label,
                             const std::vector<std::string> &left,
                             const std::vector<std::string> &right);

    RawUserResponse execute_user_extract(const std::string &userid, bool debug)
    {
        const nlohmann::json request = {
            {"operation", "extract"},
            {"admin_type", "user"},
            {"userid", userid}};

        const std::string request_json = request.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

        RawUserResponse response = {.userid = userid};
        if (result != nullptr && result->result_json != nullptr)
        {
            response.result_json = result->result_json;
        }
        return response;
    }

    bool parse_user_extract(const RawUserResponse &raw_response,
                            ExtractedUser &user,
                            std::string &error_message)
    {
        if (raw_response.result_json.empty())
        {
            error_message = "No response returned for user " + raw_response.userid;
            return false;
        }

        const racshell::SearResponseInfo info =
            racshell::validate_sear_response(raw_response.result_json.c_str(), "user");
        if (!info.success)
        {
            error_message = info.error_message;
            return false;
        }

        user.userid = raw_response.userid;
        user.base = info.base;
        user.profile = info.profile;
        return true;
    }

    std::vector<std::string> extract_groups(const nlohmann::json &base)
    {
        std::vector<std::string> groups;
        if (!base.contains("base:group_connections") || !base["base:group_connections"].is_array())
        {
            return groups;
        }

        for (const auto &group : base["base:group_connections"])
        {
            if (!group.contains("base:group_connection_group"))
            {
                continue;
            }

            const std::string groupid = racshell::value_to_text(group["base:group_connection_group"]);
            if (!groupid.empty() && groupid != "<missing>")
            {
                groups.push_back(groupid);
            }
        }

        std::sort(groups.begin(), groups.end());
        groups.erase(std::unique(groups.begin(), groups.end()), groups.end());
        return groups;
    }

    nlohmann::json build_security_snapshot(const nlohmann::json &base, const nlohmann::json &profile)
    {
        nlohmann::json security = nlohmann::json::object();

        const std::vector<std::string> base_security_keys = {
            "base:resume_date",
            "base:password_change_date",
            "base:password_change_interval",
            "base:password_expired",
            "base:passphrase_change_date",
            "base:passphrase_change_interval",
            "base:has_passphrase",
            "base:has_password",
            "base:special",
            "base:operations",
            "base:auditor",
            "base:read_only_auditor",
            "base:protected",
            "base:restrict_global_access_checking",
            "base:last_access_date",
            "base:installation_data"};

        for (const auto &key : base_security_keys)
        {
            const auto element = base.find(key);
            if (element != base.end())
            {
                security[key] = *element;
            }
        }

        if (profile.contains("mfa"))
        {
            security["mfa"] = profile["mfa"];
        }

        return security;
    }

    UserComparisonData build_user_comparison_data(const std::string &left_userid,
                                                  const std::string &right_userid,
                                                  bool debug,
                                                  bool raw_json_output)
    {
        UserComparisonData comparison;
        comparison.left.userid = left_userid;
        comparison.right.userid = right_userid;
        comparison.raw_json_output = raw_json_output;

        const RawUserResponse left_raw = execute_user_extract(left_userid, debug);
        const RawUserResponse right_raw = execute_user_extract(right_userid, debug);
        comparison.left.raw_response_json = left_raw.result_json;
        comparison.right.raw_response_json = right_raw.result_json;

        if (raw_json_output)
        {
            nlohmann::json left_response;
            std::string parse_error;
            if (racshell::parse_sear_response_json(left_raw.result_json.c_str(), left_response, parse_error))
            {
                comparison.left.response_json = left_response;
            }

            nlohmann::json right_response;
            if (racshell::parse_sear_response_json(right_raw.result_json.c_str(), right_response, parse_error))
            {
                comparison.right.response_json = right_response;
            }

            return comparison;
        }

        ExtractedUser left_user;
        ExtractedUser right_user;
        std::string error_message;
        if (!parse_user_extract(left_raw, left_user, error_message))
        {
            throw std::runtime_error(left_userid + ": " + error_message);
        }

        if (!parse_user_extract(right_raw, right_user, error_message))
        {
            throw std::runtime_error(right_userid + ": " + error_message);
        }

        add_difference(comparison.differences, "name",
                       get_object_value(left_user.base, "base:name"),
                       get_object_value(right_user.base, "base:name"));
        add_difference(comparison.differences, "owner",
                       get_object_value(left_user.base, "base:owner"),
                       get_object_value(right_user.base, "base:owner"));
        add_difference(comparison.differences, "default_group",
                       get_object_value(left_user.base, "base:default_group"),
                       get_object_value(right_user.base, "base:default_group"));
        add_difference(comparison.differences, "created_date",
                       get_object_value(left_user.base, "base:create_date"),
                       get_object_value(right_user.base, "base:create_date"));
        add_difference(comparison.differences, "revoked",
                       get_object_value(left_user.base, "base:revoked"),
                       get_object_value(right_user.base, "base:revoked"));

        add_list_difference(comparison.differences,
                            "groups",
                            extract_groups(left_user.base),
                            extract_groups(right_user.base));

        add_difference(comparison.differences, "security",
                       build_security_snapshot(left_user.base, left_user.profile),
                       build_security_snapshot(right_user.base, right_user.profile));
        add_difference(comparison.differences, "tso",
                       get_object_value(left_user.profile, "tso"),
                       get_object_value(right_user.profile, "tso"));
        add_difference(comparison.differences, "omvs",
                       get_object_value(left_user.profile, "omvs"),
                       get_object_value(right_user.profile, "omvs"));
        add_difference(comparison.differences, "kerberos",
                       get_object_value(left_user.profile, "kerberos"),
                       get_object_value(right_user.profile, "kerberos"));
        add_difference(comparison.differences, "cics",
                       get_object_value(left_user.profile, "cics"),
                       get_object_value(right_user.profile, "cics"));
        add_difference(comparison.differences, "csdata",
                       get_object_value(left_user.profile, "csdata"),
                       get_object_value(right_user.profile, "csdata"));

        comparison.identical = comparison.differences.empty();
        return comparison;
    }

    nlohmann::json get_object_value(const nlohmann::json &object, const char *key)
    {
        const auto it = object.find(key);
        if (it == object.end())
        {
            return nullptr;
        }
        return *it;
    }

    void add_difference(nlohmann::json &differences,
                        const char *label,
                        const nlohmann::json &left,
                        const nlohmann::json &right)
    {
        if (left != right)
        {
            differences[label] = {
                {"left", left},
                {"right", right}};
        }
    }

    void add_list_difference(nlohmann::json &differences,
                             const char *label,
                             const std::vector<std::string> &left,
                             const std::vector<std::string> &right)
    {
        if (left == right)
        {
            return;
        }

        std::set<std::string> left_set(left.begin(), left.end());
        std::set<std::string> right_set(right.begin(), right.end());

        nlohmann::json only_in_left = nlohmann::json::array();
        nlohmann::json only_in_right = nlohmann::json::array();

        for (const auto &group : left_set)
        {
            if (right_set.find(group) == right_set.end())
            {
                only_in_left.push_back(group);
            }
        }

        for (const auto &group : right_set)
        {
            if (left_set.find(group) == left_set.end())
            {
                only_in_right.push_back(group);
            }
        }

        differences[label] = {
            {"only_in_left", only_in_left},
            {"only_in_right", only_in_right}};
    }

} // namespace

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("compareusers");

    program.add_argument("left-user")
        .help("first RACF user to compare");

    program.add_argument("right-user")
        .help("second RACF user to compare");

    racshell::add_no_color_argument(program);
    racshell::add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
    racshell::add_toggle_argument(program, "-j", "--json", "output as JSON");
    racshell::add_toggle_argument(program, "-a", "--all-json", "output full raw SEAR JSON responses");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &err)
    {
        racshell::print_error(std::cerr, err.what());
        std::cerr << program;
        return 1;
    }

    racshell::set_color_output_enabled(!program.get<bool>("no-color"));

    const std::string left_userid = program.get<std::string>("left-user");
    const std::string right_userid = program.get<std::string>("right-user");
    if (left_userid.length() > 8 || right_userid.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF userid");
        return 1;
    }

    const bool debug = program.get<bool>("debug");
    const bool json_output = program.get<bool>("json");
    const bool all_json = program.get<bool>("all-json");

    try
    {
        const UserComparisonData comparison = build_user_comparison_data(left_userid,
                                                                         right_userid,
                                                                         debug,
                                                                         all_json);

        if (json_output)
        {
            JsonFormatter formatter;
            std::cout << formatter.format(comparison);
            return 0;
        }

        TextFormatter formatter;
        std::cout << formatter.format(comparison);
        return 0;
    }
    catch (const std::exception &err)
    {
        racshell::print_error(std::cerr, err.what());
        return 1;
    }
}