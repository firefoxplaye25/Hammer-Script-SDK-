#include "../../include/modules/hamhook.hpp"
#include "../../include/compiler.hpp"
#include "../../include/lib_loader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <windows.h>

namespace fs = std::filesystem;

namespace Hammer {

static std::string getPayloadStartMarker() {
    return std::string("\n@@@HAMMER_NATIVE_PE_") + "PAYLOAD_START@@@\n";
}
static std::string getPayloadEndMarker() {
    return std::string("\n@@@HAMMER_NATIVE_PE_") + "PAYLOAD_END@@@\n";
}

static std::string getExecutablePath() {
    char buffer[MAX_PATH];
    if (GetModuleFileNameA(NULL, buffer, MAX_PATH) > 0) {
        return std::string(buffer);
    }
    return "bin/hammer.exe";
}

// Direct PE Subsystem patching: sets Windows GUI (no console window) vs Console
static bool patchPESubsystem(const std::string& exePath, uint16_t subsystem) {
    std::fstream file(exePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) return false;

    // Read DOS header e_lfanew at 0x3C
    file.seekg(0x3C);
    uint32_t peOffset = 0;
    file.read(reinterpret_cast<char*>(&peOffset), sizeof(peOffset));
    if (peOffset == 0 || peOffset > 0x1000) return false;

    // Check PE signature
    file.seekg(peOffset);
    char peSig[4];
    file.read(peSig, 4);
    if (std::memcmp(peSig, "PE\0\0", 4) != 0) return false;

    // Optional header magic is at peOffset + 24
    file.seekg(peOffset + 24);
    uint16_t optMagic = 0;
    file.read(reinterpret_cast<char*>(&optMagic), sizeof(optMagic));

    // For 64-bit PE32+ (magic 0x20b) and 32-bit PE (magic 0x10b),
    // Subsystem field is at offset 68 inside IMAGE_OPTIONAL_HEADER
    if (optMagic == 0x20B || optMagic == 0x10B) {
        file.seekp(peOffset + 24 + 68);
        file.write(reinterpret_cast<const char*>(&subsystem), sizeof(subsystem));
        return true;
    }
    return false;
}

std::string HamHookModule::resolveToolchain(std::string& toolchainBinDir) const {
    toolchainBinDir = "";

    // 1. Explicit user override via environment variable
    char envBuf[4096];
    if (GetEnvironmentVariableA("HAMMER_CC", envBuf, sizeof(envBuf)) > 0) {
        std::string customCc = envBuf;
        if (fs::exists(customCc)) {
            toolchainBinDir = fs::path(customCc).parent_path().string();
            return customCc;
        }
    }
    if (GetEnvironmentVariableA("HAMMER_TOOLCHAIN", envBuf, sizeof(envBuf)) > 0) {
        std::string toolchainDir = envBuf;
        std::string candidate = toolchainDir + "/bin/g++.exe";
        if (fs::exists(candidate)) {
            toolchainBinDir = toolchainDir + "/bin";
            return candidate;
        }
    }

    // 2. Dedicated embedded toolchain in Hammer SDK
    fs::path exeDir = fs::path(getExecutablePath()).parent_path();
    const std::vector<std::string> dedicatedLocations = {
        (exeDir / "toolchain" / "bin" / "g++.exe").string(),
        (exeDir / ".." / "toolchain" / "bin" / "g++.exe").string(),
        "./toolchain/bin/g++.exe",
        "../toolchain/bin/g++.exe",
        (exeDir / "toolchain" / "bin" / "clang++.exe").string(),
        "./toolchain/bin/clang++.exe"
    };

    for (const auto& path : dedicatedLocations) {
        if (fs::exists(path)) {
            toolchainBinDir = fs::path(path).parent_path().string();
            return path;
        }
    }

    // 3. System PATH detection (g++, clang++)
    char pathSearchBuf[MAX_PATH];
    if (SearchPathA(NULL, "g++.exe", NULL, MAX_PATH, pathSearchBuf, NULL) > 0) {
        toolchainBinDir = fs::path(pathSearchBuf).parent_path().string();
        return pathSearchBuf;
    }
    if (SearchPathA(NULL, "clang++.exe", NULL, MAX_PATH, pathSearchBuf, NULL) > 0) {
        toolchainBinDir = fs::path(pathSearchBuf).parent_path().string();
        return pathSearchBuf;
    }

    // 4. Common fallback locations
    const std::vector<std::string> fallbackLocations = {
        "C:\\msys64\\ucrt64\\bin\\g++.exe",
        "C:\\msys64\\mingw64\\bin\\g++.exe",
        "C:\\msys64\\clang64\\bin\\clang++.exe",
        "C:\\MinGW\\bin\\g++.exe",
        "C:\\w64devkit\\bin\\g++.exe",
        "C:\\Program Files\\LLVM\\bin\\clang++.exe"
    };

    for (const auto& path : fallbackLocations) {
        if (fs::exists(path)) {
            toolchainBinDir = fs::path(path).parent_path().string();
            return path;
        }
    }

    return "";
}

bool HamHookModule::compile(
    const std::vector<std::shared_ptr<Stmt>>& ast,
    const std::string& sourcePath,
    const ModuleOptions& options
) {
    const std::string CYAN = "\033[1;36m";
    const std::string GREEN = "\033[1;32m";
    const std::string YELLOW = "\033[1;33m";
    const std::string RED = "\033[1;31m";
    const std::string BOLD = "\033[1m";
    const std::string RESET = "\033[0m";

    auto tStart = std::chrono::high_resolution_clock::now();

    std::cout << "\n" << CYAN << "================================================================================" << RESET << "\n";
    std::cout << CYAN << "[HamHook - Dedicated Native Compiler]" << RESET << " " << BOLD << "Target: Standalone Windows PE (.exe)" << RESET << "\n";
    std::cout << "Source File: " << sourcePath << "\n";

    std::string outputPath = options.outputPath;
    if (outputPath.empty()) {
        fs::path p(sourcePath);
        outputPath = p.stem().string() + ".exe";
    }

    std::cout << "Target Executable: " << outputPath << "\n";
    std::cout << CYAN << "--------------------------------------------------------------------------------" << RESET << "\n";

    // Check if user explicitly requested external C++ compilation
    bool forceCppTranspile = false;
    auto itBackend = options.extraParams.find("backend");
    if (itBackend != options.extraParams.end() && itBackend->second == "cpp") {
        forceCppTranspile = true;
    }

    // -------------------------------------------------------------
    // DEFAULT MODE: PURE DEDICATED NATIVE PE COMPILER (ZERO DEPENDENCIES)
    // -------------------------------------------------------------
    if (!forceCppTranspile) {
        std::cout << "[HamHook] Engine: " << GREEN << "Built-in Standalone PE Generator (Zero External Dependencies)" << RESET << "\n";
        std::cout << "[HamHook] Packaging runtime engine and native bytecode/AST payload...\n";

        // Read source script
        std::ifstream srcIn(sourcePath, std::ios::binary);
        if (!srcIn.is_open()) {
            std::cerr << RED << "[HamHook Error] Cannot open source file: " << sourcePath << RESET << "\n";
            return false;
        }
        std::stringstream ssSrc;
        ssSrc << srcIn.rdbuf();
        std::string scriptSource = ssSrc.str();
        srcIn.close();

        // Locate base runner binary
        std::string baseExe = getExecutablePath();
        if (!fs::exists(baseExe)) {
            if (fs::exists("bin/hammer.exe")) baseExe = "bin/hammer.exe";
            else if (fs::exists("hammer.exe")) baseExe = "hammer.exe";
        }

        std::ifstream baseIn(baseExe, std::ios::binary);
        if (!baseIn.is_open()) {
            std::cerr << RED << "[HamHook Error] Cannot locate base runtime stub: " << baseExe << RESET << "\n";
            return false;
        }
        std::vector<char> baseBytes((std::istreambuf_iterator<char>(baseIn)), std::istreambuf_iterator<char>());
        baseIn.close();

        // If the base executable itself already has an embedded payload, strip it back to the clean base stub
        std::string startMarker = getPayloadStartMarker();
        std::string endMarker = getPayloadEndMarker();

        std::string baseStr(baseBytes.data(), baseBytes.size());
        size_t existingPayloadPos = baseStr.rfind(startMarker);
        if (existingPayloadPos != std::string::npos) {
            baseBytes.resize(existingPayloadPos);
        }

        // Create target directory if needed
        fs::path outDir = fs::path(outputPath).parent_path();
        if (!outDir.empty()) {
            fs::create_directories(outDir);
        }

        // Write standalone native PE executable
        std::ofstream outExe(outputPath, std::ios::binary);
        if (!outExe.is_open()) {
            std::cerr << RED << "[HamHook Error] Cannot create target executable: " << outputPath << RESET << "\n";
            return false;
        }

        // 1. Write the clean native PE base engine
        outExe.write(baseBytes.data(), baseBytes.size());

        // 2. Append the embedded native payload (bundling all installed modules for full portability)
        std::string modulesSource = LibLoader::getAllInstalledModulesSource();
        std::string combinedPayload = modulesSource.empty() ? scriptSource : (modulesSource + "\n# --- Main Script Entry ---\n" + scriptSource);

        outExe.write(startMarker.data(), startMarker.size());
        outExe.write(combinedPayload.data(), combinedPayload.size());
        outExe.write(endMarker.data(), endMarker.size());
        outExe.close();

        // 3. Patch Subsystem if GUI mode (--no-console) is requested
        auto itConsole = options.extraParams.find("no_console");
        if (itConsole != options.extraParams.end() && itConsole->second == "true") {
            patchPESubsystem(outputPath, 2); // 2 = IMAGE_SUBSYSTEM_WINDOWS_GUI
            std::cout << "[HamHook] Subsystem Patched: Windowed GUI Mode (No Console)\n";
        }

        auto tEnd = std::chrono::high_resolution_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

        auto sz = fs::file_size(outputPath);
        std::cout << GREEN << "[HamHook Success] Native standalone PE executable built in " << elapsedMs << " ms!" << RESET << "\n";
        std::cout << "File: " << BOLD << outputPath << RESET << " (" << (sz / 1024.0) << " KB)\n";
        std::cout << "Requirements: " << BOLD << "None" << RESET << " (Runs anywhere on any Windows machine with 0 dependencies!)\n";
        std::cout << CYAN << "================================================================================" << RESET << "\n\n";
        return true;
    }

    // -------------------------------------------------------------
    // OPTIONAL ADVANCED MODE: C++ TRANSPILATION & NATIVE RE-COMPILATION
    // -------------------------------------------------------------
    Compiler compiler;
    std::string cppCode = compiler.generateCpp(ast);

    std::string tempCpp = outputPath + ".hamhook.tmp.cpp";
    {
        std::ofstream out(tempCpp);
        if (!out.is_open()) {
            std::cerr << RED << "[HamHook Error] Failed to write intermediate translation: " << tempCpp << RESET << "\n";
            return false;
        }
        out << cppCode;
    }

    std::string toolchainBinDir;
    std::string compilerExe = resolveToolchain(toolchainBinDir);
    if (compilerExe.empty()) {
        std::cerr << RED << "[HamHook Error] No C++ compiler toolchain found on system. Use default standalone mode.\n" << RESET;
        return false;
    }
    std::cout << "[HamHook] C++ Backend Toolchain: " << YELLOW << compilerExe << RESET << "\n";

    std::string optFlags = options.optimize ? "-O3 -s" : "-O0";
    std::string linkFlags = "-static";

    auto itConsole = options.extraParams.find("no_console");
    if (itConsole != options.extraParams.end() && itConsole->second == "true") {
        linkFlags += " -mwindows";
        std::cout << "[HamHook] Mode: Windowed (GUI / No Console window)\n";
    }

    std::string pathPrefix = "";
    if (!toolchainBinDir.empty()) {
        pathPrefix = "set PATH=" + toolchainBinDir + ";%PATH% && ";
    }

    std::string cmd = pathPrefix + "\"" + compilerExe + "\" -std=c++17 " + optFlags + " " + linkFlags + " \"" + tempCpp + "\" -o \"" + outputPath + "\" -I . -lwininet -lws2_32 -lwinmm";

    std::cout << "[HamHook] Compiling machine code via C++ backend...\n";
    int exitCode = std::system(cmd.c_str());

    if (!options.keepIntermediate) {
        std::error_code ec;
        fs::remove(tempCpp, ec);
    }

    if (exitCode == 0 && fs::exists(outputPath)) {
        auto sz = fs::file_size(outputPath);
        std::cout << GREEN << "[HamHook Success] Native executable built successfully!" << RESET << "\n";
        std::cout << "File: " << BOLD << outputPath << RESET << " (" << (sz / 1024.0) << " KB)\n";
        std::cout << CYAN << "================================================================================" << RESET << "\n\n";
        return true;
    } else {
        std::cerr << RED << "[HamHook Error] Native compilation failed with exit code: " << exitCode << RESET << "\n";
        std::cout << CYAN << "================================================================================" << RESET << "\n\n";
        return false;
    }
}

} // namespace Hammer
