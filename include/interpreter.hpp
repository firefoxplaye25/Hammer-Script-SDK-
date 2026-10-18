#ifndef HAMMER_INTERPRETER_HPP
#define HAMMER_INTERPRETER_HPP

#include "ast.hpp"
#include "environment.hpp"
#include "value.hpp"
#include "diagnostics.hpp"
#include <memory>
#include <vector>
#include <unordered_set>

namespace Hammer {

class ReturnException {
public:
    Value value;
    explicit ReturnException(Value v) : value(std::move(v)) {}
};

class BreakException {};
class ContinueException {};

class Interpreter {
public:
    explicit Interpreter(std::shared_ptr<Environment> globalEnv = nullptr);

    void setDiagnosticEngine(std::shared_ptr<DiagnosticEngine> diag);
    void execute(const std::vector<std::shared_ptr<Stmt>>& statements);
    Value evaluate(const std::shared_ptr<Expr>& expr, std::shared_ptr<Environment> env);
    void executeStatement(const std::shared_ptr<Stmt>& stmt, std::shared_ptr<Environment> env);

    std::shared_ptr<Environment> getGlobals() const { return globals; }

private:
    std::shared_ptr<Environment> globals;
    std::shared_ptr<DiagnosticEngine> diagnostics;
    std::unordered_set<std::string> includedFiles;

    Value evaluateBinary(const BinaryExpr& expr, std::shared_ptr<Environment> env);
    Value evaluateUnary(const UnaryExpr& expr, std::shared_ptr<Environment> env);
    Value evaluateCall(const CallExpr& expr, std::shared_ptr<Environment> env);
    Value evaluateIndex(const IndexExpr& expr, std::shared_ptr<Environment> env);
};

} // namespace Hammer

#endif // HAMMER_INTERPRETER_HPP
