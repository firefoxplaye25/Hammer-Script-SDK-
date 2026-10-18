#ifndef HAMMER_LIB_LOADER_HPP
#define HAMMER_LIB_LOADER_HPP

#include <string>
#include <vector>
#include <memory>
#include "environment.hpp"

namespace Hammer {

struct PackageInfo {
    std::string name;
    std::string author;
    std::string version;
    std::string description;
    std::string entryFile;
    bool isMonolithiiun = false;
    std::vector<std::string> exportedFunctions;
};

class LibLoader {
public:
    static std::string getLibLoaderDir();
    static void ensureDirectoryStructure();
    
    // Package installation commands
    static bool installPackage(const std::string& author, const std::string& packageName, bool isMonolithiiun);
    static bool uninstallPackage(const std::string& packageName);
    static void listInstalledPackages();

    // Resolving library file path for include or load_lib
    static std::string resolveLibPath(const std::string& name);

    // Library creation and auto-loader
    static bool createNewLibrary(const std::string& packageName, const std::string& author = "");
    static void autoLoadInstalledModules(std::shared_ptr<Environment> env);
    static std::string getAllInstalledModulesSource();

    // Register load_lib in runtime environment
    static void registerLibLoader(std::shared_ptr<Environment> env);
};

} // namespace Hammer

#endif // HAMMER_LIB_LOADER_HPP
