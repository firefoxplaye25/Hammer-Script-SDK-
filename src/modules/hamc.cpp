#include "../../include/modules/hamc.hpp"
#include "../../include/compiler.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace Hammer {

bool HamCModule::compile(
    const std::vector<std::shared_ptr<Stmt>>& ast,
    const std::string& sourcePath,
    const ModuleOptions& options
) {
    const std::string GREEN = "\033[1;32m";
    const std::string CYAN = "\033[1;36m";
    const std::string RESET = "\033[0m";

    std::string outputPath = options.outputPath;
    if (outputPath.empty()) {
        fs::path p(sourcePath);
        outputPath = p.stem().string() + ".cpp";
    }

    std::cout << "\n" << CYAN << "[HamC - C++ Source Transpiler]" << RESET << " Generating C++17 source code...\n";

    Compiler compiler;
    std::string cppCode = compiler.generateCpp(ast);

    std::ofstream out(outputPath);
    if (!out.is_open()) {
        std::cerr << "Error: Could not open output file: " << outputPath << "\n";
        return false;
    }
    out << cppCode;

    std::cout << GREEN << "[HamC Success] C++ source file generated: " << outputPath << RESET << "\n\n";
    return true;
}

} // namespace Hammer
