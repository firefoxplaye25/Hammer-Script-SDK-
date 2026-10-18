#include "../include/interpreter.hpp"
#include "../include/stdlib.hpp"
#include "../include/hamfs.hpp"
#include "../include/asock.hpp"
#include "../include/hmripa.hpp"
#include "../include/hamsys.hpp"
#include "../include/hmshell.hpp"
#include "../include/hmcanvas.hpp"
#include "../include/lib_loader.hpp"
#include "../include/errors.hpp"
#include "../include/lexer.hpp"
#include "../include/parser.hpp"
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace Hammer {

Interpreter::Interpreter(std::shared_ptr<Environment> globalEnv) {
    if (globalEnv) {
        globals = std::move(globalEnv);
    } else {
        globals = std::make_shared<Environment>();
        registerStdLib(globals);
        registerHamFS(globals);
        registerASock(globals);
        registerHMRipa(globals);
        registerHamSys(globals);
        registerHmShell(globals);
        registerHmCanvas(globals);
        LibLoader::registerLibLoader(globals);
        LibLoader::autoLoadInstalledModules(globals);
    }
}

void Interpreter::setDiagnosticEngine(std::shared_ptr<DiagnosticEngine> diag) {
    diagnostics = std::move(diag);
}

void Interpreter::execute(const std::vector<std::shared_ptr<Stmt>>& statements) {
    for (const auto& stmt : statements) {
        executeStatement(stmt, globals);
    }
}

void Interpreter::executeStatement(const std::shared_ptr<Stmt>& stmt, std::shared_ptr<Environment> env) {
    if (!stmt) return;

    if (auto showStmt = dynamic_cast<ShowStmt*>(stmt.get())) {
        Value val = evaluate(showStmt->expr, env);
        std::cout << val.toString();
        if (showStmt->newline) {
            std::cout << "\n";
        }
        return;
    }

    if (auto setStmt = dynamic_cast<SetStmt*>(stmt.get())) {
        Value val = evaluate(setStmt->value, env);
        if (setStmt->index) {
            // Setting list[idx] or map[key]
            Value target = env->get(setStmt->name, setStmt->line, 1);
            Value idxVal = evaluate(setStmt->index, env);

            if (target.type == ValueType::VAL_LIST && target.listVal) {
                int idx = static_cast<int>(idxVal.numberVal);
                if (idx < 0 || idx >= static_cast<int>(target.listVal->size())) {
                    throw HammerError(
                        setStmt->line, 1, "Index Error",
                        "List index " + std::to_string(idx) + " is out of bounds.",
                        "The list currently has " + std::to_string(target.listVal->size()) + " items (indices 0 to " + std::to_string(target.listVal->size() - 1) + ").",
                        "Ensure your index is within the valid range, or use push() to add new items.",
                        "set my_list[0] to \"first\"\npush(my_list, \"new_item\")"
                    );
                }
                (*target.listVal)[idx] = val;
            } else if (target.type == ValueType::VAL_MAP && target.mapVal) {
                std::string key = idxVal.toString();
                (*target.mapVal)[key] = val;
            } else {
                throw HammerError(
                    setStmt->line, 1, "Type Error",
                    "Cannot assign indexed element to '" + setStmt->name + "' because it is a " + target.typeName() + ".",
                    "Only lists and maps can have their elements updated using index brackets [].",
                    "Make sure '" + setStmt->name + "' is a list or map before indexing.",
                    "set my_list to [1, 2, 3]\nset my_list[0] to 99"
                );
            }
        } else {
            env->assign(setStmt->name, val, setStmt->line, 1);
        }
        return;
    }

    if (auto ifStmt = dynamic_cast<IfStmt*>(stmt.get())) {
        bool branchTaken = false;
        for (const auto& branch : ifStmt->branches) {
            Value cond = evaluate(branch.condition, env);
            if (cond.isTruthy()) {
                auto blockEnv = std::make_shared<Environment>(env);
                for (const auto& s : branch.body) {
                    executeStatement(s, blockEnv);
                }
                branchTaken = true;
                break;
            }
        }
        if (!branchTaken && !ifStmt->elseBody.empty()) {
            auto blockEnv = std::make_shared<Environment>(env);
            for (const auto& s : ifStmt->elseBody) {
                executeStatement(s, blockEnv);
            }
        }
        return;
    }

    if (auto repeatStmt = dynamic_cast<RepeatStmt*>(stmt.get())) {
        Value countVal = evaluate(repeatStmt->count, env);
        long long count = static_cast<long long>(countVal.numberVal);
        for (long long i = 0; i < count; ++i) {
            auto loopEnv = std::make_shared<Environment>(env);
            try {
                for (const auto& s : repeatStmt->body) {
                    executeStatement(s, loopEnv);
                }
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
        return;
    }

    if (auto repWith = dynamic_cast<RepeatWithStmt*>(stmt.get())) {
        Value startVal = evaluate(repWith->start, env);
        Value endVal = evaluate(repWith->end, env);
        double step = 1.0;
        if (repWith->step) {
            step = evaluate(repWith->step, env).numberVal;
            if (step == 0.0) step = 1.0;
        } else if (startVal.numberVal > endVal.numberVal) {
            step = -1.0;
        }

        double cur = startVal.numberVal;
        double limit = endVal.numberVal;

        auto shouldContinue = [&](double c) {
            return step > 0 ? c <= limit : c >= limit;
        };

        while (shouldContinue(cur)) {
            auto loopEnv = std::make_shared<Environment>(env);
            loopEnv->set(repWith->varName, Value::makeNumber(cur));
            try {
                for (const auto& s : repWith->body) {
                    executeStatement(s, loopEnv);
                }
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                cur += step;
                continue;
            }
            cur += step;
        }
        return;
    }

    if (auto incStmt = dynamic_cast<IncludeStmt*>(stmt.get())) {
        std::string path = incStmt->path;
        if (!fs::exists(path)) {
            std::string resolved = LibLoader::resolveLibPath(path);
            if (!resolved.empty()) {
                path = resolved;
            }
        }
        if (includedFiles.find(path) != includedFiles.end()) {
            return; // already included
        }
        includedFiles.insert(path);

        std::ifstream file(path);
        if (!file.is_open()) {
            throw HammerError(
                incStmt->line, 1, "Include Error",
                "Cannot find included file '" + path + "'.",
                "The file path does not exist or cannot be accessed.",
                "Verify the file path and ensure the file exists.",
                "include \"helpers.hmscp\""
            );
        }
        std::stringstream ss;
        ss << file.rdbuf();
        std::string incSource = ss.str();

        Lexer incLexer(incSource);
        auto incTokens = incLexer.tokenize();
        Parser incParser(incTokens);
        auto incStmts = incParser.parse();

        for (const auto& s : incStmts) {
            executeStatement(s, env);
        }
        return;
    }

    if (auto attStmt = dynamic_cast<AttemptStmt*>(stmt.get())) {
        auto tryEnv = std::make_shared<Environment>(env);
        try {
            for (const auto& s : attStmt->tryBody) {
                executeStatement(s, tryEnv);
            }
        } catch (const HammerError& err) {
            auto catchEnv = std::make_shared<Environment>(env);
            catchEnv->set(attStmt->errorVar, Value::makeString(err.message));
            for (const auto& s : attStmt->catchBody) {
                executeStatement(s, catchEnv);
            }
        } catch (const std::exception& e) {
            auto catchEnv = std::make_shared<Environment>(env);
            catchEnv->set(attStmt->errorVar, Value::makeString(e.what()));
            for (const auto& s : attStmt->catchBody) {
                executeStatement(s, catchEnv);
            }
        }
        return;
    }

    if (auto whileStmt = dynamic_cast<WhileStmt*>(stmt.get())) {
        while (evaluate(whileStmt->condition, env).isTruthy()) {
            auto loopEnv = std::make_shared<Environment>(env);
            try {
                for (const auto& s : whileStmt->body) {
                    executeStatement(s, loopEnv);
                }
            } catch (const BreakException&) {
                break;
            } catch (const ContinueException&) {
                continue;
            }
        }
        return;
    }

    if (auto forStmt = dynamic_cast<ForEachStmt*>(stmt.get())) {
        Value coll = evaluate(forStmt->collection, env);
        if (coll.type == ValueType::VAL_LIST && coll.listVal) {
            for (const auto& item : *coll.listVal) {
                auto loopEnv = std::make_shared<Environment>(env);
                loopEnv->set(forStmt->itemName, item);
                try {
                    for (const auto& s : forStmt->body) {
                        executeStatement(s, loopEnv);
                    }
                } catch (const BreakException&) {
                    break;
                } catch (const ContinueException&) {
                    continue;
                }
            }
        } else if (coll.type == ValueType::VAL_STRING) {
            for (char ch : coll.stringVal) {
                auto loopEnv = std::make_shared<Environment>(env);
                loopEnv->set(forStmt->itemName, Value::makeString(std::string(1, ch)));
                try {
                    for (const auto& s : forStmt->body) {
                        executeStatement(s, loopEnv);
                    }
                } catch (const BreakException&) {
                    break;
                } catch (const ContinueException&) {
                    continue;
                }
            }
        } else {
            throw HammerError(
                forStmt->line, 1, "Type Error",
                "Cannot iterate over a " + coll.typeName() + " with a for loop.",
                "For loops can only iterate through lists or strings.",
                "Provide a list or string to loop over.",
                "for each x in [1, 2, 3]\n    show x\nend"
            );
        }
        return;
    }

    if (auto actionStmt = dynamic_cast<ActionStmt*>(stmt.get())) {
        Value fn = Value::makeFunction(actionStmt->name, actionStmt->params, actionStmt->body, env);
        env->set(actionStmt->name, fn);
        return;
    }

    if (auto giveStmt = dynamic_cast<GiveStmt*>(stmt.get())) {
        Value res = giveStmt->value ? evaluate(giveStmt->value, env) : Value::makeNothing();
        throw ReturnException(res);
    }

    if (dynamic_cast<StopStmt*>(stmt.get())) {
        throw BreakException();
    }

    if (dynamic_cast<SkipStmt*>(stmt.get())) {
        throw ContinueException();
    }

    if (auto exprStmt = dynamic_cast<ExprStmt*>(stmt.get())) {
        evaluate(exprStmt->expr, env);
        return;
    }
}

Value Interpreter::evaluate(const std::shared_ptr<Expr>& expr, std::shared_ptr<Environment> env) {
    if (!expr) return Value::makeNothing();

    if (auto lit = dynamic_cast<LiteralExpr*>(expr.get())) {
        return lit->value;
    }

    if (auto id = dynamic_cast<IdentifierExpr*>(expr.get())) {
        return env->get(id->name, id->line, 1);
    }

    if (auto bin = dynamic_cast<BinaryExpr*>(expr.get())) {
        return evaluateBinary(*bin, env);
    }

    if (auto un = dynamic_cast<UnaryExpr*>(expr.get())) {
        return evaluateUnary(*un, env);
    }

    if (auto call = dynamic_cast<CallExpr*>(expr.get())) {
        return evaluateCall(*call, env);
    }

    if (auto idx = dynamic_cast<IndexExpr*>(expr.get())) {
        return evaluateIndex(*idx, env);
    }

    if (auto listExpr = dynamic_cast<ListExpr*>(expr.get())) {
        std::vector<Value> items;
        for (const auto& el : listExpr->elements) {
            items.push_back(evaluate(el, env));
        }
        return Value::makeList(items);
    }

    if (auto mapExpr = dynamic_cast<MapExpr*>(expr.get())) {
        Value m = Value::makeMap();
        for (const auto& entry : mapExpr->entries) {
            (*m.mapVal)[entry.first] = evaluate(entry.second, env);
        }
        return m;
    }

    if (auto interp = dynamic_cast<InterpolatedStringExpr*>(expr.get())) {
        std::string res;
        for (const auto& part : interp->parts) {
            Value v = evaluate(part, env);
            res += v.toString();
        }
        return Value::makeString(res);
    }

    if (auto askExpr = dynamic_cast<AskExpr*>(expr.get())) {
        if (askExpr->prompt) {
            Value promptVal = evaluate(askExpr->prompt, env);
            std::cout << promptVal.toString();
            std::cout.flush();
        }
        std::string input;
        if (std::getline(std::cin, input)) {
            if (!input.empty() && input.back() == '\r') {
                input.pop_back();
            }
            return Value::makeString(input);
        }
        return Value::makeString("");
    }

    return Value::makeNothing();
}

Value Interpreter::evaluateBinary(const BinaryExpr& expr, std::shared_ptr<Environment> env) {
    // Short-circuiting for logical operators
    if (expr.op == TokenType::TOKEN_OR) {
        Value l = evaluate(expr.left, env);
        if (l.isTruthy()) return Value::makeBoolean(true);
        Value r = evaluate(expr.right, env);
        return Value::makeBoolean(r.isTruthy());
    }

    if (expr.op == TokenType::TOKEN_AND) {
        Value l = evaluate(expr.left, env);
        if (!l.isTruthy()) return Value::makeBoolean(false);
        Value r = evaluate(expr.right, env);
        return Value::makeBoolean(r.isTruthy());
    }

    Value left = evaluate(expr.left, env);
    Value right = evaluate(expr.right, env);

    // Equality
    if (expr.op == TokenType::TOKEN_EQUAL) {
        return Value::makeBoolean(left == right);
    }
    if (expr.op == TokenType::TOKEN_NOT_EQUAL) {
        return Value::makeBoolean(left != right);
    }

    // Addition & Concatenation
    if (expr.op == TokenType::TOKEN_PLUS) {
        // Numbers
        if (left.type == ValueType::VAL_NUMBER && right.type == ValueType::VAL_NUMBER) {
            return Value::makeNumber(left.numberVal + right.numberVal);
        }
        // String concatenation (seamlessly converts numbers/booleans/etc to string!)
        if (left.type == ValueType::VAL_STRING || right.type == ValueType::VAL_STRING) {
            return Value::makeString(left.toString() + right.toString());
        }
        // List concatenation
        if (left.type == ValueType::VAL_LIST && right.type == ValueType::VAL_LIST) {
            std::vector<Value> merged;
            if (left.listVal) merged.insert(merged.end(), left.listVal->begin(), left.listVal->end());
            if (right.listVal) merged.insert(merged.end(), right.listVal->begin(), right.listVal->end());
            return Value::makeList(merged);
        }
        throw HammerError(
            expr.line, 1, "Type Error",
            "Cannot add " + left.typeName() + " and " + right.typeName() + ".",
            "Addition is supported for numbers, strings, and lists.",
            "Make sure both operands are compatible types.",
            "total = 10 + 20\nmsg = \"Score: \" + total"
        );
    }

    // Numerical operations
    if (expr.op == TokenType::TOKEN_MINUS) {
        return Value::makeNumber(left.numberVal - right.numberVal);
    }
    if (expr.op == TokenType::TOKEN_STAR) {
        if (left.type == ValueType::VAL_STRING && right.type == ValueType::VAL_NUMBER) {
            std::string rep = "";
            int count = static_cast<int>(right.numberVal);
            for (int i = 0; i < count; ++i) rep += left.stringVal;
            return Value::makeString(rep);
        }
        return Value::makeNumber(left.numberVal * right.numberVal);
    }
    if (expr.op == TokenType::TOKEN_SLASH) {
        if (right.numberVal == 0.0) {
            throw HammerError(
                expr.line, 1, "Math Error",
                "Division by zero.",
                "You attempted to divide by 0, which is mathematically impossible.",
                "Check the denominator to make sure it's not zero.",
                "if total_items > 0 then\n    average = sum / total_items\nend"
            );
        }
        return Value::makeNumber(left.numberVal / right.numberVal);
    }
    if (expr.op == TokenType::TOKEN_PERCENT) {
        long long l = static_cast<long long>(left.numberVal);
        long long r = static_cast<long long>(right.numberVal);
        if (r == 0) {
            throw HammerError(
                expr.line, 1, "Math Error",
                "Modulo by zero.",
                "Cannot perform remainder division by zero.",
                "Ensure modulo operand is not 0.",
                "is_even = (number % 2) is 0"
            );
        }
        return Value::makeNumber(static_cast<double>(l % r));
    }
    if (expr.op == TokenType::TOKEN_CARET) {
        return Value::makeNumber(std::pow(left.numberVal, right.numberVal));
    }

    // Relative Comparisons
    if (expr.op == TokenType::TOKEN_LT) {
        if (left.type == ValueType::VAL_STRING && right.type == ValueType::VAL_STRING) {
            return Value::makeBoolean(left.stringVal < right.stringVal);
        }
        return Value::makeBoolean(left.numberVal < right.numberVal);
    }
    if (expr.op == TokenType::TOKEN_GT) {
        if (left.type == ValueType::VAL_STRING && right.type == ValueType::VAL_STRING) {
            return Value::makeBoolean(left.stringVal > right.stringVal);
        }
        return Value::makeBoolean(left.numberVal > right.numberVal);
    }
    if (expr.op == TokenType::TOKEN_LTE) {
        if (left.type == ValueType::VAL_STRING && right.type == ValueType::VAL_STRING) {
            return Value::makeBoolean(left.stringVal <= right.stringVal);
        }
        return Value::makeBoolean(left.numberVal <= right.numberVal);
    }
    if (expr.op == TokenType::TOKEN_GTE) {
        if (left.type == ValueType::VAL_STRING && right.type == ValueType::VAL_STRING) {
            return Value::makeBoolean(left.stringVal >= right.stringVal);
        }
        return Value::makeBoolean(left.numberVal >= right.numberVal);
    }

    return Value::makeNothing();
}

Value Interpreter::evaluateUnary(const UnaryExpr& expr, std::shared_ptr<Environment> env) {
    Value val = evaluate(expr.right, env);
    if (expr.op == TokenType::TOKEN_MINUS) {
        return Value::makeNumber(-val.numberVal);
    }
    if (expr.op == TokenType::TOKEN_NOT) {
        return Value::makeBoolean(!val.isTruthy());
    }
    return val;
}

Value Interpreter::evaluateCall(const CallExpr& expr, std::shared_ptr<Environment> env) {
    Value callee = evaluate(expr.callee, env);

    std::vector<Value> evaluatedArgs;
    for (const auto& a : expr.args) {
        evaluatedArgs.push_back(evaluate(a, env));
    }

    if (callee.type == ValueType::VAL_NATIVE) {
        return callee.nativeVal(evaluatedArgs);
    }

    if (callee.type == ValueType::VAL_FUNCTION && callee.funcVal) {
        auto func = callee.funcVal;
        if (func->params.size() != evaluatedArgs.size()) {
            throw HammerError(
                expr.line, 1, "Argument Error",
                "Action '" + func->name + "' expects " + std::to_string(func->params.size()) +
                " argument(s), but received " + std::to_string(evaluatedArgs.size()) + ".",
                "The number of arguments provided in the call does not match the action's definition.",
                "Call '" + func->name + "' with exactly " + std::to_string(func->params.size()) + " argument(s).",
                func->name + "(" + (func->params.empty() ? "" : "arg1") + ")"
            );
        }

        auto actionEnv = std::make_shared<Environment>(func->closure);
        for (size_t i = 0; i < func->params.size(); ++i) {
            actionEnv->set(func->params[i], evaluatedArgs[i]);
        }

        try {
            for (const auto& s : func->body) {
                executeStatement(s, actionEnv);
            }
        } catch (const ReturnException& ret) {
            return ret.value;
        }

        return Value::makeNothing();
    }

    throw HammerError(
        expr.line, 1, "Call Error",
        "Cannot call a " + callee.typeName() + " as an action.",
        "You attempted to invoke a value that is not an action or function using parentheses ().",
        "Verify that you are calling a function and not a standard variable.",
        "action greet()\n    show \"Hello!\"\nend\ngreet()"
    );
}

Value Interpreter::evaluateIndex(const IndexExpr& expr, std::shared_ptr<Environment> env) {
    Value target = evaluate(expr.target, env);
    Value idxVal = evaluate(expr.index, env);

    if (expr.sliceEnd) {
        Value endVal = evaluate(expr.sliceEnd, env);
        if (target.type == ValueType::VAL_LIST && target.listVal) {
            int len = static_cast<int>(target.listVal->size());
            int start = static_cast<int>(idxVal.numberVal);
            int end = static_cast<int>(endVal.numberVal);
            if (start < 0) start = std::max(0, len + start);
            if (end < 0) end = std::max(0, len + end);
            start = std::clamp(start, 0, len);
            end = std::clamp(end, 0, len);
            std::vector<Value> sub;
            for (int i = start; i <= end && i < len; ++i) {
                sub.push_back((*target.listVal)[i]);
            }
            return Value::makeList(sub);
        }
        if (target.type == ValueType::VAL_STRING) {
            int len = static_cast<int>(target.stringVal.size());
            int start = static_cast<int>(idxVal.numberVal);
            int end = static_cast<int>(endVal.numberVal);
            if (start < 0) start = std::max(0, len + start);
            if (end < 0) end = std::max(0, len + end);
            start = std::clamp(start, 0, len);
            end = std::clamp(end, 0, len);
            if (start <= end && start < len) {
                int count = std::min(end - start + 1, len - start);
                return Value::makeString(target.stringVal.substr(start, count));
            }
            return Value::makeString("");
        }
    }

    if (target.type == ValueType::VAL_LIST && target.listVal) {
        int idx = static_cast<int>(idxVal.numberVal);
        if (idx < 0 || idx >= static_cast<int>(target.listVal->size())) {
            throw HammerError(
                expr.line, 1, "Index Error",
                "Index " + std::to_string(idx) + " is out of bounds for list of size " + std::to_string(target.listVal->size()) + ".",
                "Hammer Script lists start at index 0 and end at size - 1.",
                "Use an index between 0 and " + std::to_string(target.listVal->size() - 1) + ".",
                "first = my_list[0]"
            );
        }
        return (*target.listVal)[idx];
    }

    if (target.type == ValueType::VAL_MAP && target.mapVal) {
        std::string key = idxVal.toString();
        auto it = target.mapVal->find(key);
        if (it != target.mapVal->end()) {
            return it->second;
        }
        return Value::makeNothing();
    }

    if (target.type == ValueType::VAL_STRING) {
        int idx = static_cast<int>(idxVal.numberVal);
        if (idx < 0 || idx >= static_cast<int>(target.stringVal.size())) {
            throw HammerError(
                expr.line, 1, "Index Error",
                "Character index " + std::to_string(idx) + " is out of bounds for string of length " + std::to_string(target.stringVal.size()) + ".",
                "String indices start at 0.",
                "Use an index within the length of the string.",
                "letter = \"Hammer\"[0]"
            );
        }
        return Value::makeString(std::string(1, target.stringVal[idx]));
    }

    throw HammerError(
        expr.line, 1, "Type Error",
        "Cannot index into a " + target.typeName() + ".",
        "Index access with [] is only supported for lists, maps, and strings.",
        "Make sure the target expression evaluates to a list, map, or string.",
        "my_list = [10, 20]\nshow my_list[0]"
    );
}

} // namespace Hammer
