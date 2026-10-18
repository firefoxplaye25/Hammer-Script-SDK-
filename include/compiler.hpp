#ifndef HAMMER_COMPILER_HPP
#define HAMMER_COMPILER_HPP

#include "ast.hpp"
#include <string>
#include <vector>
#include <memory>

namespace Hammer {

class Compiler {
public:
    Compiler() = default;

    std::string generateCpp(const std::vector<std::shared_ptr<Stmt>>& statements);
    bool compileToBinary(const std::string& cppSource, const std::string& outputPath, bool keepCpp = false);

private:
    int indentLevel = 0;
    std::string indent() const;

    void compileStmt(const std::shared_ptr<Stmt>& stmt, std::stringstream& ss);
    std::string compileExpr(const std::shared_ptr<Expr>& expr);
    std::string findGppCompiler() const;
};

} // namespace Hammer

#endif // HAMMER_COMPILER_HPP
