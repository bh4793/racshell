#include <argparse.hpp>
#include <string>
#include <iostream>
#include <vector>

#include "sear/sear.h"
#include "lib/command_helper.hpp"
#include <nlohmann/json.hpp>

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("altgroup");

    program.add_argument("group")
        .help("RACF group to alter");

    program.add_argument("-t", "--traits")
        .help("traits to alter, e.g. base:owner='SYSPROG' omvs:gid=1234567")
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

    std::string group = program.get<std::string>("group");
    if (group.length() > 8)
    {
        racshell::print_error(std::cerr, "Invalid input, must be a valid RACF group");
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
        {"operation", "alter"},
        {"admin_type", "group"},
        {"group", group},
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
    std::cout << "Group " << group << " altered successfully.\n";
    return 0;
}
