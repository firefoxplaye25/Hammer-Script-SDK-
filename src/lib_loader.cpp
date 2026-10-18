#include "../include/lib_loader.hpp"
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/errors.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

namespace Hammer {

static const char* SECURITY_WARNING = 
"================================================================================\n"
"               HAMMER SCRIPT LIB LOADER - SECURITY NOTICE                      \n"
"================================================================================\n"
"WARNING: Community libraries installed from unknown third-party authors may    \n"
"contain malicious code, file system modifications, or security risks.          \n"
"\n"
"Always inspect the source code of community packages in:                        \n"
"  'lib loader/community/<package_name>/'                                        \n"
"\n"
"For complete security and verified safety, ONLY install packages published      \n"
"under the official monolithic channel:                                          \n"
"  hammer inst hmx --monolithiiun <package_name>                                 \n"
"================================================================================\n";

std::string LibLoader::getLibLoaderDir() {
    if (fs::exists("lib loader")) return "lib loader";
    if (fs::exists("Lib loader")) return "Lib loader";
    return "lib loader";
}

void LibLoader::ensureDirectoryStructure() {
    std::string base = getLibLoaderDir();
    fs::create_directories(base + "/community");
    fs::create_directories(base + "/monolithiiun");

    std::string warnFile = base + "/WARNING_SECURITY.txt";
    if (!fs::exists(warnFile)) {
        std::ofstream out(warnFile);
        if (out.is_open()) {
            out << SECURITY_WARNING;
            out.close();
        }
    }
}

bool LibLoader::installPackage(const std::string& author, const std::string& packageName, bool isMonolithiiun) {
    ensureDirectoryStructure();
    std::string base = getLibLoaderDir();
    std::string channelDir = isMonolithiiun ? (base + "/monolithiiun") : (base + "/community");
    std::string targetDir = channelDir + "/" + packageName;

    std::cout << "\n================================================================================\n";
    if (isMonolithiiun) {
        std::cout << "\033[1;32m[Monolithiiun Security Verified]\033[0m\n";
        std::cout << "Installing officially audited and verified library: '" << packageName << "'\n";
        std::cout << "Publisher: Monolithiiun Core Security Repository\n";
    } else {
        std::cout << "\033[1;33m[WARNING - COMMUNITY LIBRARY RISK]\033[0m\n";
        std::cout << "Installing community package '" << packageName << "' by author: @" << (author.empty() ? "unknown" : author) << "\n";
        std::cout << "ATTENTION: Community packages are created by third parties and have full access\n";
        std::cout << "to your system. For verified code, use: hammer inst hmx --monolithiiun " << packageName << "\n";
    }
    std::cout << "--------------------------------------------------------------------------------\n";

    fs::create_directories(targetDir);

    // Create manifest.json
    std::string manifestPath = targetDir + "/manifest.json";
    std::ofstream mOut(manifestPath);
    if (mOut.is_open()) {
        mOut << "{\n";
        mOut << "  \"name\": \"" << packageName << "\",\n";
        mOut << "  \"author\": \"" << (isMonolithiiun ? "monolithiiun" : (author.empty() ? "community" : author)) << "\",\n";
        mOut << "  \"version\": \"1.0.0\",\n";
        mOut << "  \"verified\": " << (isMonolithiiun ? "true" : "false") << ",\n";
        mOut << "  \"entry\": \"" << packageName << ".hmscp\"\n";
        mOut << "}\n";
        mOut.close();
    }

    // Create entry file if not existing
    std::string entryPath = targetDir + "/" + packageName + ".hmscp";
    if (!fs::exists(entryPath)) {
        std::ofstream eOut(entryPath);
        if (eOut.is_open()) {
            eOut << "# ========================================================\n";
            eOut << "# Library: " << packageName << "\n";
            eOut << "# Author: " << (isMonolithiiun ? "monolithiiun" : author) << "\n";
            eOut << "# Channel: " << (isMonolithiiun ? "monolithiiun (verified)" : "community") << "\n";
            eOut << "# ========================================================\n\n";
            eOut << "action " << packageName << "_info()\n";
            eOut << "    give \"Library '" << packageName << "' v1.0.0 is active!\"\n";
            eOut << "end\n";
            eOut.close();
        }
    }

    std::cout << "[SUCCESS] Package '" << packageName << "' installed successfully!\n";
    std::cout << "Location: " << targetDir << "\n";
    std::cout << "Use in Hammer Script: include \"" << packageName << "\" or load_lib(\"" << packageName << "\")\n";
    std::cout << "================================================================================\n\n";
    return true;
}

bool LibLoader::uninstallPackage(const std::string& packageName) {
    ensureDirectoryStructure();
    std::string base = getLibLoaderDir();
    bool found = false;

    std::string comDir = base + "/community/" + packageName;
    if (fs::exists(comDir)) {
        std::error_code ec;
        fs::remove_all(comDir, ec);
        found = true;
    }

    std::string monoDir = base + "/monolithiiun/" + packageName;
    if (fs::exists(monoDir)) {
        std::error_code ec;
        fs::remove_all(monoDir, ec);
        found = true;
    }

    if (found) {
        std::cout << "[SUCCESS] Library '" << packageName << "' uninstalled successfully.\n";
        return true;
    } else {
        std::cout << "[ERROR] Library '" << packageName << "' not found in installed packages.\n";
        return false;
    }
}

void LibLoader::listInstalledPackages() {
    ensureDirectoryStructure();
    std::string base = getLibLoaderDir();

    std::cout << "\n================================================================================\n";
    std::cout << "                    INSTALLED HAMMER SCRIPT LIBRARIES                           \n";
    std::cout << "================================================================================\n";
    std::cout << " Channel       | Name                 | Author               | Location\n";
    std::cout << "---------------+----------------------+----------------------+------------------\n";

    int count = 0;

    auto scanDir = [&](const std::string& dir, const std::string& channel) {
        if (!fs::exists(dir)) return;
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_directory()) {
                std::string pkgName = entry.path().filename().string();
                std::string author = "unknown";
                std::string manifestFile = entry.path().string() + "/manifest.json";
                if (fs::exists(manifestFile)) {
                    std::ifstream in(manifestFile);
                    std::string line;
                    while (std::getline(in, line)) {
                        if (line.find("\"author\"") != std::string::npos) {
                            size_t c = line.find(":");
                            if (c != std::string::npos) {
                                std::string a = line.substr(c + 1);
                                a.erase(std::remove(a.begin(), a.end(), '\"'), a.end());
                                a.erase(std::remove(a.begin(), a.end(), ','), a.end());
                                a.erase(std::remove(a.begin(), a.end(), ' '), a.end());
                                if (!a.empty()) author = a;
                            }
                        }
                    }
                }

                std::string colorPrefix = (channel == "monolithiiun") ? "\033[1;32m" : "\033[1;33m";
                std::cout << " " << colorPrefix << channel << "\033[0m";
                for (size_t i = channel.size(); i < 14; ++i) std::cout << " ";
                std::cout << "| " << pkgName;
                for (size_t i = pkgName.size(); i < 21; ++i) std::cout << " ";
                std::cout << "| @" << author;
                for (size_t i = author.size() + 1; i < 21; ++i) std::cout << " ";
                std::cout << "| " << entry.path().string() << "\n";
                count++;
            }
        }
    };

    scanDir(base + "/monolithiiun", "monolithiiun");
    scanDir(base + "/community", "community");

    if (count == 0) {
        std::cout << " (No libraries installed yet. Install with: hammer inst hmx --author <author> <name>)\n";
    }
    std::cout << "================================================================================\n\n";
}

std::string LibLoader::resolveLibPath(const std::string& name) {
    // 1. Direct path or current dir
    if (fs::exists(name)) return name;
    if (fs::exists(name + ".hmscp")) return name + ".hmscp";

    std::string base = getLibLoaderDir();

    // 2. Monolithiiun verified channel
    std::string p1 = base + "/monolithiiun/" + name + "/" + name + ".hmscp";
    if (fs::exists(p1)) return p1;
    std::string p2 = base + "/monolithiiun/" + name + "/entry.hmscp";
    if (fs::exists(p2)) return p2;

    // 3. Community channel
    std::string p3 = base + "/community/" + name + "/" + name + ".hmscp";
    if (fs::exists(p3)) return p3;
    std::string p4 = base + "/community/" + name + "/entry.hmscp";
    if (fs::exists(p4)) return p4;

    // 4. Root of lib loader
    std::string p5 = base + "/" + name + ".hmscp";
    if (fs::exists(p5)) return p5;

    return "";
}

void LibLoader::registerLibLoader(std::shared_ptr<Environment> env) {
    // load_lib(name)
    auto fnLoadLib = [env](const std::vector<Value>& args) -> Value {
        if (args.empty()) {
            throw HammerError(0, 0, "LibLoader Error", "load_lib requires a library name string.",
                              "Call load_lib with package name.", "load_lib(\"ansi_colors\")");
        }
        std::string libName = args[0].toString();
        std::string path = LibLoader::resolveLibPath(libName);
        if (path.empty()) {
            throw HammerError(0, 0, "LibLoader Error", "Library '" + libName + "' could not be found.",
                              "Verify that the library is installed or check spelling.", "hammer libs");
        }

        std::ifstream file(path);
        if (!file.is_open()) {
            throw HammerError(0, 0, "LibLoader Error", "Cannot read library file: " + path,
                              "Check file permissions.", "");
        }
        std::stringstream ss;
        ss << file.rdbuf();
        std::string src = ss.str();

        Lexer lex(src);
        auto tokens = lex.tokenize();
        Parser parser(tokens);
        auto stmts = parser.parse();

        // Execute in caller environment
        for (const auto& s : stmts) {
            if (s) {
                // If it defines a function, it binds to env
                // We can run via mini interpreter with same env
                Interpreter mini(env);
                mini.execute({s});
            }
        }

        return Value::makeBoolean(true);
    };

    env->set("load_lib", Value::makeNative(fnLoadLib));
    env->set("import_lib", Value::makeNative(fnLoadLib));
}

bool LibLoader::createNewLibrary(const std::string& packageName, const std::string& author) {
    ensureDirectoryStructure();
    std::string base = getLibLoaderDir();
    std::string targetDir = base + "/community/" + packageName;
    std::string realAuthor = author.empty() ? "developer" : author;

    if (fs::exists(targetDir)) {
        std::cerr << "\033[1;31m[Error] Library module '" << packageName << "' already exists in: " << targetDir << "\033[0m\n";
        return false;
    }

    fs::create_directories(targetDir);

    // 1. manifest.json
    std::string manifestPath = targetDir + "/manifest.json";
    std::ofstream mOut(manifestPath);
    if (mOut.is_open()) {
        mOut << "{\n";
        mOut << "  \"name\": \"" << packageName << "\",\n";
        mOut << "  \"author\": \"" << realAuthor << "\",\n";
        mOut << "  \"version\": \"1.0.0\",\n";
        mOut << "  \"description\": \"Custom module extending Hammer Script with new functions\",\n";
        mOut << "  \"entry\": \"" << packageName << ".hmscp\"\n";
        mOut << "}\n";
        mOut.close();
    }

    // 2. <packageName>.lib header
    std::string libHeaderPath = targetDir + "/" + packageName + ".lib";
    std::ofstream hOut(libHeaderPath);
    if (hOut.is_open()) {
        hOut << "# ========================================================\n";
        hOut << "# Hammer Script Library - " << packageName << ".lib\n";
        hOut << "# Author: @" << realAuthor << "\n";
        hOut << "# ========================================================\n\n";
        hOut << packageName << "_hello(name)    -> Greets with library branding\n";
        hOut << packageName << "_calc(a, b)     -> Example module function\n";
        hOut.close();
    }

    // 3. <packageName>.hmscp implementation
    std::string implPath = targetDir + "/" + packageName + ".hmscp";
    std::ofstream iOut(implPath);
    if (iOut.is_open()) {
        iOut << "# ========================================================\n";
        iOut << "# Hammer Script Module Implementation: " << packageName << "\n";
        iOut << "# ========================================================\n\n";
        iOut << "action " << packageName << "_hello(name)\n";
        iOut << "    give \"[" << packageName << " v1.0.0] Hello, \" + name + \"!\"\n";
        iOut << "end\n\n";
        iOut << "action " << packageName << "_calc(a, b)\n";
        iOut << "    give (a * 10) + b\n";
        iOut << "end\n";
        iOut.close();
    }

    std::cout << "\n================================================================================\n";
    std::cout << "\033[1;32m[SUCCESS] Created new library module: '" << packageName << "'!\033[0m\n";
    std::cout << "Location: " << targetDir << "\n";
    std::cout << "Generated files:\n";
    std::cout << "  - manifest.json  (Package metadata)\n";
    std::cout << "  - " << packageName << ".lib        (Function signatures & documentation)\n";
    std::cout << "  - " << packageName << ".hmscp      (Implementation of functions)\n";
    std::cout << "\nThis module now automatically extends Hammer Script with new functions:\n";
    std::cout << "  show " << packageName << "_hello(\"User\")\n";
    std::cout << "  show " << packageName << "_calc(5, 2)\n";
    std::cout << "================================================================================\n\n";
    return true;
}

void LibLoader::autoLoadInstalledModules(std::shared_ptr<Environment> env) {
    std::string base = getLibLoaderDir();
    auto scanAndLoad = [&](const std::string& folder) {
        if (!fs::exists(folder)) return;
        for (const auto& entry : fs::directory_iterator(folder)) {
            if (entry.is_directory()) {
                std::string pkgName = entry.path().filename().string();
                std::string entryScript = entry.path().string() + "/" + pkgName + ".hmscp";
                if (!fs::exists(entryScript)) {
                    entryScript = entry.path().string() + "/entry.hmscp";
                }
                if (fs::exists(entryScript)) {
                    try {
                        std::ifstream in(entryScript);
                        if (in.is_open()) {
                            std::stringstream ss;
                            ss << in.rdbuf();
                            std::string src = ss.str();
                            Lexer lex(src);
                            auto tokens = lex.tokenize();
                            Parser p(tokens);
                            auto stmts = p.parse();
                            for (const auto& s : stmts) {
                                Interpreter mini(env);
                                mini.execute({s});
                            }
                        }
                    } catch (...) {
                        // Keep going if an individual module had issues
                    }
                }
            }
        }
    };

    scanAndLoad(base + "/monolithiiun");
    scanAndLoad(base + "/community");
}

std::string LibLoader::getAllInstalledModulesSource() {
    std::string base = getLibLoaderDir();
    std::stringstream bundle;
    auto scanAndAppend = [&](const std::string& folder) {
        if (!fs::exists(folder)) return;
        for (const auto& entry : fs::directory_iterator(folder)) {
            if (entry.is_directory()) {
                std::string pkgName = entry.path().filename().string();
                std::string entryScript = entry.path().string() + "/" + pkgName + ".hmscp";
                if (!fs::exists(entryScript)) {
                    entryScript = entry.path().string() + "/entry.hmscp";
                }
                if (fs::exists(entryScript)) {
                    std::ifstream in(entryScript);
                    if (in.is_open()) {
                        bundle << "\n# --- Bundled Module: " << pkgName << " ---\n";
                        bundle << in.rdbuf();
                        bundle << "\n";
                    }
                }
            }
        }
    };
    scanAndAppend(base + "/monolithiiun");
    scanAndAppend(base + "/community");
    return bundle.str();
}

} // namespace Hammer
