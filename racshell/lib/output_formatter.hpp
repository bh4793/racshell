#ifndef OUTPUT_FORMATTER_HPP
#define OUTPUT_FORMATTER_HPP

#include <string>
#include "user_data.hpp"
#include "group_data.hpp"

class OutputFormatter {
public:
    virtual ~OutputFormatter() = default;
    virtual std::string format(const UserData& user) = 0;
    virtual std::string format(const GroupData& group) = 0;
};

class TextFormatter : public OutputFormatter {
public:
    std::string format(const UserData& user) override;
    std::string format(const GroupData& group) override;
};

class JsonFormatter : public OutputFormatter {
public:
    std::string format(const UserData& user) override;
    std::string format(const GroupData& group) override;
};

#endif // OUTPUT_FORMATTER_HPP
