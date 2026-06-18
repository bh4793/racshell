#include <argparse.hpp>
#include <string>
#include <iostream>
#include <memory>

#include "sear/sear.h"
#include "lib/output_formatter.hpp"
#include <nlohmann/json.hpp>

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("listgrp");

    program.add_argument("group")
        .help("RACF group to list");

    program.add_argument("-u", "--users")
        .help("list connected users")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-o", "--omvs")
        .help("list OMVS segment")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-d", "--debug")
        .help("debug sear request and response")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-j", "--json")
        .help("output as JSON")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-a", "--all-json")
        .help("output full raw SEAR JSON response")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string input = program.get<std::string>("group");

    if (input.length() > 8)
    {
        std::cout << "\u001b[31mRACSHELL Error\x1b[0m: Invalid input, must be a valid RACF group name\n";
    }
    else
    {

        bool debug = program.get<bool>("debug");
        bool users = program.get<bool>("users");
        bool omvs = program.get<bool>("omvs");
        bool json_output = program.get<bool>("json");
        bool all_json = program.get<bool>("all-json");

        nlohmann::json req = {
            {"operation", "extract"},
            {"admin_type", "group"},
            {"group", input}};

        std::string request_json = req.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

        if (all_json)
        {
            std::cout << result->result_json << "\n";
            return 0;
        }

        nlohmann::json response = nlohmann::json::parse(result->result_json);
        nlohmann::json return_codes = response.value("return_codes", nlohmann::json::object());
        int sear_rc = return_codes.value("sear_return_code", 0);
        int saf_rc = return_codes.value("saf_return_code", 0);
        int racf_rc = return_codes.value("racf_return_code", 0);
        int racf_reason = return_codes.value("racf_reason_code", 0);

        if (sear_rc != 0 || saf_rc != 0 || racf_rc != 0)
        {
            std::cerr << "RACSHELL Error: request failed (sear=" << sear_rc
                      << ", saf=" << saf_rc
                      << ", racf=" << racf_rc
                      << ", reason=" << racf_reason << ")\n";
            return 1;
        }

        nlohmann::json profile = response.value("profile", nlohmann::json::object());
        nlohmann::json base = profile.value("base", nlohmann::json::object());

        if (!response.contains("profile") || !profile.is_object() || !profile.contains("base") || !base.is_object())
        {
            std::cerr << "RACSHELL Error: group not found or missing profile data\n";
            return 1;
        }

        GroupData group_data;
        group_data.groupid = input;

        if (base.contains("base:owner"))
        {
            group_data.owner = base["base:owner"];
        }

        if (base.contains("base:create_date"))
        {
            group_data.created_date = base["base:create_date"];
        }
        if (base.contains("base:superior_group"))
        {
            group_data.superior_group = base["base:superior_group"];
        }
        if (base.contains("base:installation_data"))
        {
            group_data.installation_data = base["base:installation_data"];
        }

        if (base.contains("base:universal"))
            group_data.universal = base["base:universal"].get<bool>();

        if (base.contains("base:terminal_universal_access"))
            group_data.terminal_universal_access = base["base:terminal_universal_access"].get<bool>();

        if (users && base.contains("base:connected_users"))
        {
            for (const auto &u : base["base:connected_users"])
            {
                GroupUser gu;
                if (u.contains("base:connected_userid"))
                {
                    gu.userid = u["base:connected_userid"];
                }
                if (u.contains("base:connected_user_authority"))
                {
                    gu.authority = u["base:connected_user_authority"];
                }
                group_data.connected_users.push_back(gu);
            }
        }

        if (omvs && profile.contains("omvs"))
        {
            group_data.omvs = profile["omvs"];
        }

        std::unique_ptr<OutputFormatter> formatter;
        if (json_output)
        {
            formatter = std::make_unique<JsonFormatter>();
        }
        else
        {
            formatter = std::make_unique<TextFormatter>();
        }

        std::cout << formatter->format(group_data) << "\n";
    }
    return 0;
}
