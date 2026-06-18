#include <argparse.hpp>
#include <string>
#include <iostream>
#include <vector>

#include "sear/sear.h"
#include "lib/command_helper.hpp"
#include <nlohmann/json.hpp>

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("adduser");

    program.add_argument("userid")
        .help("RACF userid to create");

    program.add_argument("-t", "--trait")
        .help("trait to set, e.g. base:name='John Doe' omvs:uid=24 omvs:home_directory=/home/USER")
        .nargs(argparse::nargs_pattern::any);

    racshell::add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
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

    std::string userid = program.get<std::string>("userid");

    if (userid.length() > 8)
    {
        std::cerr << "\u001b[31mRACSHELL Error\x1b[0m: Invalid input, must be a valid RACF userid\n";
        return 1;
    }

    bool debug = program.get<bool>("debug");
    bool all_json = program.get<bool>("all-json");

    // Build traits object from --trait key=value pairs
    nlohmann::json traits = nlohmann::json::object();
    auto trait_args = program.get<std::vector<std::string>>("trait");
    for (const auto &trait : trait_args)
    {
        auto sep = trait.find('=');
        if (sep == std::string::npos)
        {
            std::cerr << "\u001b[31mRACSHELL Error\x1b[0m: trait must be in key=value format, got: " << trait << "\n";
            return 1;
        }
        std::string key = trait.substr(0, sep);
        std::string value = trait.substr(sep + 1);

        // Try to parse as integer, otherwise keep as string
        try {
            size_t pos; // variable to store the number of characters processed by stoll
            long long int_val = std::stoll(value, &pos);
            if (pos == value.size()) {
                traits[key] = int_val;
                continue;
            }
        } catch (...) {}

        traits[key] = value;
    }

    nlohmann::json req = {
        {"operation", "add"},
        {"admin_type", "user"},
        {"userid", userid},
        {"traits", traits}
    };

    std::string request_json = req.dump();

    sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);
    
    if (all_json)
    {
        std::cout << result->result_json << "\n";
        return 0;
    }

    racshell::SearOperationInfo op_info = racshell::validate_sear_operation_result(result->result_json);
    if (!op_info.success)
    {
        std::cerr << op_info.error_message << "\n";
        racshell::print_sear_errors(op_info.response, std::cerr);
        return 1;
    }

    std::cout << "User " << userid << " created successfully.\n";
    return 0;
}
