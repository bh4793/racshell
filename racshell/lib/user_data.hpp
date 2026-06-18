#ifndef USER_DATA_HPP
#define USER_DATA_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct UserData {
    std::string userid;
    std::string name;
    std::string owner;
    std::string created_date;
    std::vector<std::string> groups;
    nlohmann::json tso;
    nlohmann::json omvs;
    nlohmann::json kerberos;
    nlohmann::json cics;
};

#endif // USER_DATA_HPP
