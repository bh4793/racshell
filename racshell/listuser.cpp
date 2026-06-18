
#include <argparse.hpp>
#include <string>
#include <iostream>
#include <memory>

#include "sear/sear.h"
#include "lib/output_formatter.hpp"
#include "lib/command_helper.hpp"
#include <nlohmann/json.hpp>
#include <stdlib.h>
#include <vector>

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("listuser");

    // Main parameter, the user to list
    program.add_argument("user")
        .help("RACF user to list");

    // Optional parameters for data not displayed by default
    racshell::add_toggle_argument(program, "-g", "--groups", "list connected RACF groups");
    racshell::add_toggle_argument(program, "-t", "--tso", "list TSO segment");
    racshell::add_toggle_argument(program, "-k", "--kerberos", "list kerberos segment");
    racshell::add_toggle_argument(program, "-c", "--cics", "list CICS segment");
    racshell::add_toggle_argument(program, "-o", "--omvs", "list OMVS segment");
    racshell::add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
    racshell::add_toggle_argument(program, "-j", "--json", "output as JSON");
    racshell::add_toggle_argument(program, "-a", "--all-json", "output full raw SEAR JSON response");

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

    std::string input = program.get<std::string>("user");

    // Check if the input is too long to be a valid user
    if (input.length() > 8)
    {
        std::cout << std::string("\u001b[31mRACSHELL Error\x1b[0m: Invalid input, must be a valid RACF userid \n");
    }
    else
    {
        bool debug = program.get<bool>("debug");
        bool groups = program.get<bool>("groups");
        bool tso = program.get<bool>("tso");
        bool kerberos = program.get<bool>("kerberos");
        bool cics = program.get<bool>("cics");
        bool omvs = program.get<bool>("omvs");
        bool json_output = program.get<bool>("json");
        bool all_json = program.get<bool>("all-json");


        // extract user information
        nlohmann::json req = {
            {"operation", "extract"},
            {"admin_type", "user"},
            {"userid", input}};

        std::string request_json = req.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

        if (all_json)
        {
            std::cout << result->result_json << "\n";
            return 0;
        }

        racshell::SearResponseInfo sear_info = racshell::validate_sear_response(result->result_json, "user");
        if (!sear_info.success) {
            std::cerr << sear_info.error_message << "\n";
            return 1;
        }
        nlohmann::json base = sear_info.base;
        nlohmann::json profile = sear_info.profile;

        UserData user_data;
        user_data.userid = input;
        racshell::assign_string(base, "base:name", user_data.name);
        racshell::assign_string(base, "base:owner", user_data.owner);
        racshell::assign_string(base, "base:create_date", user_data.created_date);
        racshell::assign_bool(base, "base:revoked", user_data.revoked);

        if (groups && base.contains("base:group_connections"))
        {
            for (const auto &group : base["base:group_connections"])
            {
                if (group.contains("base:group_connection_group"))
                {
                    user_data.groups.push_back(group["base:group_connection_group"]);
                }
            }
        }
        if (tso && profile.contains("tso"))
        {
            user_data.tso = profile["tso"];
        }
        if (omvs && profile.contains("omvs"))
        {
            user_data.omvs = profile["omvs"];
        }
        if (kerberos && profile.contains("kerberos"))
        {
            user_data.kerberos = profile["kerberos"];
        }
        if (cics && profile.contains("cics"))
        {
            user_data.cics = profile["cics"];
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

        std::cout << formatter->format(user_data) << std::string("\n");
    }

    return 0;
}