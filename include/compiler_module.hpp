#ifndef HAMMER_COMPILER_MODULE_HPP
#define HAMMER_COMPILER_MODULE_HPP

#include "ast.hpp"
#include <string>
#include <vector>
#include <memory>
#include <map>

namespace Hammer {

struct ModuleOptions {
    std::string outputPath;
    bool keepIntermediate = false;
    bool optimize = true;
    bool staticLink = true;
    bool verbose = false;
    std::map<std::string, std::string> extraParams;
};

class CompilerModule {
public:
    virtual ~CompilerModule() = default;

    virtual std::string getName() const = 0;        // e.g. "hamhook"
    virtual std::string getDisplayName() const = 0; // e.g. "Primitive EXE Compiler"
    virtual std::string getDescription() const = 0;

    virtual bool compile(
        const std::vector<std::shared_ptr<Stmt>>& ast,
        const std::string& sourcePath,
        const ModuleOptions& options
    ) = 0;
};

} // namespace Hammer

#endif // HAMMER_COMPILER_MODULE_HPP
