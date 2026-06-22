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

    program.add_argument("-t", "--traits")
        .help("traits to set, e.g. base:name='John Doe' omvs:uid=24 omvs:home_directory=/home/USER")
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

    std::string userid = program.get<std::string>("userid");

    if (userid.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF userid");
        return 1;
    }

    bool debug = program.get<bool>("debug");
    bool all_json = program.get<bool>("all-json");

    nlohmann::json traits;
    if (!racshell::parse_traits(program.get<std::vector<std::string>>("traits"), traits))
    {
        return 1;
    }
    nlohmann::json req = {
        {"operation", "add"},
        {"admin_type", "user"},
        {"userid", userid},
        {"traits", traits}};

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
        racshell::print_error(std::cerr, op_info.error_message);
        racshell::print_sear_errors(op_info.response, std::cerr);
        return 1;
    }

    racshell::print_success_prefix(std::cout);
    std::cout << "User " << userid << " created successfully.\n";
    return 0;
}
