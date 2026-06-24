#ifndef RESOURCE_DATA_HPP
#define RESOURCE_DATA_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct ResourceAccessEntry
{
    std::string access_type;
    std::string access_id;
};

struct ResourceData
{
    std::string resource;
    std::string resource_class;
    std::string owner;
    std::string universal_access;
    nlohmann::json profile;
    nlohmann::json base;
    std::vector<ResourceAccessEntry> access_list;
    nlohmann::json csdata;
};

#endif // RESOURCE_DATA_HPP
