#include "../include/environment.hpp"
#include "../include/errors.hpp"
#include "../include/diagnostics.hpp"

namespace Hammer {

Environment::Environment(std::shared_ptr<Environment> parent) : parent(std::move(parent)) {}

void Environment::set(const std::string& name, const Value& value) {
    bindings[name] = value;
}

void Environment::assign(const std::string& name, const Value& value, int line, int col) {
    if (bindings.find(name) != bindings.end()) {
        bindings[name] = value;
        return;
    }
    if (parent) {
        parent->assign(name, value, line, col);
        return;
    }
    // If not found in any parent, define in current scope
    bindings[name] = value;
}

bool Environment::exists(const std::string& name) const {
    if (bindings.find(name) != bindings.end()) return true;
    if (parent) return parent->exists(name);
    return false;
}

std::vector<std::string> Environment::getAllVariableNames() const {
    std::vector<std::string> names;
    for (const auto& pair : bindings) {
        names.push_back(pair.first);
    }
    if (parent) {
        auto parentNames = parent->getAllVariableNames();
        names.insert(names.end(), parentNames.begin(), parentNames.end());
    }
    return names;
}

Value Environment::get(const std::string& name, int line, int col) {
    auto it = bindings.find(name);
    if (it != bindings.end()) {
        return it->second;
    }
    if (parent) {
        return parent->get(name, line, col);
    }

    auto allVars = getAllVariableNames();
    std::string closest = DiagnosticEngine::findClosestMatch(name, allVars);

    std::string hint;
    if (!closest.empty()) {
        hint = "Did you mean '" + closest + "'?";
    } else {
        hint = "Make sure you create '" + name + "' first using: set " + name + " to <value>";
    }

    throw HammerError(
        line,
        col,
        "Name Error",
        "Variable '" + name + "' is not defined.",
        "You tried to use the variable '" + name + "', but Hammer Script couldn't find any variable with that name in this scope.",
        hint,
        "set " + name + " to 10\nshow " + name
    );
}

} // namespace Hammer
