#include "../include/compiler.hpp"
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;

namespace Hammer {

std::string Compiler::indent() const {
    return std::string(indentLevel * 4, ' ');
}

std::string Compiler::findGppCompiler() const {
    const std::vector<std::string> candidates = {
        "C:\\msys64\\ucrt64\\bin\\g++.exe",
        "C:\\msys64\\mingw64\\bin\\g++.exe",
        "C:\\msys64\\clang64\\bin\\clang++.exe",
        "g++"
    };

    for (const auto& path : candidates) {
        if (path == "g++" || fs::exists(path)) {
            return path;
        }
    }
    return "g++";
}

std::string Compiler::compileExpr(const std::shared_ptr<Expr>& expr) {
    if (!expr) return "HammerRuntime::Var()";

    if (auto lit = dynamic_cast<LiteralExpr*>(expr.get())) {
        switch (lit->value.type) {
            case ValueType::VAL_NOTHING: return "HammerRuntime::Var()";
            case ValueType::VAL_BOOLEAN: return lit->value.boolVal ? "HammerRuntime::Var(true)" : "HammerRuntime::Var(false)";
            case ValueType::VAL_NUMBER: return "HammerRuntime::Var(" + std::to_string(lit->value.numberVal) + ")";
            case ValueType::VAL_STRING: {
                std::string escaped;
                for (char c : lit->value.stringVal) {
                    if (c == '"') escaped += "\\\"";
                    else if (c == '\\') escaped += "\\\\";
                    else if (c == '\n') escaped += "\\n";
                    else if (c == '\t') escaped += "\\t";
                    else escaped += c;
                }
                return "HammerRuntime::Var(\"" + escaped + "\")";
            }
            default: return "HammerRuntime::Var()";
        }
    }

    if (auto id = dynamic_cast<IdentifierExpr*>(expr.get())) {
        return "var_" + id->name;
    }

    if (auto bin = dynamic_cast<BinaryExpr*>(expr.get())) {
        std::string l = compileExpr(bin->left);
        std::string r = compileExpr(bin->right);
        switch (bin->op) {
            case TokenType::TOKEN_PLUS: return "(" + l + " + " + r + ")";
            case TokenType::TOKEN_MINUS: return "(" + l + " - " + r + ")";
            case TokenType::TOKEN_STAR: return "(" + l + " * " + r + ")";
            case TokenType::TOKEN_SLASH: return "(" + l + " / " + r + ")";
            case TokenType::TOKEN_PERCENT: return "(" + l + " % " + r + ")";
            case TokenType::TOKEN_CARET: return "(" + l + ".power(" + r + "))";
            case TokenType::TOKEN_EQUAL: return "(" + l + " == " + r + ")";
            case TokenType::TOKEN_NOT_EQUAL: return "(" + l + " != " + r + ")";
            case TokenType::TOKEN_LT: return "(" + l + " < " + r + ")";
            case TokenType::TOKEN_GT: return "(" + l + " > " + r + ")";
            case TokenType::TOKEN_LTE: return "(" + l + " <= " + r + ")";
            case TokenType::TOKEN_GTE: return "(" + l + " >= " + r + ")";
            case TokenType::TOKEN_AND: return "(" + l + ".isTruthy() ? " + r + " : HammerRuntime::Var(false))";
            case TokenType::TOKEN_OR: return "(" + l + ".isTruthy() ? " + l + " : " + r + ")";
            default: return "(" + l + " + " + r + ")";
        }
    }

    if (auto un = dynamic_cast<UnaryExpr*>(expr.get())) {
        std::string r = compileExpr(un->right);
        if (un->op == TokenType::TOKEN_MINUS) return "(-" + r + ")";
        if (un->op == TokenType::TOKEN_NOT) return "(!" + r + ")";
        return r;
    }

    if (auto call = dynamic_cast<CallExpr*>(expr.get())) {
        std::string fnName;
        if (auto id = dynamic_cast<IdentifierExpr*>(call->callee.get())) {
            fnName = id->name;
        } else if (auto idx = dynamic_cast<IndexExpr*>(call->callee.get())) {
            if (auto targetId = dynamic_cast<IdentifierExpr*>(idx->target.get())) {
                if (auto lit = dynamic_cast<LiteralExpr*>(idx->index.get())) {
                    std::string key = lit->value.stringVal;
                    if (targetId->name == "hamsys") fnName = "hamsys_" + key;
                    else if (targetId->name == "hamfs") fnName = "hamfs_" + key;
                    else if (targetId->name == "asock") fnName = "asock_" + key;
                    else if (targetId->name == "hmhookwww") fnName = "hmhookwww_" + key;
                    else if (targetId->name == "hmripa") fnName = "hmripa_" + key;
                }
            }
        }

        std::string argsList;
        for (size_t i = 0; i < call->args.size(); ++i) {
            if (i > 0) argsList += ", ";
            argsList += compileExpr(call->args[i]);
        }

        // Check if built-in standard library function
        if (fnName == "length" || fnName == "len") return "HammerRuntime::length(" + argsList + ")";
        if (fnName == "push" || fnName == "append") return "HammerRuntime::push(" + argsList + ")";
        if (fnName == "pop") return "HammerRuntime::pop(" + argsList + ")";
        if (fnName == "random") return "HammerRuntime::random(" + argsList + ")";
        if (fnName == "round") return "HammerRuntime::round(" + argsList + ")";
        if (fnName == "sqrt") return "HammerRuntime::sqrt(" + argsList + ")";
        if (fnName == "abs") return "HammerRuntime::abs(" + argsList + ")";
        if (fnName == "to_number") return "HammerRuntime::to_number(" + argsList + ")";
        if (fnName == "to_string") return "HammerRuntime::to_string(" + argsList + ")";
        if (fnName == "read_file") return "HammerRuntime::read_file(" + argsList + ")";
        if (fnName == "write_file") return "HammerRuntime::write_file(" + argsList + ")";
        if (fnName == "file_exists") return "HammerRuntime::file_exists(" + argsList + ")";
        if (fnName == "sleep") return "HammerRuntime::sleep(" + argsList + ")";
        if (fnName == "time") return "HammerRuntime::time()";

        // hamfs runtime filesystem functions
        if (fnName == "hamfs_read") return "HammerRuntime::hamfs_read(" + argsList + ")";
        if (fnName == "hamfs_write") return "HammerRuntime::hamfs_write(" + argsList + ")";
        if (fnName == "hamfs_append") return "HammerRuntime::hamfs_append(" + argsList + ")";
        if (fnName == "hamfs_delete") return "HammerRuntime::hamfs_delete(" + argsList + ")";
        if (fnName == "hamfs_make_dir") return "HammerRuntime::hamfs_make_dir(" + argsList + ")";
        if (fnName == "hamfs_list") return "HammerRuntime::hamfs_list(" + argsList + ")";
        if (fnName == "hamfs_exists") return "HammerRuntime::hamfs_exists(" + argsList + ")";
        if (fnName == "hamfs_is_folder") return "HammerRuntime::hamfs_is_folder(" + argsList + ")";
        if (fnName == "hamfs_is_file") return "HammerRuntime::hamfs_is_file(" + argsList + ")";
        if (fnName == "hamfs_size") return "HammerRuntime::hamfs_size(" + argsList + ")";
        if (fnName == "hamfs_copy") return "HammerRuntime::hamfs_copy(" + argsList + ")";
        if (fnName == "hamfs_move") return "HammerRuntime::hamfs_move(" + argsList + ")";
        if (fnName == "hamfs_cwd") return "HammerRuntime::hamfs_cwd()";
        if (fnName == "hamfs_exec") return "HammerRuntime::hamfs_exec(" + argsList + ")";

        // asock & hmhookwww internet functions
        if (fnName == "asock_get") return "HammerRuntime::asock_get(" + argsList + ")";
        if (fnName == "asock_post") return "HammerRuntime::asock_post(" + argsList + ")";
        if (fnName == "asock_download") return "HammerRuntime::asock_download(" + argsList + ")";
        if (fnName == "hmhookwww_get") return "HammerRuntime::hmhookwww_get(" + argsList + ")";
        if (fnName == "hmhookwww_post") return "HammerRuntime::hmhookwww_post(" + argsList + ")";
        if (fnName == "hmhookwww_download") return "HammerRuntime::hmhookwww_download(" + argsList + ")";
        if (fnName == "fetch") return "HammerRuntime::fetch(" + argsList + ")";

        // hmripa primitive file & data encryptor/decryptor
        if (fnName == "hmripa_encrypt") return "HammerRuntime::hmripa_encrypt(" + argsList + ")";
        if (fnName == "hmripa_decrypt") return "HammerRuntime::hmripa_decrypt(" + argsList + ")";
        if (fnName == "hmripa_encrypt_file") return "HammerRuntime::hmripa_encrypt_file(" + argsList + ")";
        if (fnName == "hmripa_decrypt_file") return "HammerRuntime::hmripa_decrypt_file(" + argsList + ")";
        if (fnName == "hmripa_hash") return "HammerRuntime::hmripa_hash(" + argsList + ")";

        // stdlib strings, lists, maps, json
        if (fnName == "split") return "HammerRuntime::split(" + argsList + ")";
        if (fnName == "join") return "HammerRuntime::join(" + argsList + ")";
        if (fnName == "replace") return "HammerRuntime::replace(" + argsList + ")";
        if (fnName == "to_upper" || fnName == "uppercase") return "HammerRuntime::to_upper(" + argsList + ")";
        if (fnName == "to_lower" || fnName == "lowercase") return "HammerRuntime::to_lower(" + argsList + ")";
        if (fnName == "trim") return "HammerRuntime::trim(" + argsList + ")";
        if (fnName == "contains") return "HammerRuntime::contains(" + argsList + ")";
        if (fnName == "starts_with") return "HammerRuntime::starts_with(" + argsList + ")";
        if (fnName == "ends_with") return "HammerRuntime::ends_with(" + argsList + ")";
        if (fnName == "insert") return "HammerRuntime::insert(" + argsList + ")";
        if (fnName == "remove_at") return "HammerRuntime::remove_at(" + argsList + ")";
        if (fnName == "sort") return "HammerRuntime::sort(" + argsList + ")";
        if (fnName == "reverse") return "HammerRuntime::reverse(" + argsList + ")";
        if (fnName == "keys") return "HammerRuntime::keys(" + argsList + ")";
        if (fnName == "values") return "HammerRuntime::values(" + argsList + ")";
        if (fnName == "has_key") return "HammerRuntime::has_key(" + argsList + ")";
        if (fnName == "json_parse") return "HammerRuntime::json_parse(" + argsList + ")";
        if (fnName == "json_encode") return "HammerRuntime::json_encode(" + argsList + ")";

        // hamsys system, time, audio & platform utilities
        if (fnName == "hamsys_now") return "HammerRuntime::hamsys_now()";
        if (fnName == "hamsys_date") return "HammerRuntime::hamsys_date()";
        if (fnName == "hamsys_time_str") return "HammerRuntime::hamsys_time_str()";
        if (fnName == "hamsys_datetime") return "HammerRuntime::hamsys_datetime()";
        if (fnName == "hamsys_uptime") return "HammerRuntime::hamsys_uptime()";
        if (fnName == "hamsys_beep") return "HammerRuntime::hamsys_beep(" + argsList + ")";
        if (fnName == "hamsys_play_sound") return "HammerRuntime::hamsys_play_sound(" + argsList + ")";
        if (fnName == "hamsys_system_sound") return "HammerRuntime::hamsys_system_sound(" + argsList + ")";
        if (fnName == "hamsys_os") return "HammerRuntime::hamsys_os()";
        if (fnName == "hamsys_cpu_count") return "HammerRuntime::hamsys_cpu_count()";
        if (fnName == "hamsys_memory") return "HammerRuntime::hamsys_memory()";
        if (fnName == "hamsys_user") return "HammerRuntime::hamsys_user()";
        if (fnName == "hamsys_hostname") return "HammerRuntime::hamsys_hostname()";
        if (fnName == "hamsys_env") return "HammerRuntime::hamsys_env(" + argsList + ")";
        if (fnName == "hamsys_clipboard_set") return "HammerRuntime::hamsys_clipboard_set(" + argsList + ")";
        if (fnName == "hamsys_clipboard_get") return "HammerRuntime::hamsys_clipboard_get()";
        if (fnName == "hamsys_alert") return "HammerRuntime::hamsys_alert(" + argsList + ")";

        return "fn_" + fnName + "(" + argsList + ")";
    }

    if (auto idx = dynamic_cast<IndexExpr*>(expr.get())) {
        if (idx->sliceEnd) {
            return compileExpr(idx->target) + ".slice(" + compileExpr(idx->index) + ", " + compileExpr(idx->sliceEnd) + ")";
        }
        return compileExpr(idx->target) + ".get(" + compileExpr(idx->index) + ")";
    }

    if (auto listExpr = dynamic_cast<ListExpr*>(expr.get())) {
        std::string items = "std::vector<HammerRuntime::Var>{";
        for (size_t i = 0; i < listExpr->elements.size(); ++i) {
            if (i > 0) items += ", ";
            items += compileExpr(listExpr->elements[i]);
        }
        items += "}";
        return "HammerRuntime::Var(" + items + ")";
    }

    if (auto mapExpr = dynamic_cast<MapExpr*>(expr.get())) {
        std::string entries = "std::map<std::string, HammerRuntime::Var>{";
        for (size_t i = 0; i < mapExpr->entries.size(); ++i) {
            if (i > 0) entries += ", ";
            entries += "{\"" + mapExpr->entries[i].first + "\", " + compileExpr(mapExpr->entries[i].second) + "}";
        }
        entries += "}";
        return "HammerRuntime::Var(" + entries + ")";
    }

    if (auto askExpr = dynamic_cast<AskExpr*>(expr.get())) {
        std::string p = askExpr->prompt ? compileExpr(askExpr->prompt) : "HammerRuntime::Var(\"\")";
        return "HammerRuntime::ask(" + p + ")";
    }

    if (auto interp = dynamic_cast<InterpolatedStringExpr*>(expr.get())) {
        std::string res = "(HammerRuntime::Var(\"\")";
        for (const auto& part : interp->parts) {
            res += " + " + compileExpr(part);
        }
        res += ")";
        return res;
    }

    return "HammerRuntime::Var()";
}

void Compiler::compileStmt(const std::shared_ptr<Stmt>& stmt, std::stringstream& ss) {
    if (!stmt) return;

    if (auto showStmt = dynamic_cast<ShowStmt*>(stmt.get())) {
        ss << indent() << "HammerRuntime::show(" << compileExpr(showStmt->expr) << ");\n";
        return;
    }

    if (auto setStmt = dynamic_cast<SetStmt*>(stmt.get())) {
        if (setStmt->index) {
            ss << indent() << "var_" << setStmt->name << "[" << compileExpr(setStmt->index) << "] = " << compileExpr(setStmt->value) << ";\n";
        } else {
            ss << indent() << "var_" << setStmt->name << " = " << compileExpr(setStmt->value) << ";\n";
        }
        return;
    }

    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
        for (size_t i = 0; i < ifStmt->branches.size(); ++i) {
            if (i == 0) {
                ss << indent() << "if (" << compileExpr(ifStmt->branches[i].condition) << ".isTruthy()) {\n";
            } else {
                ss << indent() << "} else if (" << compileExpr(ifStmt->branches[i].condition) << ".isTruthy()) {\n";
            }
            indentLevel++;
            for (const auto& s : ifStmt->branches[i].body) compileStmt(s, ss);
            indentLevel--;
        }
        if (!ifStmt->elseBody.empty()) {
            ss << indent() << "} else {\n";
            indentLevel++;
            for (const auto& s : ifStmt->elseBody) compileStmt(s, ss);
            indentLevel--;
        }
        ss << indent() << "}\n";
        return;
    }

    if (auto repStmt = dynamic_cast<RepeatStmt*>(stmt.get())) {
        std::string countVar = "count_" + std::to_string(std::rand() % 100000);
        ss << indent() << "long long " << countVar << " = (long long)" << compileExpr(repStmt->count) << ".num;\n";
        ss << indent() << "for (long long i = 0; i < " << countVar << "; ++i) {\n";
        indentLevel++;
        for (const auto& s : repStmt->body) compileStmt(s, ss);
        indentLevel--;
        ss << indent() << "}\n";
        return;
    }

    if (auto repWith = dynamic_cast<RepeatWithStmt*>(stmt.get())) {
        std::string varName = "var_" + repWith->varName;
        std::string startVar = "start_" + std::to_string(std::rand() % 100000);
        std::string endVar = "end_" + std::to_string(std::rand() % 100000);
        std::string stepVar = "step_" + std::to_string(std::rand() % 100000);

        ss << indent() << "double " << startVar << " = " << compileExpr(repWith->start) << ".num;\n";
        ss << indent() << "double " << endVar << " = " << compileExpr(repWith->end) << ".num;\n";
        if (repWith->step) {
            ss << indent() << "double " << stepVar << " = " << compileExpr(repWith->step) << ".num;\n";
        } else {
            ss << indent() << "double " << stepVar << " = (" << startVar << " > " << endVar << ") ? -1.0 : 1.0;\n";
        }
        ss << indent() << "for (" << varName << " = " << startVar << "; (" << stepVar << " > 0 ? " << varName << ".num <= " << endVar << " : " << varName << ".num >= " << endVar << "); " << varName << ".num += " << stepVar << ") {\n";
        indentLevel++;
        for (const auto& s : repWith->body) compileStmt(s, ss);
        indentLevel--;
        ss << indent() << "}\n";
        return;
    }

    if (auto incStmt = dynamic_cast<IncludeStmt*>(stmt.get())) {
        std::ifstream file(incStmt->path);
        if (file.is_open()) {
            std::stringstream sstr;
            sstr << file.rdbuf();
            Lexer incLexer(sstr.str());
            auto incTokens = incLexer.tokenize();
            Parser incParser(incTokens);
            auto incStmts = incParser.parse();
            for (const auto& s : incStmts) {
                compileStmt(s, ss);
            }
        }
        return;
    }

    if (auto attStmt = dynamic_cast<AttemptStmt*>(stmt.get())) {
        ss << indent() << "try {\n";
        indentLevel++;
        for (const auto& s : attStmt->tryBody) compileStmt(s, ss);
        indentLevel--;
        ss << indent() << "} catch (const std::exception& e) {\n";
        indentLevel++;
        ss << indent() << "var_" << attStmt->errorVar << " = HammerRuntime::Var(e.what());\n";
        for (const auto& s : attStmt->catchBody) compileStmt(s, ss);
        indentLevel--;
        ss << indent() << "}\n";
        return;
    }

    if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt.get())) {
        ss << indent() << "while (" << compileExpr(whileStmt->condition) << ".isTruthy()) {\n";
        indentLevel++;
        for (const auto& s : whileStmt->body) compileStmt(s, ss);
        indentLevel--;
        ss << indent() << "}\n";
        return;
    }

    if (auto forStmt = dynamic_cast<ForEachStmt*>(stmt.get())) {
        std::string collVar = "coll_" + std::to_string(std::rand() % 100000);
        ss << indent() << "HammerRuntime::Var " << collVar << " = " << compileExpr(forStmt->collection) << ";\n";
        ss << indent() << "if (" << collVar << ".type == HammerRuntime::Type::LIST && " << collVar << ".list) {\n";
        indentLevel++;
        ss << indent() << "for (auto& item : *" << collVar << ".list) {\n";
        indentLevel++;
        ss << indent() << "var_" << forStmt->itemName << " = item;\n";
        for (const auto& s : forStmt->body) compileStmt(s, ss);
        indentLevel--;
        ss << indent() << "}\n";
        indentLevel--;
        ss << indent() << "}\n";
        return;
    }

    if (auto actStmt = dynamic_cast<ActionStmt*>(stmt.get())) {
        std::string params;
        for (size_t i = 0; i < actStmt->params.size(); ++i) {
            if (i > 0) params += ", ";
            params += "HammerRuntime::Var var_" + actStmt->params[i];
        }
        ss << indent() << "auto fn_" << actStmt->name << " = [" << "]( " << params << ") -> HammerRuntime::Var {\n";
        indentLevel++;
        for (const auto& s : actStmt->body) compileStmt(s, ss);
        ss << indent() << "return HammerRuntime::Var();\n";
        indentLevel--;
        ss << indent() << "};\n";
        return;
    }

    if (auto giveStmt = dynamic_cast<GiveStmt*>(stmt.get())) {
        ss << indent() << "return " << (giveStmt->value ? compileExpr(giveStmt->value) : "HammerRuntime::Var()") << ";\n";
        return;
    }

    if (dynamic_cast<StopStmt*>(stmt.get())) {
        ss << indent() << "break;\n";
        return;
    }

    if (dynamic_cast<SkipStmt*>(stmt.get())) {
        ss << indent() << "continue;\n";
        return;
    }

    if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt.get())) {
        ss << indent() << compileExpr(exprStmt->expr) << ";\n";
        return;
    }
}

static void scanVars(const std::vector<std::shared_ptr<Stmt>>& stmts, std::unordered_set<std::string>& declaredVars) {
    for (const auto& s : stmts) {
        if (!s) continue;
        if (auto setStmt = dynamic_cast<SetStmt*>(s.get())) {
            declaredVars.insert(setStmt->name);
        } else if (auto forStmt = dynamic_cast<ForEachStmt*>(s.get())) {
            declaredVars.insert(forStmt->itemName);
            scanVars(forStmt->body, declaredVars);
        } else if (auto repWith = dynamic_cast<RepeatWithStmt*>(s.get())) {
            declaredVars.insert(repWith->varName);
            scanVars(repWith->body, declaredVars);
        } else if (auto attStmt = dynamic_cast<AttemptStmt*>(s.get())) {
            declaredVars.insert(attStmt->errorVar);
            scanVars(attStmt->tryBody, declaredVars);
            scanVars(attStmt->catchBody, declaredVars);
        } else if (auto ifStmt = dynamic_cast<IfStmt*>(s.get())) {
            for (const auto& br : ifStmt->branches) scanVars(br.body, declaredVars);
            scanVars(ifStmt->elseBody, declaredVars);
        } else if (auto repStmt = dynamic_cast<RepeatStmt*>(s.get())) {
            scanVars(repStmt->body, declaredVars);
        } else if (auto whileStmt = dynamic_cast<WhileStmt*>(s.get())) {
            scanVars(whileStmt->body, declaredVars);
        }
    }
}

std::string Compiler::generateCpp(const std::vector<std::shared_ptr<Stmt>>& statements) {
    std::stringstream ss;
    ss << "#include \"sdk/hammer_runtime.hpp\"\n\n";

    // Collect all variable identifiers in script to declare them as HammerRuntime::Var
    ss << "// Hammer Script Auto-generated C++ code\n";
    ss << "int main(int argc, char* argv[]) {\n";
    indentLevel = 1;

    // Declare common variable scope
    ss << indent() << "// Variables\n";
    std::unordered_set<std::string> declaredVars;
    scanVars(statements, declaredVars);
    for (const auto& varName : declaredVars) {
        ss << indent() << "HammerRuntime::Var var_" << varName << ";\n";
    }

    ss << "\n" << indent() << "// Execution\n";
    for (const auto& s : statements) {
        compileStmt(s, ss);
    }

    ss << indent() << "return 0;\n";
    ss << "}\n";

    return ss.str();
}

bool Compiler::compileToBinary(const std::string& cppSource, const std::string& outputPath, bool keepCpp) {
    std::string tempCpp = outputPath + ".tmp.cpp";
    {
        std::ofstream out(tempCpp);
        if (!out.is_open()) {
            std::cerr << "Failed to create temporary C++ file: " << tempCpp << "\n";
            return false;
        }
        out << cppSource;
    }

    std::string gpp = findGppCompiler();
    std::string cmd = "set PATH=C:\\msys64\\ucrt64\\bin;%PATH% && \"" + gpp + "\" -std=c++17 -O2 \"" + tempCpp + "\" -o \"" + outputPath + "\" -I .";

    std::cout << "[Hammer Compiler] Invoking MSYS2 toolchain: " << gpp << "\n";
    std::cout << "[Hammer Compiler] Compiling to: " << outputPath << "\n";

    int result = std::system(cmd.c_str());

    if (!keepCpp) {
        std::error_code ec;
        fs::remove(tempCpp, ec);
    }

    if (result == 0) {
        std::cout << "[Hammer Compiler] Compilation succeeded! Executable generated: " << outputPath << "\n";
        return true;
    } else {
        std::cerr << "[Hammer Compiler] Compilation failed with exit code: " << result << "\n";
        return false;
    }
}

} // namespace Hammer
