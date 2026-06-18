
#include <argparse.hpp>
#include <string>
#include <iostream>
#include <memory>

#include "sear/sear.h"
#include "lib/output_formatter.hpp"
#include <nlohmann/json.hpp>
#include <stdlib.h>
#include <vector>

// char *get_sample(const char *filename, const char *mode) {
//     // open file
//     FILE *fp = fopen(filename, mode);
//     if (fp == NULL) {
//         perror("");
//         printf("Unable to open sample '%s' in '%s' mode for reading.\n", filename,
//             mode);
//         exit(1);
//     }
//     // get size of file
//     fseek(fp, 0L, SEEK_END);
//     int size = ftell(fp);
//     rewind(fp);
//     // allocate space to read in data from file
//     char *file_data = (char *)calloc(size + 1, sizeof(char));
//     if (file_data == NULL) {
//         perror("");
//         printf("Unable to allocate space to load data from '%s'.\n", filename);
//         fclose(fp);
//         exit(1);
//     }
//     // read file data
//     fread(file_data, size, 1, fp);
//     fclose(fp);
//     return file_data;
// }

// std::string get_json_sample(const char *filename) {
//     char *json_sample_string = get_sample(filename, "r");
//     std::string json_sample_cpp_string =
//         nlohmann::json::parse(json_sample_string).dump();
//     free(json_sample_string);
//     return json_sample_cpp_string;
// }

// Split "base:name" into ["base", "name"]
// std::vector<std::string> split_path(const std::string& path) {
//     std::vector<std::string> parts;
//     size_t start = 0;
//     size_t pos = path.find(':');
    
//     while (pos != std::string::npos) {
//         parts.push_back(path.substr(start, pos - start));
//         start = pos + 1;
//         pos = path.find(':', start);
//     }
//     parts.push_back(path.substr(start));
    
//     return parts;
// }

// // Navigate JSON using path like "base:name"
// nlohmann::json get_by_path(const nlohmann::json& obj, const std::string& path) {
//     auto parts = split_path(path);
//     nlohmann::json current = obj;
    
//     for (size_t i = 0; i < parts.size(); ++i) {
//         const auto& part = parts[i];
//         bool is_last = (i + 1 == parts.size());

//         if (!current.contains(part)) {
//             return nlohmann::json();
//         }

//         if (is_last) {
//             // last segment in path
//             return current[path];
//         }

//         current = current[part];
//     }
//     return current;
// }


int main(int argc, char *argv[]) {
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
        bool groups = program.get<bool>("groups");
        bool tso = program.get<bool>("tso");
        bool kerberos = program.get<bool>("kerberos");
        bool cics = program.get<bool>("cics");
        bool omvs = program.get<bool>("omvs");
        bool json_output = program.get<bool>("json");

        // std::string listuser_fields = get_json_sample("../../racshell/listuser_fields.json");

        // extract user information
        nlohmann::json req = {
            {"operation", "extract"},
            {"admin_type", "user"},
            {"userid", input}
        };

        std::string request_json = req.dump();
        sear_result_t* result = sear(request_json.c_str(), request_json.length(), debug);

        nlohmann::json response = nlohmann::json::parse(result->result_json);

        UserData user_data;
        user_data.userid = input;
        user_data.name = response["profile"]["base"]["base:name"];
        user_data.owner = response["profile"]["base"]["base:owner"];
        user_data.created_date = response["profile"]["base"]["base:create_date"];

        if (groups) {
            for (const auto& group : response["profile"]["base"]["base:group_connections"]) {
                user_data.groups.push_back(group["base:group_connection_group"]);
            }
        }
        if (tso) {
            user_data.tso = response["profile"]["tso"];
        }
        if (omvs) {
            user_data.omvs = response["profile"]["omvs"];
        }
        if (kerberos) {
            user_data.kerberos = response["profile"]["kerberos"];
        }
        if (cics) {
            user_data.cics = response["profile"]["cics"];
        }

        std::unique_ptr<OutputFormatter> formatter;
        if (json_output) {
            formatter = std::make_unique<JsonFormatter>();
        } else {
            formatter = std::make_unique<TextFormatter>();
        }

        std::cout << formatter->format(user_data) << std::string("\n");

        // std::cout << listuser_fields << "\n";

        if (!json_output) {
            std::cout << std::string("\033[35m---------------------------------------------------------\033[37mRACSHELL\033[35m---\x1b[0m\n");
            std::cout << std::string("Listuser: ") << input << std::string("\n");
            std::cout << std::string("\033[35m--------------------------------------------------------------------\x1b[0m\n");
        }


        }

    return 0;
}