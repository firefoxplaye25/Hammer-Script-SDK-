#ifndef HAMMER_MODULE_HAMC_HPP
#define HAMMER_MODULE_HAMC_HPP

#include "../compiler_module.hpp"

namespace Hammer {

class HamCModule : public CompilerModule {
public:
    HamCModule() = default;

    std::string getName() const override { return "hamc"; }
    std::string getDisplayName() const override { return "C++ Source Transpiler"; }
    std::string getDescription() const override {
        return "Transpiles Hammer script AST into standalone, human-readable modern C++17 source code.";
    }

    bool compile(
        const std::vector<std::shared_ptr<Stmt>>& ast,
        const std::string& sourcePath,
        const ModuleOptions& options
    ) override;
};

} // namespace Hammer

#endif // HAMMER_MODULE_HAMC_HPP
