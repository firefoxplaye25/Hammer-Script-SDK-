#ifndef HAMMER_ERRORS_HPP
#define HAMMER_ERRORS_HPP

#include <string>
#include <exception>

namespace Hammer {

class HammerError : public std::exception {
public:
    int line;
    int column;
    std::string errorType;
    std::string message;
    std::string explanation;
    std::string suggestion;
    std::string example;

    HammerError(
        int line,
        int column,
        std::string errorType,
        std::string message,
        std::string explanation,
        std::string suggestion,
        std::string example = ""
    ) : line(line),
        column(column),
        errorType(std::move(errorType)),
        message(std::move(message)),
        explanation(std::move(explanation)),
        suggestion(std::move(suggestion)),
        example(std::move(example)) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

} // namespace Hammer

#endif // HAMMER_ERRORS_HPP
