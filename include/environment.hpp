#ifndef HAMMER_ENVIRONMENT_HPP
#define HAMMER_ENVIRONMENT_HPP

#include "value.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace Hammer {

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr);

    void set(const std::string& name, const Value& value);
    void assign(const std::string& name, const Value& value, int line = 1, int col = 1);
    Value get(const std::string& name, int line = 1, int col = 1);
    bool exists(const std::string& name) const;

    std::vector<std::string> getAllVariableNames() const;

private:
    std::shared_ptr<Environment> parent;
    std::unordered_map<std::string, Value> bindings;
};

} // namespace Hammer

#endif // HAMMER_ENVIRONMENT_HPP
