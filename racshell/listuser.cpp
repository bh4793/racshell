
#include <argparse.hpp>
#include <string>
#include <iostream>

#include "sear/sear.h"
#include <nlohmann/json.hpp>
#include <stdlib.h>
#include <vector>

char *get_sample(const char *filename, const char *mode) {
    // open file
    FILE *fp = fopen(filename, mode);
    if (fp == NULL) {
        perror("");
        printf("Unable to open sample '%s' in '%s' mode for reading.\n", filename,
            mode);
        exit(1);
    }
    // get size of file
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
    // allocate space to read in data from file
    char *file_data = (char *)calloc(size + 1, sizeof(char));
    if (file_data == NULL) {
        perror("");
        printf("Unable to allocate space to load data from '%s'.\n", filename);
        fclose(fp);
        exit(1);
    }
    // read file data
    fread(file_data, size, 1, fp);
    fclose(fp);
    return file_data;
}

std::string get_json_sample(const char *filename) {
    char *json_sample_string = get_sample(filename, "r");
    std::string json_sample_cpp_string =
        nlohmann::json::parse(json_sample_string).dump();
    free(json_sample_string);
    return json_sample_cpp_string;
}

// Split "base:name" into ["base", "name"]
std::vector<std::string> split_path(const std::string& path) {
    std::vector<std::string> parts;
    size_t start = 0;
    size_t pos = path.find(':');
    
    while (pos != std::string::npos) {
        parts.push_back(path.substr(start, pos - start));
        start = pos + 1;
        pos = path.find(':', start);
    }
    parts.push_back(path.substr(start));
    
    return parts;
}

// Navigate JSON using path like "base:name"
nlohmann::json get_by_path(const nlohmann::json& obj, const std::string& path) {
    auto parts = split_path(path);
    nlohmann::json current = obj;
    
    for (size_t i = 0; i < parts.size(); ++i) {
        const auto& part = parts[i];
        bool is_last = (i + 1 == parts.size());

        if (!current.contains(part)) {
            return nlohmann::json();
        }

        if (is_last) {
            // last segment in path
            return current[path];
        }

        current = current[part];
    }
    return current;
}


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

        std::string user_default_info = "User: " + input + "\n";
        user_default_info += "Name: ";
        user_default_info += response["profile"]["base"]["base:name"];
        user_default_info += "\n";
        user_default_info += "Owner: ";
        user_default_info += response["profile"]["base"]["base:owner"];
        user_default_info += "\n";
        user_default_info += "Created: "; 
        user_default_info += response["profile"]["base"]["base:create_date"];
        user_default_info += "\n";

        if (groups) {
            user_default_info += "Groups: ";
            for (const auto& group : response["profile"]["base"]["base:group_connections"]) {
                user_default_info += group["base:group_connection_group"];
                user_default_info += ", ";
            }
            user_default_info += "\n";
        }
        if (tso) {
            user_default_info += "TSO: ";
            user_default_info += response["profile"]["tso"];
            user_default_info += "\n";
        }
        if (omvs) {
            user_default_info += "OMVS: ";
            user_default_info += response["profile"]["omvs"];
            user_default_info += "\n";
        }

        std::cout << user_default_info << std::string("\n");

        // std::cout << listuser_fields << "\n";

        std::cout << std::string("\033[35m---------------------------------------------------------\033[37mRACSHELL\033[35m---\x1b[0m\n");
        std::cout << std::string("Listuser: ") << input << std::string("\n");
        std::cout << std::string("\033[35m--------------------------------------------------------------------\x1b[0m\n");


        }

    return 0;
}