#ifndef HAMMER_MODULE_HAMHOOK_HPP
#define HAMMER_MODULE_HAMHOOK_HPP

#include "../compiler_module.hpp"

namespace Hammer {

class HamHookModule : public CompilerModule {
public:
    HamHookModule() = default;

    std::string getName() const override { return "hamhook"; }
    std::string getDisplayName() const override { return "Dedicated Native PE Compiler"; }
    std::string getDescription() const override {
        return "Compiles Hammer script directly into an optimized, standalone Windows PE executable (.exe) using the dedicated native compiler backend.";
    }

    bool compile(
        const std::vector<std::shared_ptr<Stmt>>& ast,
        const std::string& sourcePath,
        const ModuleOptions& options
    ) override;

private:
    std::string resolveToolchain(std::string& toolchainBinDir) const;
};

} // namespace Hammer

#endif // HAMMER_MODULE_HAMHOOK_HPP
