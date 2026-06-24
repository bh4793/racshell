#ifndef RESOURCE_DATA_HPP
#define RESOURCE_DATA_HPP

#include <string>
#include <nlohmann/json.hpp>

struct ResourceData
{
    std::string resource;
    std::string resource_class;
    std::string owner;
    std::string universal_access;
    nlohmann::json profile;
    nlohmann::json base;
    nlohmann::json csdata;
};

#endif // RESOURCE_DATA_HPP
