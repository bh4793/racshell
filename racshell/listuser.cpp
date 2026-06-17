
#include <argparse.hpp>
#include <string>
#include <iostream>

#include "sear/sear.h"
#include <nlohmann/json.hpp>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("listuser");

    // Main parameter, the user to list
    program.add_argument("user")
    .help("RACF user to list");

    // Optional parameters for data not displayed by default
    program.add_argument("-g", "--groups")
    .help("list connected RACF groups");

    program.add_argument("-t", "--tso")
    .help("list TSO segment");

    program.add_argument("-k", "--kerberos")
    .help("list kerberos segment");

    program.add_argument("-c", "--cics")
    .help("list CICS segment");

    program.add_argument("-d", "--debug")
    .help("debug sear request and response")
    .default_value(false)
    .implicit_value(true)
    .nargs(0);

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string input = program.get<std::string>("user");
    

    // Check if the input is too long to be a valid user
    if (input.length() > 8) {
        std::cout << std::string("\u001b[31mRACSHELL Error\x1b[0m: Invalid input, must be a valid RACF userid \n");
    } else {
        bool debug = program.get<bool>("debug");

        nlohmann::json req = {
            {"operation", "extract"},
            {"admin_type", "user"},
            {"userid", input}
        };

        std::string request_json = req.dump();
        sear_result_t* result = sear(request_json.c_str(), request_json.length(), debug);

        nlohmann::json response = nlohmann::json::parse(result->result_json);
        // std::cout << result->result_json << "\n";

        std::cout << std::string("\033[35m---------------------------------------------------------\033[37mRACSHELL\033[35m---\x1b[0m\n");
        std::cout << std::string("Listuser: ") << input << std::string("\n");
        std::cout << std::string("\033[35m--------------------------------------------------------------------\x1b[0m\n");


        }

    return 0;
}