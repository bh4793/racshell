#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "sear/sear.h"
#include "include/racshell/command_helper.hpp"

namespace
{

    struct RawGroupResponse
    {
        std::string groupid;
        std::string result_json;
    };

    struct ExtractedGroup
    {
        std::string groupid;
        nlohmann::json profile;
        nlohmann::json base;
    };

    RawGroupResponse execute_group_extract(const std::string &groupid, bool debug)
    {
        const nlohmann::json request = {
            {"operation", "extract"},
            {"admin_type", "group"},
            {"group", groupid}};

        const std::string request_json = request.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

        RawGroupResponse response = {.groupid = groupid};
        if (result != nullptr && result->result_json != nullptr)
        {
            response.result_json = result->result_json;
        }
        return response;
    }

    bool parse_group_extract(const RawGroupResponse &raw_response,
                             ExtractedGroup &group,
                             std::string &error_message)
    {
        if (raw_response.result_json.empty())
        {
            error_message = "No response returned for group " + raw_response.groupid;
            return false;
        }

        const racshell::SearResponseInfo info =
            racshell::validate_sear_response(raw_response.result_json.c_str(), "group");
        if (!info.success)
        {
            error_message = info.error_message;
            return false;
        }

        group.groupid = raw_response.groupid;
        group.profile = info.profile;
        group.base = info.base;
        return true;
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

    /**
     * @brief Converts connected-user JSON entries into a userid->authority map.
     * @param users JSON array containing group connection entries.
     * @return Map keyed by userid with authority as value.
     */
    inline std::map<std::string, std::string> to_user_map(const nlohmann::json &users)
    {
        std::map<std::string, std::string> mapped_users;
        if (!users.is_array())
        {
            return mapped_users;
        }

        for (const auto &user : users)
        {
            if (!user.is_object())
            {
                continue;
            }

            const auto userid_it = user.find("base:connected_userid");
            if (userid_it == user.end() || !userid_it->is_string())
            {
                continue;
            }

            std::string authority;
            const auto authority_it = user.find("base:connected_user_authority");
            if (authority_it != user.end() && authority_it->is_string())
            {
                authority = authority_it->get<std::string>();
            }

            mapped_users[userid_it->get<std::string>()] = authority;
        }

        return mapped_users;
    }

    void add_connected_user_differences(const ExtractedGroup &left_group,
                                        const ExtractedGroup &right_group,
                                        nlohmann::json &differences)
    {
        const std::map<std::string, std::string> left_users = to_user_map(get_object_value(left_group.base, "base:connected_users"));
        const std::map<std::string, std::string> right_users = to_user_map(get_object_value(right_group.base, "base:connected_users"));

        nlohmann::json only_in_left = nlohmann::json::array();
        nlohmann::json only_in_right = nlohmann::json::array();
        nlohmann::json authority_mismatches = nlohmann::json::array();

        for (const auto &[userid, authority] : left_users)
        {
            const auto right_it = right_users.find(userid);
            if (right_it == right_users.end())
            {
                only_in_left.push_back({
                    {"userid", userid},
                    {"authority", authority}});
                continue;
            }

            if (authority != right_it->second)
            {
                authority_mismatches.push_back({
                    {"userid", userid},
                    {"left", authority},
                    {"right", right_it->second}});
            }
        }

        for (const auto &[userid, authority] : right_users)
        {
            if (left_users.find(userid) == left_users.end())
            {
                only_in_right.push_back({
                    {"userid", userid},
                    {"authority", authority}});
            }
        }

        if (!only_in_left.empty() || !only_in_right.empty() || !authority_mismatches.empty())
        {
            differences["connected_users"] = nlohmann::json::object();
            if (!only_in_left.empty())
            {
                differences["connected_users"]["only_in_left"] = only_in_left;
            }
            if (!only_in_right.empty())
            {
                differences["connected_users"]["only_in_right"] = only_in_right;
            }
            if (!authority_mismatches.empty())
            {
                differences["connected_users"]["authority_mismatches"] = authority_mismatches;
            }
        }
    }

    void print_user_list(const std::string &heading, const nlohmann::json &users)
    {
        std::cout << heading << "\n";
        for (const auto &user : users)
        {
            std::cout << "    " << user.value("userid", "")
                      << " (" << user.value("authority", "") << ")\n";
        }
    }

} // namespace

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("comparegroups");

    program.add_argument("left-group")
        .help("first RACF group to compare");

    program.add_argument("right-group")
        .help("second RACF group to compare");

    racshell::add_toggle_argument(program, "-u", "--users", "compare connected users");
    racshell::add_toggle_argument(program, "-o", "--omvs", "compare OMVS segment");
    racshell::add_toggle_argument(program, "-x", "--csdata", "compare CSDATA segment");
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

    const std::string left_groupid = program.get<std::string>("left-group");
    const std::string right_groupid = program.get<std::string>("right-group");
    if (left_groupid.length() > 8 || right_groupid.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF group name");
        return 1;
    }

    const bool compare_users = program.get<bool>("users");
    const bool compare_omvs = program.get<bool>("omvs");
    const bool compare_csdata = program.get<bool>("csdata");
    const bool debug = program.get<bool>("debug");
    const bool json_output = program.get<bool>("json");
    const bool all_json = program.get<bool>("all-json");

    const RawGroupResponse left_raw = execute_group_extract(left_groupid, debug);
    const RawGroupResponse right_raw = execute_group_extract(right_groupid, debug);

    if (all_json)
    {
        nlohmann::json output = {
            {"left_group", left_groupid},
            {"right_group", right_groupid}};

        nlohmann::json left_response;
        std::string parse_error;
        if (racshell::parse_sear_response_json(left_raw.result_json.c_str(), left_response, parse_error))
        {
            output["left_response"] = left_response;
        }
        else
        {
            output["left_response_raw"] = left_raw.result_json;
        }

        nlohmann::json right_response;
        if (racshell::parse_sear_response_json(right_raw.result_json.c_str(), right_response, parse_error))
        {
            output["right_response"] = right_response;
        }
        else
        {
            output["right_response_raw"] = right_raw.result_json;
        }

        std::cout << output.dump(2) << "\n";
        return 0;
    }

    ExtractedGroup left_group;
    ExtractedGroup right_group;
    std::string error_message;
    if (!parse_group_extract(left_raw, left_group, error_message))
    {
        racshell::print_error(std::cerr, left_groupid + ": " + error_message);
        return 1;
    }

    if (!parse_group_extract(right_raw, right_group, error_message))
    {
        racshell::print_error(std::cerr, right_groupid + ": " + error_message);
        return 1;
    }

    nlohmann::json differences = nlohmann::json::object();
    add_difference(differences, "owner",
                   get_object_value(left_group.base, "base:owner"),
                   get_object_value(right_group.base, "base:owner"));
    add_difference(differences, "created_date",
                   get_object_value(left_group.base, "base:create_date"),
                   get_object_value(right_group.base, "base:create_date"));
    add_difference(differences, "superior_group",
                   get_object_value(left_group.base, "base:superior_group"),
                   get_object_value(right_group.base, "base:superior_group"));
    add_difference(differences, "installation_data",
                   get_object_value(left_group.base, "base:installation_data"),
                   get_object_value(right_group.base, "base:installation_data"));
    add_difference(differences, "universal",
                   get_object_value(left_group.base, "base:universal"),
                   get_object_value(right_group.base, "base:universal"));
    add_difference(differences, "terminal_universal_access",
                   get_object_value(left_group.base, "base:terminal_universal_access"),
                   get_object_value(right_group.base, "base:terminal_universal_access"));

    if (compare_omvs)
    {
        add_difference(differences, "omvs",
                       get_object_value(left_group.profile, "omvs"),
                       get_object_value(right_group.profile, "omvs"));
    }

    if (compare_csdata)
    {
        add_difference(differences, "csdata",
                       get_object_value(left_group.profile, "csdata"),
                       get_object_value(right_group.profile, "csdata"));
    }

    if (compare_users)
    {
        add_connected_user_differences(left_group, right_group, differences);
    }

    const bool identical = differences.empty();

    if (json_output)
    {
        nlohmann::json output = {
            {"left_group", left_groupid},
            {"right_group", right_groupid},
            {"identical", identical},
            {"differences", differences}};
        std::cout << output.dump(2) << "\n";
        return 0;
    }

    if (identical)
    {
        racshell::print_success_prefix(std::cout);
        std::cout << "Groups " << left_groupid << " and " << right_groupid
                  << " are identical for compared fields.\n";
        return 0;
    }

    std::cout << "Comparing groups " << left_groupid << " and " << right_groupid << ":\n";
    for (auto it = differences.begin(); it != differences.end(); ++it)
    {
        if (it.key() == "connected_users")
        {
            const nlohmann::json &users = it.value();
            if (users.contains("only_in_left"))
            {
                print_user_list("  Users only in " + left_groupid + ":", users["only_in_left"]);
            }
            if (users.contains("only_in_right"))
            {
                print_user_list("  Users only in " + right_groupid + ":", users["only_in_right"]);
            }
            if (users.contains("authority_mismatches"))
            {
                std::cout << "  User authority differences\n";
                for (const auto &mismatch : users["authority_mismatches"])
                {
                    std::cout << "    " << mismatch.value("userid", "")
                              << ": " << left_groupid << "='" << mismatch.value("left", "")
                              << "', " << right_groupid << "='" << mismatch.value("right", "") << "'\n";
                }
            }
            continue;
        }

        std::cout << "  " << it.key() << ": "
                  << left_groupid << "='" << racshell::value_to_text(it.value()["left"])
                  << "', " << right_groupid << "='" << racshell::value_to_text(it.value()["right"]) << "'\n";
    }

    return 0;
}