#include "../include/module_manager.hpp"
#include "../include/modules/hamhook.hpp"
#include "../include/modules/hamc.hpp"
#include <iostream>
#include <iomanip>

namespace Hammer {

ModuleManager::ModuleManager() {
    registerModule(std::make_shared<HamHookModule>());
    registerModule(std::make_shared<HamCModule>());
}

void ModuleManager::registerModule(std::shared_ptr<CompilerModule> module) {
    if (module) {
        modules[module->getName()] = module;
    }
}

std::shared_ptr<CompilerModule> ModuleManager::getModule(const std::string& name) const {
    auto it = modules.find(name);
    if (it != modules.end()) {
        return it->second;
    }
    return nullptr;
}

bool ModuleManager::hasModule(const std::string& name) const {
    return modules.find(name) != modules.end();
}

std::vector<std::shared_ptr<CompilerModule>> ModuleManager::getAllModules() const {
    std::vector<std::shared_ptr<CompilerModule>> list;
    for (const auto& pair : modules) {
        list.push_back(pair.second);
    }
    return list;
}

void ModuleManager::listModules() const {
    const std::string CYAN = "\033[1;36m";
    const std::string YELLOW = "\033[1;33m";
    const std::string BOLD = "\033[1m";
    const std::string RESET = "\033[0m";

    std::cout << "\n" << BOLD << "Available Hammer Script Compilation Modules:" << RESET << "\n";
    std::cout << "--------------------------------------------------------------------------------\n";

    for (const auto& pair : modules) {
        auto mod = pair.second;
        std::cout << "  " << CYAN << std::left << std::setw(10) << mod->getName() << RESET;
        std::cout << " [" << YELLOW << mod->getDisplayName() << RESET << "]\n";
        std::cout << "             " << mod->getDescription() << "\n\n";
    }
    std::cout << "Use with: hammer build --module <name> <file.hmscp> -o <output>\n\n";
}

} // namespace Hammer
