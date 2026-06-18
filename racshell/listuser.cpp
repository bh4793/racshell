
#include <argparse.hpp>
#include <string>
#include <iostream>
#include <memory>

#include "sear/sear.h"
#include "lib/output_formatter.hpp"
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
    program.add_argument("-g", "--groups")
        .help("list connected RACF groups")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-t", "--tso")
        .help("list TSO segment")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-k", "--kerberos")
        .help("list kerberos segment")
        .default_value(false)
        .implicit_value(true)
        .nargs(0);

    program.add_argument("-c", "--cics")
        .help("list CICS segment")
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

        nlohmann::json response = nlohmann::json::parse(result->result_json);

        UserData user_data;
        user_data.userid = input;
        user_data.name = response["profile"]["base"]["base:name"];
        user_data.owner = response["profile"]["base"]["base:owner"];
        user_data.created_date = response["profile"]["base"]["base:create_date"];
        user_data.revoked = response["profile"]["base"]["base:revoked"].get<bool>();

        if (groups)
        {
            for (const auto &group : response["profile"]["base"]["base:group_connections"])
            {
                user_data.groups.push_back(group["base:group_connection_group"]);
            }
        }
        if (tso)
        {
            user_data.tso = response["profile"]["tso"];
        }
        if (omvs)
        {
            user_data.omvs = response["profile"]["omvs"];
        }
        if (kerberos)
        {
            user_data.kerberos = response["profile"]["kerberos"];
        }
        if (cics)
        {
            user_data.cics = response["profile"]["cics"];
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