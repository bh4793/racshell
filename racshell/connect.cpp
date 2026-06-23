#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "sear/sear.h"
#include "lib/command_helper.hpp"

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("connect");

    program.add_argument("userid")
        .help("RACF userid to connect");

    program.add_argument("group")
        .help("RACF group to connect the user to");

    program.add_argument("-t", "--traits")
        .help("connection traits to set, e.g. base:special=true")
        .nargs(argparse::nargs_pattern::any);

    racshell::add_no_color_argument(program);
    racshell::add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
    racshell::add_toggle_argument(program, "-a", "--all-json", "output full raw SEAR JSON response");

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

    const std::string userid = program.get<std::string>("userid");
    if (userid.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF userid");
        return 1;
    }

    const std::string group = program.get<std::string>("group");
    if (group.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF group");
        return 1;
    }

    const bool debug = program.get<bool>("debug");
    const bool all_json = program.get<bool>("all-json");

    nlohmann::json traits;
    if (!racshell::parse_traits(program.get<std::vector<std::string>>("traits"), traits))
    {
        return 1;
    }

    const nlohmann::json request = {
        {"operation", "alter"},
        {"admin_type", "group-connection"},
        {"userid", userid},
        {"group", group},
        {"traits", traits}};

    const std::string request_json = request.dump();
    sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

    if (all_json)
    {
        std::cout << result->result_json << "\n";
        return 0;
    }

    const racshell::SearOperationInfo op_info = racshell::validate_sear_operation_result(result->result_json);
    if (!op_info.success)
    {
        racshell::print_error(std::cerr, op_info.error_message);
        racshell::print_sear_errors(op_info.response, std::cerr);
        return 1;
    }

    racshell::print_success_prefix(std::cout);
    std::cout << "User " << userid << " connected to group " << group << " successfully.\n";
    return 0;
}
