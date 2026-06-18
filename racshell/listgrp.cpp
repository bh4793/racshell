#include <argparse.hpp>
#include <string>
#include <iostream>
#include <memory>

#include "sear/sear.h"
#include "lib/output_formatter.hpp"
#include <nlohmann/json.hpp>

int main(int argc, char *argv[]) {
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

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    std::string input = program.get<std::string>("group");

    if (input.length() > 8) {
        std::cout << "\u001b[31mRACSHELL Error\x1b[0m: Invalid input, must be a valid RACF group name\n";
        return 1;
    }

    bool debug       = program.get<bool>("debug");
    bool users       = program.get<bool>("users");
    bool omvs        = program.get<bool>("omvs");
    bool json_output = program.get<bool>("json");

    nlohmann::json req = {
        {"operation", "extract"},
        {"admin_type", "group"},
        {"group", input}
    };

    std::string request_json = req.dump();
    sear_result_t* result = sear(request_json.c_str(), request_json.length(), debug);

    std::cout << result->result_json << "\n"; // TODO: remove this debug output

    nlohmann::json response = nlohmann::json::parse(result->result_json);

    GroupData group_data;
    group_data.groupid        = input;
    group_data.owner          = response["profile"]["base"]["base:owner"];
    group_data.created_date   = response["profile"]["base"]["base:create_date"];
    group_data.superior_group = response["profile"]["base"]["base:superior_group"];
    if (response["profile"]["base"].contains("base:installation_data")) {
        group_data.installation_data = response["profile"]["base"]["base:installation_data"];
    }

    if (response["profile"]["base"].contains("base:universal"))
        group_data.universal = response["profile"]["base"]["base:universal"].get<bool>();

    if (response["profile"]["base"].contains("base:terminal_universal_access"))
        group_data.terminal_universal_access = response["profile"]["base"]["base:terminal_universal_access"].get<bool>();

    if (users && response["profile"]["base"].contains("base:connected_users")) {
        for (const auto& u : response["profile"]["base"]["base:connected_users"]) {
            GroupUser gu;
            gu.userid    = u["base:connected_userid"];
            gu.authority = u["base:connected_user_authority"];
            group_data.connected_users.push_back(gu);
        }
    }

    if (omvs && response["profile"].contains("omvs")) {
        group_data.omvs = response["profile"]["omvs"];
    }

    std::unique_ptr<OutputFormatter> formatter;
    if (json_output) {
        formatter = std::make_unique<JsonFormatter>();
    } else {
        formatter = std::make_unique<TextFormatter>();
    }

    std::cout << formatter->format(group_data) << "\n";

    if (!json_output) {
        std::cout << "\033[35m---------------------------------------------------------\033[37mRACSHELL\033[35m---\x1b[0m\n";
        std::cout << "Listgrp: " << input << "\n";
        std::cout << "\033[35m--------------------------------------------------------------------\x1b[0m\n";
    }

    return 0;
}
