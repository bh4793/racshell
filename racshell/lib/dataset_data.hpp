#ifndef DATASET_DATA_HPP
#define DATASET_DATA_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct DatasetAccessEntry
{
    std::string access_type;
    std::string access_id;
};

struct DatasetData
{
    std::string dataset;
    std::string owner;
    std::string uacc;
    bool audited = false;
    int access_count = 0;
    std::vector<DatasetAccessEntry> access_list;
    nlohmann::json csdata;
};

#endif // DATASET_DATA_HPP
