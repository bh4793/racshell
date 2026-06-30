#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "sear/sear.h"
#include "include/racshell/command_helper.hpp"
#include "racshell/output_formatter.hpp"

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

    nlohmann::json get_object_value(const nlohmann::json &object, const char *key);

    void add_difference(nlohmann::json &differences,
                        const char *label,
                        const nlohmann::json &left,
                        const nlohmann::json &right);

    void add_connected_user_differences(const ExtractedGroup &left_group,
                                        const ExtractedGroup &right_group,
                                        nlohmann::json &differences);

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

    GroupComparisonData build_group_comparison_data(const std::string &left_groupid,
                                                    const std::string &right_groupid,
                                                    bool compare_users,
                                                    bool compare_omvs,
                                                    bool compare_csdata,
                                                    bool debug,
                                                    bool raw_json_output)
    {
        GroupComparisonData comparison;
        comparison.left.groupid = left_groupid;
        comparison.right.groupid = right_groupid;
        comparison.compare_users = compare_users;
        comparison.compare_omvs = compare_omvs;
        comparison.compare_csdata = compare_csdata;
        comparison.raw_json_output = raw_json_output;

        const RawGroupResponse left_raw = execute_group_extract(left_groupid, debug);
        const RawGroupResponse right_raw = execute_group_extract(right_groupid, debug);
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

        ExtractedGroup left_group;
        ExtractedGroup right_group;
        std::string error_message;
        if (!parse_group_extract(left_raw, left_group, error_message))
        {
            throw std::runtime_error(left_groupid + ": " + error_message);
        }

        if (!parse_group_extract(right_raw, right_group, error_message))
        {
            throw std::runtime_error(right_groupid + ": " + error_message);
        }

        comparison.left.profile = left_group.profile;
        comparison.left.base = left_group.base;
        comparison.right.profile = right_group.profile;
        comparison.right.base = right_group.base;

        add_difference(comparison.differences, "owner",
                       get_object_value(left_group.base, "base:owner"),
                       get_object_value(right_group.base, "base:owner"));
        add_difference(comparison.differences, "created_date",
                       get_object_value(left_group.base, "base:create_date"),
                       get_object_value(right_group.base, "base:create_date"));
        add_difference(comparison.differences, "superior_group",
                       get_object_value(left_group.base, "base:superior_group"),
                       get_object_value(right_group.base, "base:superior_group"));
        add_difference(comparison.differences, "installation_data",
                       get_object_value(left_group.base, "base:installation_data"),
                       get_object_value(right_group.base, "base:installation_data"));
        add_difference(comparison.differences, "universal",
                       get_object_value(left_group.base, "base:universal"),
                       get_object_value(right_group.base, "base:universal"));
        add_difference(comparison.differences, "terminal_universal_access",
                       get_object_value(left_group.base, "base:terminal_universal_access"),
                       get_object_value(right_group.base, "base:terminal_universal_access"));

        if (compare_omvs)
        {
            add_difference(comparison.differences, "omvs",
                           get_object_value(left_group.profile, "omvs"),
                           get_object_value(right_group.profile, "omvs"));
        }

        if (compare_csdata)
        {
            add_difference(comparison.differences, "csdata",
                           get_object_value(left_group.profile, "csdata"),
                           get_object_value(right_group.profile, "csdata"));
        }

        if (compare_users)
        {
            add_connected_user_differences(left_group, right_group, comparison.differences);
        }

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

    void add_connected_user_differences(const ExtractedGroup &left_group,
                                        const ExtractedGroup &right_group,
                                        nlohmann::json &differences)
    {
        const std::map<std::string, std::string> left_users = racshell::to_user_map(get_object_value(left_group.base, "base:connected_users"));
        const std::map<std::string, std::string> right_users = racshell::to_user_map(get_object_value(right_group.base, "base:connected_users"));

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

    try
    {
        const GroupComparisonData comparison = build_group_comparison_data(left_groupid,
                                                                           right_groupid,
                                                                           compare_users,
                                                                           compare_omvs,
                                                                           compare_csdata,
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