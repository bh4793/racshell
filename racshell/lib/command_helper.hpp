#pragma once

#include <argparse.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>

namespace racshell {

inline void add_toggle_argument(argparse::ArgumentParser& program,
                                const char* short_name,
                                const char* long_name,
                                const char* help_text) {
    program.add_argument(short_name, long_name)
        .help(help_text)
        .default_value(false)
        .implicit_value(true)
        .nargs(0);
}

struct SearResponseInfo {
    bool success;
    std::string error_message;
    nlohmann::json profile;
    nlohmann::json base;
};

struct SearOperationInfo {
    bool success;
    std::string error_message;
    nlohmann::json response;
};

inline SearOperationInfo validate_sear_operation_result(const char* result_json) {
    SearOperationInfo info;
    info.success = false;

    try {
        nlohmann::json response = nlohmann::json::parse(result_json);
        nlohmann::json return_codes = response.value("return_codes", nlohmann::json::object());
        int sear_rc = return_codes.value("sear_return_code", 0);
        int saf_rc = return_codes.value("saf_return_code", 0);
        int racf_rc = return_codes.value("racf_return_code", 0);
        int racf_reason = return_codes.value("racf_reason_code", 0);

        if (sear_rc != 0 || saf_rc != 0 || racf_rc != 0) {
            info.error_message = "RACSHELL Error: request failed (sear=" + std::to_string(sear_rc)
                + ", saf=" + std::to_string(saf_rc)
                + ", racf=" + std::to_string(racf_rc)
                + ", reason=" + std::to_string(racf_reason) + ")";
            info.response = response;
            return info;
        }

        info.success = true;
        info.response = response;
        return info;
    } catch (const std::exception& e) {
        info.error_message = std::string("RACSHELL Error: Failed to parse SEAR response: ") + e.what();
        return info;
    }
}

inline void print_sear_errors(const nlohmann::json& response, std::ostream& output) {
    if (response.contains("errors") && response["errors"].is_array()) {
        for (const auto& err : response["errors"]) {
            if (err.is_string()) {
                output << "  " << err.get<std::string>() << "\n";
            }
        }
    }
}

/**
 * Validate and extract profile/base from SEAR response JSON string
 * Parses the JSON and checks return codes and structure validity
 */
inline SearResponseInfo validate_sear_response(const char* result_json, const std::string& entity_type) {
    SearResponseInfo info;
    info.success = false;
    
    try {
        nlohmann::json response = nlohmann::json::parse(result_json);
        
        // Extract return codes
        nlohmann::json return_codes = response.value("return_codes", nlohmann::json::object());
        int sear_rc = return_codes.value("sear_return_code", 0);
        int saf_rc = return_codes.value("saf_return_code", 0);
        int racf_rc = return_codes.value("racf_return_code", 0);
        int racf_reason = return_codes.value("racf_reason_code", 0);

        if (sear_rc != 0 || saf_rc != 0 || racf_rc != 0) {
            info.error_message = "RACSHELL Error: request failed (sear=" + std::to_string(sear_rc)
                + ", saf=" + std::to_string(saf_rc)
                + ", racf=" + std::to_string(racf_rc)
                + ", reason=" + std::to_string(racf_reason) + ")";
            return info;
        }

        // Check profile and base structure
        nlohmann::json profile = response.value("profile", nlohmann::json::object());
        nlohmann::json base = profile.value("base", nlohmann::json::object());

        if (!response.contains("profile") || !profile.is_object() || 
            !profile.contains("base") || !base.is_object()) {
            info.error_message = std::string("RACSHELL Error: ") + entity_type + " not found or missing profile data";
            return info;
        }

        info.success = true;
        info.profile = profile;
        info.base = base;
        return info;
    } catch (const std::exception& e) {
        info.error_message = std::string("RACSHELL Error: Failed to parse SEAR response: ") + e.what();
        return info;
    }
}

/**
 * Parse a list of "key=value" trait strings into a JSON object.
 * Detects booleans (true/false), integers, and falls back to strings.
 * Returns false and writes an error to stderr if a value is malformed.
 */
inline bool parse_traits(const std::vector<std::string>& trait_args, nlohmann::json& traits) {
    traits = nlohmann::json::object();
    for (const auto& trait : trait_args) {
        auto sep = trait.find('=');
        if (sep == std::string::npos) {
            std::cerr << "\u001b[31mRACSHELL Error\x1b[0m: trait must be in key=value format, got: " << trait << "\n";
            return false;
        }
        std::string key   = trait.substr(0, sep);
        std::string value = trait.substr(sep + 1);

        if (value == "true" || value == "TRUE")  { traits[key] = true;  continue; }
        if (value == "false" || value == "FALSE") { traits[key] = false; continue; }

        try {
            size_t pos;
            long long int_val = std::stoll(value, &pos);
            if (pos == value.size()) { traits[key] = int_val; continue; }
        } catch (...) {}

        traits[key] = value;
    }
    return true;
}

/**
 * Safe string extraction from JSON object
 */
inline void assign_string(const nlohmann::json& obj, const char* key, std::string& out) {
    auto element = obj.find(key);
    if (element != obj.end() && element->is_string()) {
        out = element->get<std::string>();
    }
}

/**
 * Safe boolean extraction from JSON object
 */
inline void assign_bool(const nlohmann::json& obj, const char* key, bool& out) {
    auto element = obj.find(key);
    if (element != obj.end() && element->is_boolean()) {
        out = element->get<bool>();
    }
}

}  // namespace racshell
