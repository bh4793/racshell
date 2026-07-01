#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "include/racshell/command_helper.hpp"
#include "racshell/output_formatter.hpp"

namespace
{
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
                                                  bool compare_groups,
                                                  bool compare_security,
                                                  bool compare_tso,
                                                  bool compare_kerberos,
                                                  bool compare_cics,
                                                  bool compare_omvs,
                                                  bool compare_csdata,
                                                  bool debug,
                                                  bool raw_json_output)
    {
        UserComparisonData comparison;
        comparison.left.userid = left_userid;
        comparison.right.userid = right_userid;
        comparison.compare_groups = compare_groups;
        comparison.compare_security = compare_security;
        comparison.compare_tso = compare_tso;
        comparison.compare_kerberos = compare_kerberos;
        comparison.compare_cics = compare_cics;
        comparison.compare_omvs = compare_omvs;
        comparison.compare_csdata = compare_csdata;
        comparison.raw_json_output = raw_json_output;

        comparison.left.raw_response_json = racshell::execute_extract_request("user", "userid", left_userid, debug);
        comparison.right.raw_response_json = racshell::execute_extract_request("user", "userid", right_userid, debug);

        if (raw_json_output)
        {
            nlohmann::json left_response;
            std::string parse_error;
            if (racshell::parse_sear_response_json(comparison.left.raw_response_json.c_str(), left_response, parse_error))
            {
                comparison.left.response_json = left_response;
            }

            nlohmann::json right_response;
            if (racshell::parse_sear_response_json(comparison.right.raw_response_json.c_str(), right_response, parse_error))
            {
                comparison.right.response_json = right_response;
            }

            return comparison;
        }

        nlohmann::json left_profile;
        nlohmann::json left_base;
        nlohmann::json right_profile;
        nlohmann::json right_base;
        std::string error_message;
        if (!racshell::parse_extract_payload(comparison.left.raw_response_json,
                                             "user",
                                             left_userid,
                                             left_profile,
                                             left_base,
                                             error_message))
        {
            throw std::runtime_error(left_userid + ": " + error_message);
        }

        if (!racshell::parse_extract_payload(comparison.right.raw_response_json,
                                             "user",
                                             right_userid,
                                             right_profile,
                                             right_base,
                                             error_message))
        {
            throw std::runtime_error(right_userid + ": " + error_message);
        }

        comparison.left.profile = left_profile;
        comparison.left.base = left_base;
        comparison.right.profile = right_profile;
        comparison.right.base = right_base;

        racshell::add_difference(comparison.differences, "name",
                                 racshell::get_object_value(left_base, "base:name"),
                                 racshell::get_object_value(right_base, "base:name"));
        racshell::add_difference(comparison.differences, "owner",
                                 racshell::get_object_value(left_base, "base:owner"),
                                 racshell::get_object_value(right_base, "base:owner"));
        racshell::add_difference(comparison.differences, "default_group",
                                 racshell::get_object_value(left_base, "base:default_group"),
                                 racshell::get_object_value(right_base, "base:default_group"));
        racshell::add_difference(comparison.differences, "created_date",
                                 racshell::get_object_value(left_base, "base:create_date"),
                                 racshell::get_object_value(right_base, "base:create_date"));
        racshell::add_difference(comparison.differences, "revoked",
                                 racshell::get_object_value(left_base, "base:revoked"),
                                 racshell::get_object_value(right_base, "base:revoked"));

        if (compare_security)
        {
            racshell::add_difference(comparison.differences, "security",
                                     build_security_snapshot(left_base, left_profile),
                                     build_security_snapshot(right_base, right_profile));
        }
        if (compare_tso)
        {
            racshell::add_difference(comparison.differences, "tso",
                                     racshell::get_object_value(left_profile, "tso"),
                                     racshell::get_object_value(right_profile, "tso"));
        }
        if (compare_kerberos)
        {
            racshell::add_difference(comparison.differences, "kerberos",
                                     racshell::get_object_value(left_profile, "kerberos"),
                                     racshell::get_object_value(right_profile, "kerberos"));
        }
        if (compare_cics)
        {
            racshell::add_difference(comparison.differences, "cics",
                                     racshell::get_object_value(left_profile, "cics"),
                                     racshell::get_object_value(right_profile, "cics"));
        }

        if (compare_groups)
        {
            racshell::add_list_difference(comparison.differences,
                                          "groups",
                                          extract_groups(left_base),
                                          extract_groups(right_base));
        }
        if (compare_omvs)
        {
            racshell::add_difference(comparison.differences, "omvs",
                                     racshell::get_object_value(left_profile, "omvs"),
                                     racshell::get_object_value(right_profile, "omvs"));
        }
        if (compare_csdata)
        {
            racshell::add_difference(comparison.differences, "csdata",
                                     racshell::get_object_value(left_profile, "csdata"),
                                     racshell::get_object_value(right_profile, "csdata"));
        }

        comparison.identical = comparison.differences.empty();
        return comparison;
    }

} // namespace

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("compareusers");

    program.add_argument("left-user")
        .help("first RACF user to compare");

    program.add_argument("right-user")
        .help("second RACF user to compare");

    racshell::add_toggle_argument(program, "-u", "--groups", "compare groups");
    racshell::add_toggle_argument(program, "-s", "--security", "compare security data");
    racshell::add_toggle_argument(program, "-t", "--tso", "compare TSO segment");
    racshell::add_toggle_argument(program, "-k", "--kerberos", "compare Kerberos segment");
    racshell::add_toggle_argument(program, "-c", "--cics", "compare CICS segment");
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

    const std::string left_userid = program.get<std::string>("left-user");
    const std::string right_userid = program.get<std::string>("right-user");
    if (left_userid.length() > 8 || right_userid.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF userid");
        return 1;
    }

    const bool compare_groups = program.get<bool>("groups");
    const bool compare_security = program.get<bool>("security");
    const bool compare_tso = program.get<bool>("tso");
    const bool compare_kerberos = program.get<bool>("kerberos");
    const bool compare_cics = program.get<bool>("cics");
    const bool compare_omvs = program.get<bool>("omvs");
    const bool compare_csdata = program.get<bool>("csdata");
    const bool debug = program.get<bool>("debug");
    const bool json_output = program.get<bool>("json");
    const bool all_json = program.get<bool>("all-json");

    try
    {
        const UserComparisonData comparison = build_user_comparison_data(left_userid,
                                                                         right_userid,
                                                                         compare_groups,
                                                                         compare_security,
                                                                         compare_tso,
                                                                         compare_kerberos,
                                                                         compare_cics,
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