#ifndef OUTPUT_FORMATTER_HPP
#define OUTPUT_FORMATTER_HPP

#include <string>
#include "user_data.hpp"
#include "group_data.hpp"
#include "dataset_data.hpp"
#include "resource_data.hpp"

class OutputFormatter
{
public:
    virtual ~OutputFormatter() = default;
    virtual std::string format(const UserData &user) = 0;
    virtual std::string format(const GroupData &group) = 0;
    virtual std::string format(const DatasetData &dataset) = 0;
    virtual std::string format(const ResourceData &resource) = 0;
};

class TextFormatter : public OutputFormatter
{
public:
    std::string format(const UserData &user) override;
    std::string format(const GroupData &group) override;
    std::string format(const DatasetData &dataset) override;
    std::string format(const ResourceData &resource) override;
};

class JsonFormatter : public OutputFormatter
{
public:
    std::string format(const UserData &user) override;
    std::string format(const GroupData &group) override;
    std::string format(const DatasetData &dataset) override;
    std::string format(const ResourceData &resource) override;
};

#endif // OUTPUT_FORMATTER_HPP
