
#include <argparse.hpp>
#include <string>
#include <iostream>

#include "sear/sear.h"
#include <nlohmann/json.hpp>
#include <stdlib.h>

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
        std::string request_json = get_json_sample("test_extract_user.json");
        bool debug = false;
        sear_result_t *result =
           sear(request_json.c_str(), request_json.length(), debug);
        std::cout << "Result: " << result <<  std::string("\n");
        std::cout << result->result_json << "\n";

        std::cout << std::string("\033[35m---------------------------------------------------------\033[37mRACSHELL\033[35m---\x1b[0m\n");
        std::cout << std::string("Listuser: ") << input << std::string("\n");
        std::cout << std::string("\033[35m--------------------------------------------------------------------\x1b[0m\n");
    }

    return 0;
}