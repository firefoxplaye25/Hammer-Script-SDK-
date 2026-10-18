#ifndef HAMMER_MODULE_MANAGER_HPP
#define HAMMER_MODULE_MANAGER_HPP

#include "compiler_module.hpp"
#include <map>
#include <memory>
#include <vector>

namespace Hammer {

class ModuleManager {
public:
    ModuleManager();

    void registerModule(std::shared_ptr<CompilerModule> module);
    std::shared_ptr<CompilerModule> getModule(const std::string& name) const;
    bool hasModule(const std::string& name) const;
    std::vector<std::shared_ptr<CompilerModule>> getAllModules() const;

    void listModules() const;

private:
    std::map<std::string, std::shared_ptr<CompilerModule>> modules;
};

} // namespace Hammer

#endif // HAMMER_MODULE_MANAGER_HPP
