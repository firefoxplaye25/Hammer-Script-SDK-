#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include "../include/interpreter.hpp"
#include "../include/compiler.hpp"
#include "../include/diagnostics.hpp"
#include "../include/errors.hpp"
#include "../include/module_manager.hpp"
#include "../include/lib_loader.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <cstring>
#include <windows.h>

namespace fs = std::filesystem;
using namespace Hammer;

static void printBanner() {
    std::cout << "\033[1;36m";
    std::cout << "  _    _                                   ____            _       _   \n";
    std::cout << " | |  | |                                 / ____|         (_)     | |  \n";
    std::cout << " | |__| | __ _ _ __ ___  _ __ ___   ___  | (___   ___ _ __ _ _ __ | |_ \n";
    std::cout << " |  __  |/ _` | '_ ` _ \\| '_ ` _ \\ / _ \\  \\___ \\ / __| '__| | '_ \\| __|\n";
    std::cout << " | |  | | (_| | | | | | | | | | | |  __/  ____) | (__| |  | | |_) | |_ \n";
    std::cout << " |_|  |_|\\__,_|_| |_| |_|_| |_| |_|\\___| |_____/ \\___|_|  |_| .__/ \\__|\n";
    std::cout << "                                                            | |        \n";
    std::cout << "                                                            |_|        \n";
    std::cout << "\033[0m";
    std::cout << "\033[1;32mHammer Script v1.0.0 (Dedicated Native Edition)\033[0m - The friendly language easier than Python.\n";
    std::cout << "Type 'exit' or press Ctrl+C to quit. Try: show \"Hello Hammer!\"\n\n";
}

static void printHelp() {
    std::cout << "Usage:\n";
    std::cout << "  hammer run <file.hmscp>                 Run a Hammer script directly (Interpreter)\n";
    std::cout << "  hammer hamhook <file.hmscp> [-o out]   Compile with HamHook (Dedicated Native Standalone PE Compiler)\n";
    std::cout << "  hammer build <file.hmscp> [-o out]     Compile script into standalone .exe\n";
    std::cout << "  hammer build --module <name> <file>    Compile script using specific module (hamhook, hamc)\n";
    std::cout << "  hammer modules                         List available compilation modules\n";
    std::cout << "  hammer check <file.hmscp>               Check file for syntax errors and get suggestions\n";
    std::cout << "  hammer inst hmx --author <author> <pkg> Install community library (with security notice)\n";
    std::cout << "  hammer inst hmx --monolithiiun <pkg>    Install official verified monolithic library\n";
    std::cout << "  hammer create-lib <lib-name>           Create a new library module extending the language\n";
    std::cout << "  hammer libs                             List installed community & monolithic libraries\n";
    std::cout << "  hammer remove-lib <package-name>       Uninstall a library\n";
    std::cout << "  hammer repl                            Start interactive shell\n";
    std::cout << "  hammer new <project-name>              Create a new starter Hammer script project\n";
    std::cout << "  hammer --version                       Show version information\n";
}

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static bool runScript(const std::string& filepath) {
    std::string source;
    try {
        source = readFile(filepath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }

    DiagnosticEngine diag(filepath, source);

    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto stmts = parser.parse();

        Interpreter interpreter;
        interpreter.execute(stmts);
        return true;
    } catch (const HammerError& err) {
        diag.reportError(err.line, err.column, err.errorType, err.message, err.explanation, err.suggestion, err.example);
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Internal error: " << e.what() << "\n";
        return false;
    }
}

static bool checkScript(const std::string& filepath) {
    std::string source;
    try {
        source = readFile(filepath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }

    DiagnosticEngine diag(filepath, source);

    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto stmts = parser.parse();
        std::cout << "\033[1;32m[Hammer Check Success] No syntax errors found in '" << filepath << "'.\033[0m\n";
        return true;
    } catch (const HammerError& err) {
        diag.reportError(err.line, err.column, err.errorType, err.message, err.explanation, err.suggestion, err.example);
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Internal error: " << e.what() << "\n";
        return false;
    }
}

static bool buildWithModule(const std::string& moduleName, const std::string& sourcePath, const ModuleOptions& options) {
    ModuleManager mgr;
    auto mod = mgr.getModule(moduleName);
    if (!mod) {
        std::cerr << "Error: Unknown compiler module '" << moduleName << "'.\n";
        std::cerr << "Run 'hammer modules' to view available modules.\n";
        return false;
    }

    std::string source;
    try {
        source = readFile(sourcePath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return false;
    }

    DiagnosticEngine diag(sourcePath, source);
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto stmts = parser.parse();

        return mod->compile(stmts, sourcePath, options);
    } catch (const HammerError& err) {
        diag.reportError(err.line, err.column, err.errorType, err.message, err.explanation, err.suggestion, err.example);
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Internal error during compilation: " << e.what() << "\n";
        return false;
    }
}

static void runRepl() {
    printBanner();
    auto env = std::make_shared<Environment>();
    Interpreter interpreter(env);

    std::string line;
    while (true) {
        std::cout << "\033[1;33mhammer>\033[0m ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;

        DiagnosticEngine diag("<repl>", line);
        try {
            Lexer lexer(line);
            auto tokens = lexer.tokenize();
            Parser parser(tokens);
            auto stmts = parser.parse();
            interpreter.execute(stmts);
        } catch (const HammerError& err) {
            diag.reportError(err.line, err.column, err.errorType, err.message, err.explanation, err.suggestion, err.example);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

static void createNewProject(const std::string& name) {
    fs::create_directories(name);
    std::string mainFile = name + "/main.hmscp";
    std::ofstream out(mainFile);
    out << "# Welcome to " << name << " in Hammer Script!\n\n";
    out << "show \"=== Welcome to " << name << " ===\"\n\n";
    out << "set player to ask \"What is your name? \"\n";
    out << "show \"Hello, \" + player + \"! Welcome to Hammer Script!\"\n\n";
    out << "repeat 3 times\n";
    out << "    show \"Hammer is simple and powerful.\"\n";
    out << "end\n";
    std::cout << "\033[1;32mCreated new Hammer project in '" << name << "/'.\033[0m\n";
    std::cout << "Run it using:\n  hammer run " << mainFile << "\n";
}

static std::string getPayloadStartMarker() {
    return std::string("\n@@@HAMMER_NATIVE_PE_") + "PAYLOAD_START@@@\n";
}
static std::string getPayloadEndMarker() {
    return std::string("\n@@@HAMMER_NATIVE_PE_") + "PAYLOAD_END@@@\n";
}

// Check if this binary was compiled with an embedded script payload
static bool checkAndExecuteEmbeddedPayload(int argc, char* argv[]) {
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) == 0) return false;

    std::ifstream file(exePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;

    std::streamsize fileSize = file.tellg();
    if (fileSize < 1024) return false;

    // Search the last 4MB (or entire file if smaller) for embedded payload marker
    std::streamsize searchSize = std::min<std::streamsize>(fileSize, 4 * 1024 * 1024);
    file.seekg(fileSize - searchSize);
    std::string buffer(searchSize, '\0');
    file.read(&buffer[0], searchSize);
    file.close();

    std::string startMarker = getPayloadStartMarker();
    std::string endMarker = getPayloadEndMarker();

    size_t startPos = buffer.rfind(startMarker);
    if (startPos == std::string::npos) return false;

    startPos += startMarker.size();
    size_t endPos = buffer.find(endMarker, startPos);
    if (endPos == std::string::npos) return false;

    std::string scriptSource = buffer.substr(startPos, endPos - startPos);

    // Execute the embedded script directly!
    try {
        Lexer lexer(scriptSource);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto stmts = parser.parse();

        Interpreter interpreter;
        interpreter.execute(stmts);
        return true;
    } catch (const HammerError& err) {
        DiagnosticEngine diag(exePath, scriptSource);
        diag.reportError(err.line, err.column, err.errorType, err.message, err.explanation, err.suggestion, err.example);
        exit(1);
    } catch (const std::exception& e) {
        std::cerr << "Native Runtime Error: " << e.what() << "\n";
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    // 1. If this executable is a compiled standalone native PE binary, run embedded payload directly!
    if (checkAndExecuteEmbeddedPayload(argc, argv)) {
        return 0;
    }

    // Direct invocation via hamhook.exe or hamc.exe binary name
    std::string progName = fs::path(argv[0]).stem().string();
    for (auto& c : progName) c = tolower(c);

    if (progName == "hamhook") {
        if (argc < 2) {
            std::cerr << "Usage: hamhook <file.hmscp> [-o output.exe] [--no-console] [--icon app.ico]\n";
            return 1;
        }
        std::string input = argv[1];
        ModuleOptions options;
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc) {
                options.outputPath = argv[++i];
            } else if (arg == "--backend" && i + 1 < argc) {
                options.extraParams["backend"] = argv[++i];
            } else if (arg == "--keep-cpp") {
                options.keepIntermediate = true;
            } else if (arg == "--no-console" || arg == "-w") {
                options.extraParams["no_console"] = "true";
            } else if (arg == "--icon" && i + 1 < argc) {
                options.extraParams["icon"] = argv[++i];
            }
        }
        return buildWithModule("hamhook", input, options) ? 0 : 1;
    }

    if (progName == "hamc") {
        if (argc < 2) {
            std::cerr << "Usage: hamc <file.hmscp> [-o output.exe] [--keep-cpp] [--no-opt]\n";
            return 1;
        }
        std::string input = argv[1];
        ModuleOptions options;
        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc) {
                options.outputPath = argv[++i];
            } else if (arg == "--backend" && i + 1 < argc) {
                options.extraParams["backend"] = argv[++i];
            } else if (arg == "--keep-cpp") {
                options.keepIntermediate = true;
            } else if (arg == "--no-opt") {
                options.optimize = false;
            } else if (arg == "--no-console" || arg == "-w") {
                options.extraParams["no_console"] = "true";
            } else if (arg == "--icon" && i + 1 < argc) {
                options.extraParams["icon"] = argv[++i];
            }
        }
        return buildWithModule("hamc", input, options) ? 0 : 1;
    }

    // 2. Otherwise run CLI mode
    if (argc < 2) {
        runRepl();
        return 0;
    }

    std::string command = argv[1];

    if (command == "--help" || command == "-h" || command == "help") {
        printHelp();
        return 0;
    }

    if (command == "--version" || command == "-v" || command == "version") {
        std::cout << "Hammer Script v1.0.0 (Dedicated Native Standalone Engine)\n";
        std::cout << "Default Native Module: HamHook (Dedicated Standalone PE Compiler - 0 Dependencies)\n";
        return 0;
    }

    if (command == "modules" || command == "backends") {
        ModuleManager mgr;
        mgr.listModules();
        return 0;
    }

    if (command == "libs" || command == "list-libs") {
        LibLoader::listInstalledPackages();
        return 0;
    }

    if (command == "remove-lib" || command == "uninstall") {
        if (argc < 3) {
            std::cerr << "Usage: hammer remove-lib <package-name>\n";
            return 1;
        }
        return LibLoader::uninstallPackage(argv[2]) ? 0 : 1;
    }

    // Package manager installation: hammer inst hmx ... or hammer install ...
    if (command == "inst" || command == "install") {
        // Syntax examples:
        // hammer inst hmx --author <author> <pkg>
        // hammer inst hmx --author; <pkg>
        // hammer inst hmx --monolithiiun <pkg>
        bool isMonolithiiun = false;
        std::string author = "";
        std::string pkg = "";

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "hmx") continue;
            if (arg == "--monolithiiun" || arg == "-m" || arg == "--monolithiiun;") {
                isMonolithiiun = true;
            } else if (arg == "--author" || arg == "-a") {
                if (i + 1 < argc) author = argv[++i];
            } else if (arg.rfind("--author;", 0) == 0) {
                // Format: --author; pkg
                author = "community";
            } else if (pkg.empty()) {
                pkg = arg;
            }
        }

        if (pkg.empty()) {
            std::cerr << "Usage: hammer inst hmx --author <author> <package-name>\n";
            std::cerr << "   or: hammer inst hmx --monolithiiun <package-name>\n";
            return 1;
        }

        return LibLoader::installPackage(author, pkg, isMonolithiiun) ? 0 : 1;
    }

    if (command == "create-lib" || command == "new-lib") {
        if (argc < 3) {
            std::cerr << "Usage: hammer create-lib <library-name> [--author <name>]\n";
            return 1;
        }
        std::string libName = argv[2];
        std::string author = "developer";
        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "--author" || arg == "-a") && i + 1 < argc) {
                author = argv[++i];
            }
        }
        return LibLoader::createNewLibrary(libName, author) ? 0 : 1;
    }

    if (command == "repl") {
        runRepl();
        return 0;
    }

    if (command == "new") {
        if (argc < 3) {
            std::cerr << "Usage: hammer new <project-name>\n";
            return 1;
        }
        createNewProject(argv[2]);
        return 0;
    }

    if (command == "check") {
        if (argc < 3) {
            std::cerr << "Usage: hammer check <file.hmscp>\n";
            return 1;
        }
        return checkScript(argv[2]) ? 0 : 1;
    }

    // Direct hamhook command shortcut: 'hammer hamhook file.hmscp'
    if (command == "hamhook") {
        if (argc < 3) {
            std::cerr << "Usage: hammer hamhook <file.hmscp> [-o output.exe] [--no-console] [--icon app.ico]\n";
            return 1;
        }
        std::string input = argv[2];
        ModuleOptions options;
        for (int i = 3; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-o" && i + 1 < argc) {
                options.outputPath = argv[++i];
            } else if (arg == "--backend" && i + 1 < argc) {
                options.extraParams["backend"] = argv[++i];
            } else if (arg == "--keep-cpp") {
                options.keepIntermediate = true;
            } else if (arg == "--no-console" || arg == "-w") {
                options.extraParams["no_console"] = "true";
            } else if (arg == "--icon" && i + 1 < argc) {
                options.extraParams["icon"] = argv[++i];
            }
        }
        return buildWithModule("hamhook", input, options) ? 0 : 1;
    }

    if (command == "build" || command == "compile") {
        if (argc < 3) {
            std::cerr << "Usage: hammer build <file.hmscp> [-o output.exe] [--module <hamhook|hamc>] [--no-console] [--icon app.ico]\n";
            return 1;
        }

        std::string moduleName = "hamhook";
        std::string input = "";
        ModuleOptions options;

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if ((arg == "--module" || arg == "-m") && i + 1 < argc) {
                moduleName = argv[++i];
            } else if (arg == "-o" && i + 1 < argc) {
                options.outputPath = argv[++i];
            } else if (arg == "--backend" && i + 1 < argc) {
                options.extraParams["backend"] = argv[++i];
            } else if (arg == "--keep-cpp") {
                options.keepIntermediate = true;
            } else if (arg == "--no-opt") {
                options.optimize = false;
            } else if (arg == "--no-console" || arg == "-w") {
                options.extraParams["no_console"] = "true";
            } else if (arg == "--icon" && i + 1 < argc) {
                options.extraParams["icon"] = argv[++i];
            } else if (input.empty()) {
                input = arg;
            }
        }

        if (input.empty()) {
            std::cerr << "Error: No input file specified.\n";
            return 1;
        }

        return buildWithModule(moduleName, input, options) ? 0 : 1;
    }

    if (command == "run") {
        if (argc < 3) {
            std::cerr << "Usage: hammer run <file.hmscp>\n";
            return 1;
        }
        return runScript(argv[2]) ? 0 : 1;
    }

    if (fs::exists(command)) {
        return runScript(command) ? 0 : 1;
    }

    std::cerr << "Unknown command: " << command << "\n";
    printHelp();
    return 1;
}
