#ifndef HAMMER_AST_HPP
#define HAMMER_AST_HPP

#include "token.hpp"
#include "value.hpp"
#include <string>
#include <vector>
#include <memory>

namespace Hammer {

// Forward declarations of AST visitors
struct ExprVisitor;
struct StmtVisitor;

// Base Expression
struct Expr {
    virtual ~Expr() = default;
    int line = 1;
};

// Expressions
struct LiteralExpr : public Expr {
    Value value;
    LiteralExpr(Value val, int line = 1) : value(std::move(val)) { this->line = line; }
};

struct IdentifierExpr : public Expr {
    std::string name;
    IdentifierExpr(std::string name, int line = 1) : name(std::move(name)) { this->line = line; }
};

struct BinaryExpr : public Expr {
    std::shared_ptr<Expr> left;
    TokenType op;
    std::string opText;
    std::shared_ptr<Expr> right;

    BinaryExpr(std::shared_ptr<Expr> l, TokenType op, std::string opText, std::shared_ptr<Expr> r, int line = 1)
        : left(std::move(l)), op(op), opText(std::move(opText)), right(std::move(r)) { this->line = line; }
};

struct UnaryExpr : public Expr {
    TokenType op;
    std::shared_ptr<Expr> right;

    UnaryExpr(TokenType op, std::shared_ptr<Expr> r, int line = 1)
        : op(op), right(std::move(r)) { this->line = line; }
};

struct CallExpr : public Expr {
    std::shared_ptr<Expr> callee;
    std::vector<std::shared_ptr<Expr>> args;

    CallExpr(std::shared_ptr<Expr> callee, std::vector<std::shared_ptr<Expr>> args, int line = 1)
        : callee(std::move(callee)), args(std::move(args)) { this->line = line; }
};

struct ListExpr : public Expr {
    std::vector<std::shared_ptr<Expr>> elements;

    ListExpr(std::vector<std::shared_ptr<Expr>> elements, int line = 1)
        : elements(std::move(elements)) { this->line = line; }
};

struct MapExpr : public Expr {
    std::vector<std::pair<std::string, std::shared_ptr<Expr>>> entries;

    MapExpr(std::vector<std::pair<std::string, std::shared_ptr<Expr>>> entries, int line = 1)
        : entries(std::move(entries)) { this->line = line; }
};

struct IndexExpr : public Expr {
    std::shared_ptr<Expr> target;
    std::shared_ptr<Expr> index;
    std::shared_ptr<Expr> sliceEnd;

    IndexExpr(std::shared_ptr<Expr> target, std::shared_ptr<Expr> index, std::shared_ptr<Expr> sliceEnd = nullptr, int line = 1)
        : target(std::move(target)), index(std::move(index)), sliceEnd(std::move(sliceEnd)) { this->line = line; }
};

struct AskExpr : public Expr {
    std::shared_ptr<Expr> prompt;

    AskExpr(std::shared_ptr<Expr> prompt, int line = 1)
        : prompt(std::move(prompt)) { this->line = line; }
};

// Base Statement
struct Stmt {
    virtual ~Stmt() = default;
    int line = 1;
};

// Statements
struct ShowStmt : public Stmt {
    std::shared_ptr<Expr> expr;
    bool newline = true;

    ShowStmt(std::shared_ptr<Expr> expr, bool newline = true, int line = 1)
        : expr(std::move(expr)), newline(newline) { this->line = line; }
};

struct SetStmt : public Stmt {
    std::string name;
    std::shared_ptr<Expr> index; // if setting list[idx] or map[key]
    std::shared_ptr<Expr> value;

    SetStmt(std::string name, std::shared_ptr<Expr> value, std::shared_ptr<Expr> index = nullptr, int line = 1)
        : name(std::move(name)), index(std::move(index)), value(std::move(value)) { this->line = line; }
};

struct IfBranch {
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> body;
};

struct IfStmt : public Stmt {
    std::vector<IfBranch> branches;
    std::vector<std::shared_ptr<Stmt>> elseBody;

    IfStmt(std::vector<IfBranch> branches, std::vector<std::shared_ptr<Stmt>> elseBody = {}, int line = 1)
        : branches(std::move(branches)), elseBody(std::move(elseBody)) { this->line = line; }
};

struct RepeatStmt : public Stmt {
    std::shared_ptr<Expr> count;
    std::vector<std::shared_ptr<Stmt>> body;

    RepeatStmt(std::shared_ptr<Expr> count, std::vector<std::shared_ptr<Stmt>> body, int line = 1)
        : count(std::move(count)), body(std::move(body)) { this->line = line; }
};

struct WhileStmt : public Stmt {
    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> body;

    WhileStmt(std::shared_ptr<Expr> condition, std::vector<std::shared_ptr<Stmt>> body, int line = 1)
        : condition(std::move(condition)), body(std::move(body)) { this->line = line; }
};

struct ForEachStmt : public Stmt {
    std::string itemName;
    std::shared_ptr<Expr> collection;
    std::vector<std::shared_ptr<Stmt>> body;

    ForEachStmt(std::string itemName, std::shared_ptr<Expr> collection, std::vector<std::shared_ptr<Stmt>> body, int line = 1)
        : itemName(std::move(itemName)), collection(std::move(collection)), body(std::move(body)) { this->line = line; }
};

struct ActionStmt : public Stmt {
    std::string name;
    std::vector<std::string> params;
    std::vector<std::shared_ptr<Stmt>> body;

    ActionStmt(std::string name, std::vector<std::string> params, std::vector<std::shared_ptr<Stmt>> body, int line = 1)
        : name(std::move(name)), params(std::move(params)), body(std::move(body)) { this->line = line; }
};

struct GiveStmt : public Stmt {
    std::shared_ptr<Expr> value;

    GiveStmt(std::shared_ptr<Expr> value, int line = 1)
        : value(std::move(value)) { this->line = line; }
};

struct InterpolatedStringExpr : public Expr {
    std::vector<std::shared_ptr<Expr>> parts;
    InterpolatedStringExpr(std::vector<std::shared_ptr<Expr>> parts, int line = 1)
        : parts(std::move(parts)) { this->line = line; }
};

struct IncludeStmt : public Stmt {
    std::string path;
    IncludeStmt(std::string path, int line = 1) : path(std::move(path)) { this->line = line; }
};

struct AttemptStmt : public Stmt {
    std::vector<std::shared_ptr<Stmt>> tryBody;
    std::string errorVar;
    std::vector<std::shared_ptr<Stmt>> catchBody;

    AttemptStmt(std::vector<std::shared_ptr<Stmt>> tryBody, std::string errorVar, std::vector<std::shared_ptr<Stmt>> catchBody, int line = 1)
        : tryBody(std::move(tryBody)), errorVar(std::move(errorVar)), catchBody(std::move(catchBody)) { this->line = line; }
};

struct RepeatWithStmt : public Stmt {
    std::string varName;
    std::shared_ptr<Expr> start;
    std::shared_ptr<Expr> end;
    std::shared_ptr<Expr> step;
    std::vector<std::shared_ptr<Stmt>> body;

    RepeatWithStmt(std::string varName, std::shared_ptr<Expr> start, std::shared_ptr<Expr> end, std::shared_ptr<Expr> step, std::vector<std::shared_ptr<Stmt>> body, int line = 1)
        : varName(std::move(varName)), start(std::move(start)), end(std::move(end)), step(std::move(step)), body(std::move(body)) { this->line = line; }
};

struct StopStmt : public Stmt {
    StopStmt(int line = 1) { this->line = line; }
};

struct SkipStmt : public Stmt {
    SkipStmt(int line = 1) { this->line = line; }
};

struct ExprStmt : public Stmt {
    std::shared_ptr<Expr> expr;

    ExprStmt(std::shared_ptr<Expr> expr, int line = 1)
        : expr(std::move(expr)) { this->line = line; }
};

} // namespace Hammer

#endif // HAMMER_AST_HPP
