#ifndef USER_COMPARISON_DATA_HPP
#define USER_COMPARISON_DATA_HPP

#include <nlohmann/json.hpp>
#include <string>

struct UserComparisonSide
{
    std::string userid;
    std::string raw_response_json;
    nlohmann::json response_json;
};

struct UserComparisonData
{
    UserComparisonSide left;
    UserComparisonSide right;
    bool raw_json_output = false;
    bool identical = false;
    nlohmann::json differences;
};

#endif // USER_COMPARISON_DATA_HPP