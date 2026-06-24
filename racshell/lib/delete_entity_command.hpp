#pragma once

#include <argparse.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <string>

#include "sear/sear.h"
#include "lib/command_helper.hpp"

namespace racshell
{

    struct DeleteEntityCommandSpec
    {
        const char *command_name;
        const char *admin_type;
        const char *entity_argument;
        const char *entity_help;
        const char *entity_validation_error;
        const char *success_label;
    };

    inline int run_delete_entity_command(int argc, char *argv[], const DeleteEntityCommandSpec &spec)
    {
        argparse::ArgumentParser program(spec.command_name);

        program.add_argument(spec.entity_argument)
            .help(spec.entity_help);

        add_no_color_argument(program);
        add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
        add_toggle_argument(program, "-a", "--all-json", "output full raw SEAR JSON response");

        try
        {
            program.parse_args(argc, argv);
        }
        catch (const std::exception &err)
        {
            print_error(std::cerr, err.what());
            std::cerr << program;
            return 1;
        }

        set_color_output_enabled(!program.get<bool>("no-color"));

        const std::string entity_value = program.get<std::string>(spec.entity_argument);
        if (entity_value.length() > 8)
        {
            print_error(std::cerr, spec.entity_validation_error);
            return 1;
        }

        const bool debug = program.get<bool>("debug");
        const bool all_json = program.get<bool>("all-json");

        nlohmann::json request = {
            {"operation", "delete"},
            {"admin_type", spec.admin_type},
            {spec.entity_argument, entity_value}};

        const std::string request_json = request.dump();
        sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

        if (all_json)
        {
            std::cout << result->result_json << "\n";
            return 0;
        }

        const SearOperationInfo op_info = validate_sear_operation_result(result->result_json);
        if (!op_info.success)
        {
            print_error(std::cerr, op_info.error_message);
            print_sear_errors(op_info.response, std::cerr);
            return 1;
        }

        print_success_prefix(std::cout);
        std::cout << spec.success_label << " " << entity_value << " deleted successfully.\n";
        return 0;
    }

} // namespace racshell
