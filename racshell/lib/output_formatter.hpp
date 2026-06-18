#ifndef OUTPUT_FORMATTER_HPP
#define OUTPUT_FORMATTER_HPP

#include <string>
#include "user_data.hpp"

class OutputFormatter {
public:
    virtual ~OutputFormatter() = default;
    virtual std::string format(const UserData& user) = 0;
};

class TextFormatter : public OutputFormatter {
public:
    std::string format(const UserData& user) override;
};

class JsonFormatter : public OutputFormatter {
public:
    std::string format(const UserData& user) override;
};

#endif // OUTPUT_FORMATTER_HPP
