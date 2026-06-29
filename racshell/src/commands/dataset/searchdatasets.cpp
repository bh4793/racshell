#include <argparse.hpp>
#include <string>
#include <iostream>
#include <vector>

#include "sear/sear.h"
#include "include/racshell/command_helper.hpp"
#include <nlohmann/json.hpp>

int main(int argc, char *argv[])
{
    argparse::ArgumentParser program("searchdatasets");

    program.add_argument("-f", "--filter")
        .help("filter dataset profiles by name pattern, e.g. LEONARD")
        .default_value(std::string(""));

    racshell::add_no_color_argument(program);
    racshell::add_toggle_argument(program, "-d", "--debug", "debug sear request and response");
    racshell::add_toggle_argument(program, "-j", "--json", "output as JSON");
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

    const std::string filter = program.get<std::string>("filter");
    if (filter.length() > 44)
    {
        racshell::print_error(std::cerr, "Invalid input, filter must not exceed 44 characters");
        return 1;
    }

    const bool debug = program.get<bool>("debug");
    const bool json_output = program.get<bool>("json");
    const bool all_json = program.get<bool>("all-json");

    nlohmann::json request = {
        {"operation", "search"},
        {"admin_type", "dataset"}};

    if (!filter.empty())
    {
        request["dataset_filter"] = filter;
    }

    const std::string request_json = request.dump();
    sear_result_t *result = sear(request_json.c_str(), request_json.length(), debug);

    if (all_json)
    {
        std::cout << result->result_json << "\n";
        return 0;
    }

    nlohmann::json response;
    std::string error_message;
    if (!racshell::parse_sear_response_json(result->result_json, response, error_message))
    {
        racshell::print_error(std::cerr, error_message);
        return 1;
    }

    const racshell::SearReturnCodes codes = racshell::extract_sear_return_codes(response);
    if (!racshell::validate_sear_return_codes(codes, error_message))
    {
        racshell::print_error(std::cerr, error_message);
        return 1;
    }

    std::vector<std::string> datasets;
    if (response.contains("profiles") && response["profiles"].is_array())
    {
        for (const auto &profile : response["profiles"])
        {
            if (profile.is_string())
            {
                datasets.push_back(profile.get<std::string>());
            }
        }
    }

    if (json_output)
    {
        nlohmann::json output = nlohmann::json::array();
        for (const auto &dataset : datasets)
        {
            output.push_back(dataset);
        }
        std::cout << output.dump(2) << "\n";
    }
    else
    {
        if (datasets.empty())
        {
            std::cout << "No datasets found\n";
        }
        else
        {
            std::cout << "Datasets";
            if (!filter.empty())
            {
                std::cout << " matching '" << filter << "'";
            }
            std::cout << ":\n";

            for (const auto &dataset : datasets)
            {
                std::cout << "  " << dataset << "\n";
            }
        }
    }

    return 0;
}
