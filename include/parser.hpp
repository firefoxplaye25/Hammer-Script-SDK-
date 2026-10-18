#ifndef HAMMER_PARSER_HPP
#define HAMMER_PARSER_HPP

#include "token.hpp"
#include "ast.hpp"
#include <vector>
#include <memory>
#include <string>

namespace Hammer {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::vector<std::shared_ptr<Stmt>> parse();
    bool isAtEnd() const;

private:
    std::vector<Token> tokens;
    size_t current = 0;
    const Token& peek() const;
    const Token& previous() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);
    void skipNewlines();

    // Statements
    std::shared_ptr<Stmt> parseStatement();
    std::shared_ptr<Stmt> parseShowStatement();
    std::shared_ptr<Stmt> parseSetStatement();
    std::shared_ptr<Stmt> parseIfStatement();
    std::shared_ptr<Stmt> parseRepeatStatement();
    std::shared_ptr<Stmt> parseWhileStatement();
    std::shared_ptr<Stmt> parseForStatement();
    std::shared_ptr<Stmt> parseActionStatement();
    std::shared_ptr<Stmt> parseGiveStatement();
    std::shared_ptr<Stmt> parseStopStatement();
    std::shared_ptr<Stmt> parseSkipStatement();
    std::shared_ptr<Stmt> parseIncludeStatement();
    std::shared_ptr<Stmt> parseAttemptStatement();
    std::shared_ptr<Stmt> parseExpressionOrAssignStatement();

    std::vector<std::shared_ptr<Stmt>> parseBlockUntil(std::initializer_list<TokenType> endTokens);

    // Expressions
    std::shared_ptr<Expr> parseExpression();
    std::shared_ptr<Expr> parseLogicalOr();
    std::shared_ptr<Expr> parseLogicalAnd();
    std::shared_ptr<Expr> parseComparison();
    std::shared_ptr<Expr> parseAdditive();
    std::shared_ptr<Expr> parseMultiplicative();
    std::shared_ptr<Expr> parsePower();
    std::shared_ptr<Expr> parseUnary();
    std::shared_ptr<Expr> parseCallOrIndex();
    std::shared_ptr<Expr> parsePrimary();
    std::shared_ptr<Expr> parseAskExpression();
};

} // namespace Hammer

#endif // HAMMER_PARSER_HPP
