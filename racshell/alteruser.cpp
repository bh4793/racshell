#include <argparse.hpp>
#include <string>
#include <iostream>
#include <vector>

#include "sear/sear.h"
#include "lib/command_helper.hpp"
#include <nlohmann/json.hpp>

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("alteruser");

    program.add_argument("userid")
        .help("RACF userid to alter");

    program.add_argument("-t", "--trait")
        .help("trait to alter, e.g. base:name='LEONARD CARCARAMO' base:special=true")
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

    nlohmann::json traits;
    if (!racshell::parse_traits(program.get<std::vector<std::string>>("trait"), traits))
    {
        return 1;
    }
    nlohmann::json req = {
        {"operation", "alter"},
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
        std::cerr << op_info.error_message << "\n";
        racshell::print_sear_errors(op_info.response, std::cerr);
        return 1;
    }

    std::cout << "User " << userid << " altered successfully.\n";
    return 0;
}
