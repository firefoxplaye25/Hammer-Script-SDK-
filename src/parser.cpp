#include "../include/parser.hpp"
#include "../include/lexer.hpp"
#include "../include/errors.hpp"
#include "../include/diagnostics.hpp"
#include <stdexcept>
#include <algorithm>

namespace Hammer {

static const std::vector<std::string> KNOWN_KEYWORDS = {
    "set", "to", "show", "print", "ask", "if", "then", "else", "elseif", "end",
    "is", "not", "and", "or", "greater", "less", "than", "at", "least", "most",
    "repeat", "times", "while", "for", "each", "in", "action", "function",
    "give", "return", "stop", "break", "skip", "continue", "include", "use",
    "import", "attempt", "try", "on", "error", "with", "from", "step",
    "true", "false", "yes", "no", "nothing", "null"
};

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)) {}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::TOKEN_EOF;
}

const Token& Parser::peek() const {
    if (current >= tokens.size()) return tokens.back();
    return tokens[current];
}

const Token& Parser::previous() const {
    if (current == 0) return tokens[0];
    return tokens[current - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    const Token& t = peek();

    // Check for "did you mean" typo
    std::string hint = "";
    std::string typo = DiagnosticEngine::findClosestMatch(t.text, KNOWN_KEYWORDS);
    if (!typo.empty() && typo != t.text) {
        hint = "Did you mean '" + typo + "'?";
    }

    throw HammerError(
        t.line,
        t.column,
        "Syntax Error",
        message + " (Encountered '" + t.text + "')",
        "Hammer Script expected a specific token here but received '" + t.text + "'.",
        hint.empty() ? "Review the syntax around line " + std::to_string(t.line) : hint,
        ""
    );
}

void Parser::skipNewlines() {
    while (check(TokenType::TOKEN_NEWLINE)) {
        advance();
    }
}

std::vector<std::shared_ptr<Stmt>> Parser::parse() {
    std::vector<std::shared_ptr<Stmt>> statements;
    skipNewlines();

    while (!isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(stmt);
        }
        skipNewlines();
    }

    return statements;
}

std::shared_ptr<Stmt> Parser::parseStatement() {
    skipNewlines();
    if (isAtEnd()) return nullptr;

    if (match({TokenType::TOKEN_SHOW, TokenType::TOKEN_PRINT})) {
        return parseShowStatement();
    }
    if (match(TokenType::TOKEN_SET)) {
        return parseSetStatement();
    }
    if (match(TokenType::TOKEN_IF)) {
        return parseIfStatement();
    }
    if (match(TokenType::TOKEN_REPEAT)) {
        return parseRepeatStatement();
    }
    if (match(TokenType::TOKEN_WHILE)) {
        return parseWhileStatement();
    }
    if (match(TokenType::TOKEN_FOR)) {
        return parseForStatement();
    }
    if (match({TokenType::TOKEN_ACTION, TokenType::TOKEN_FUNCTION})) {
        return parseActionStatement();
    }
    if (match({TokenType::TOKEN_GIVE, TokenType::TOKEN_RETURN})) {
        return parseGiveStatement();
    }
    if (match({TokenType::TOKEN_STOP, TokenType::TOKEN_BREAK})) {
        return parseStopStatement();
    }
    if (match({TokenType::TOKEN_SKIP, TokenType::TOKEN_CONTINUE})) {
        return parseSkipStatement();
    }
    if (match(TokenType::TOKEN_INCLUDE)) {
        return parseIncludeStatement();
    }
    if (match(TokenType::TOKEN_ATTEMPT)) {
        return parseAttemptStatement();
    }

    return parseExpressionOrAssignStatement();
}

std::shared_ptr<Stmt> Parser::parseShowStatement() {
    int line = previous().line;
    int col = previous().column;

    if (check(TokenType::TOKEN_NEWLINE) || isAtEnd()) {
        throw HammerError(
            line,
            col,
            "Syntax Error",
            "Missing expression after 'show' or 'print'.",
            "You wrote 'show' or 'print', but didn't provide anything to display.",
            "Provide the message or variable you want to show.",
            "show \"Hello, World!\"\n# or:\nshow my_variable"
        );
    }

    auto expr = parseExpression();
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<ShowStmt>(expr, true, line);
}

std::shared_ptr<Stmt> Parser::parseSetStatement() {
    int line = previous().line;

    if (!check(TokenType::TOKEN_IDENTIFIER)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Expected variable name after 'set'.",
            "In Hammer Script, 'set' creates or updates a variable. It must be followed by a variable name.",
            "Specify a valid variable name (e.g. 'set name to \"Alice\"').",
            "set score to 100\nset name to \"Alice\""
        );
    }

    Token nameTok = advance();
    std::shared_ptr<Expr> indexExpr = nullptr;

    if (match(TokenType::TOKEN_LBRACKET)) {
        indexExpr = parseExpression();
        if (!match(TokenType::TOKEN_RBRACKET)) {
            throw HammerError(
                peek().line,
                peek().column,
                "Syntax Error",
                "Missing closing bracket ']' after list/map index.",
                "When setting an element of a list or map, you must close the index with ']'.",
                "Add ']' after the index expression.",
                "set my_list[0] to \"first\"\nset user[\"email\"] to \"alice@mail.com\""
            );
        }
    }

    if (!match(TokenType::TOKEN_TO)) {
        // Did they use '=' instead or completely forget?
        if (match(TokenType::TOKEN_ASSIGN)) {
            // User wrote 'set x = 10', help them gently
            auto val = parseExpression();
            match(TokenType::TOKEN_NEWLINE);
            return std::make_shared<SetStmt>(nameTok.text, val, indexExpr, line);
        }

        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Missing 'to' in variable assignment.",
            "You wrote 'set " + nameTok.text + " ...' without the keyword 'to'. Hammer Script uses natural English grammar: 'set <variable> to <value>'.",
            "Insert 'to' between the variable name and the value, or use '=' without 'set'.",
            "set " + nameTok.text + " to 100\n# or simply:\n" + nameTok.text + " = 100"
        );
    }

    auto value = parseExpression();
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<SetStmt>(nameTok.text, value, indexExpr, line);
}

std::shared_ptr<Stmt> Parser::parseIfStatement() {
    int line = previous().line;
    std::vector<IfBranch> branches;

    auto firstCond = parseExpression();
    match(TokenType::TOKEN_THEN);
    match(TokenType::TOKEN_COLON);
    match(TokenType::TOKEN_NEWLINE);

    auto firstBody = parseBlockUntil({TokenType::TOKEN_ELSEIF, TokenType::TOKEN_ELSE, TokenType::TOKEN_END});
    branches.push_back({firstCond, firstBody});

    while (match(TokenType::TOKEN_ELSEIF)) {
        auto cond = parseExpression();
        match(TokenType::TOKEN_THEN);
        match(TokenType::TOKEN_COLON);
        match(TokenType::TOKEN_NEWLINE);
        auto body = parseBlockUntil({TokenType::TOKEN_ELSEIF, TokenType::TOKEN_ELSE, TokenType::TOKEN_END});
        branches.push_back({cond, body});
    }

    std::vector<std::shared_ptr<Stmt>> elseBody;
    if (match(TokenType::TOKEN_ELSE)) {
        match(TokenType::TOKEN_COLON);
        match(TokenType::TOKEN_NEWLINE);
        elseBody = parseBlockUntil({TokenType::TOKEN_END});
    }

    if (!match(TokenType::TOKEN_END)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Missing 'end' to close 'if' block (started on line " + std::to_string(line) + ").",
            "Every conditional block in Hammer Script must be closed with 'end'. This eliminates indentation bugs and makes your code crystal clear!",
            "Add 'end' at the end of the if block.",
            "if score > 50 then\n    show \"You win!\"\nelse\n    show \"Try again!\"\nend"
        );
    }

    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<IfStmt>(branches, elseBody, line);
}

std::shared_ptr<Stmt> Parser::parseRepeatStatement() {
    int line = previous().line;

    if (match(TokenType::TOKEN_WITH)) {
        Token varTok = consume(TokenType::TOKEN_IDENTIFIER, "Expected variable name in 'repeat with <var>'");
        consume(TokenType::TOKEN_FROM, "Expected 'from' in 'repeat with <var> from <start> to <end>'");
        auto startExpr = parseExpression();
        consume(TokenType::TOKEN_TO, "Expected 'to' in 'repeat with <var> from <start> to <end>'");
        auto endExpr = parseExpression();
        std::shared_ptr<Expr> stepExpr = nullptr;
        if (match(TokenType::TOKEN_STEP)) {
            stepExpr = parseExpression();
        }
        match(TokenType::TOKEN_COLON);
        match(TokenType::TOKEN_NEWLINE);

        auto body = parseBlockUntil({TokenType::TOKEN_END});
        consume(TokenType::TOKEN_END, "Expected 'end' to close 'repeat with' loop");
        match(TokenType::TOKEN_NEWLINE);
        return std::make_shared<RepeatWithStmt>(varTok.text, startExpr, endExpr, stepExpr, body, line);
    }

    auto countExpr = parseExpression();
    if (!match(TokenType::TOKEN_TIMES)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Missing keyword 'times' after repeat count.",
            "In Hammer Script, a repeat loop specifies how many times to run using the natural English word 'times'.",
            "Add 'times' right after the number or expression.",
            "repeat 5 times\n    show \"Hammer is easy!\"\nend"
        );
    }
    match(TokenType::TOKEN_NEWLINE);

    auto body = parseBlockUntil({TokenType::TOKEN_END});
    if (!match(TokenType::TOKEN_END)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Missing 'end' to close 'repeat' loop (started on line " + std::to_string(line) + ").",
            "Loops must be terminated with the 'end' keyword.",
            "Add 'end' after the statements in your repeat loop.",
            "repeat 3 times\n    show \"Hello\"\nend"
        );
    }
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<RepeatStmt>(countExpr, body, line);
}

std::shared_ptr<Stmt> Parser::parseWhileStatement() {
    int line = previous().line;
    auto cond = parseExpression();
    match(TokenType::TOKEN_COLON);
    match(TokenType::TOKEN_NEWLINE);

    auto body = parseBlockUntil({TokenType::TOKEN_END});
    if (!match(TokenType::TOKEN_END)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Missing 'end' to close 'while' loop (started on line " + std::to_string(line) + ").",
            "While loops keep running while their condition is true, and must finish with 'end'.",
            "Add 'end' at the end of your while block.",
            "while count > 0\n    show count\n    count = count - 1\nend"
        );
    }
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<WhileStmt>(cond, body, line);
}

std::shared_ptr<Stmt> Parser::parseForStatement() {
    int line = previous().line;
    match(TokenType::TOKEN_EACH); // optional 'each'

    if (!check(TokenType::TOKEN_IDENTIFIER)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Expected variable name in 'for' loop.",
            "A for loop needs a variable name to store each item as it iterates over a list.",
            "Specify the iteration variable name.",
            "for each item in [\"apple\", \"banana\", \"orange\"]\n    show item\nend"
        );
    }

    Token itemTok = advance();
    if (!match(TokenType::TOKEN_IN)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Missing 'in' keyword in 'for' loop.",
            "In Hammer Script, for loops iterate over a collection with 'for each <item> in <collection>'.",
            "Add 'in' followed by the list or collection.",
            "for each num in [1, 2, 3, 4]\n    show num\nend"
        );
    }

    auto collection = parseExpression();
    match(TokenType::TOKEN_COLON);
    match(TokenType::TOKEN_NEWLINE);

    auto body = parseBlockUntil({TokenType::TOKEN_END});
    if (!match(TokenType::TOKEN_END)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Missing 'end' to close 'for' loop (started on line " + std::to_string(line) + ").",
            "For loops must conclude with 'end'.",
            "Add 'end' at the end of your for loop.",
            "for each item in my_list\n    show item\nend"
        );
    }
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<ForEachStmt>(itemTok.text, collection, body, line);
}

std::shared_ptr<Stmt> Parser::parseActionStatement() {
    int line = previous().line;

    if (!check(TokenType::TOKEN_IDENTIFIER)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Expected action name.",
            "When creating a reusable function or action, you must give it a name.",
            "Provide an identifier name for your action.",
            "action greet(name)\n    show \"Hello \" + name\nend"
        );
    }

    Token nameTok = advance();
    std::vector<std::string> params;

    if (match(TokenType::TOKEN_LPAREN)) {
        if (!check(TokenType::TOKEN_RPAREN)) {
            do {
                if (!check(TokenType::TOKEN_IDENTIFIER)) {
                    throw HammerError(
                        peek().line,
                        peek().column,
                        "Syntax Error",
                        "Expected parameter name inside parentheses.",
                        "Parameters must be valid names separated by commas.",
                        "List the parameter names for your action.",
                        "action add(a, b)\n    give a + b\nend"
                    );
                }
                Token param = advance();
                params.push_back(param.text);
            } while (match(TokenType::TOKEN_COMMA));
        }
        if (!match(TokenType::TOKEN_RPAREN)) {
            throw HammerError(
                peek().line,
                peek().column,
                "Syntax Error",
                "Missing ')' after action parameter list.",
                "Parameter lists opened with '(' must be closed with ')'.",
                "Add a closing parenthesis ')' after your parameter list.",
                "action calculate(x, y)\n    give x * y\nend"
            );
        }
    }
    match(TokenType::TOKEN_COLON);
    match(TokenType::TOKEN_NEWLINE);

    auto body = parseBlockUntil({TokenType::TOKEN_END});
    if (!match(TokenType::TOKEN_END)) {
        throw HammerError(
            peek().line,
            peek().column,
            "Syntax Error",
            "Missing 'end' to close action '" + nameTok.text + "' (started on line " + std::to_string(line) + ").",
            "All actions/functions in Hammer Script must be closed with 'end'.",
            "Add 'end' at the bottom of your action definition.",
            "action " + nameTok.text + "()\n    # code here\nend"
        );
    }
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<ActionStmt>(nameTok.text, params, body, line);
}

std::shared_ptr<Stmt> Parser::parseGiveStatement() {
    int line = previous().line;
    std::shared_ptr<Expr> val = nullptr;
    if (!check(TokenType::TOKEN_NEWLINE) && !isAtEnd()) {
        val = parseExpression();
    }
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<GiveStmt>(val, line);
}

std::shared_ptr<Stmt> Parser::parseStopStatement() {
    int line = previous().line;
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<StopStmt>(line);
}

std::shared_ptr<Stmt> Parser::parseSkipStatement() {
    int line = previous().line;
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<SkipStmt>(line);
}

std::shared_ptr<Stmt> Parser::parseIncludeStatement() {
    int line = previous().line;
    std::string path;
    if (check(TokenType::TOKEN_STRING)) {
        Token pathTok = advance();
        path = pathTok.text;
    } else if (check(TokenType::TOKEN_IDENTIFIER)) {
        Token pathTok = advance();
        path = pathTok.text;
    } else {
        throw HammerError(
            peek().line, peek().column,
            "Syntax Error",
            "Expected library name or file path after 'include', 'use', or 'import'.",
            "You can specify a module name or string path.",
            "Examples: use ansi_colors or include \"helpers.hmscp\"",
            "use ansi_colors\ninclude \"math_utils.hmscp\""
        );
    }
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<IncludeStmt>(path, line);
}

std::shared_ptr<Stmt> Parser::parseAttemptStatement() {
    int line = previous().line;
    match(TokenType::TOKEN_COLON);
    match(TokenType::TOKEN_NEWLINE);

    auto tryBody = parseBlockUntil({TokenType::TOKEN_ON, TokenType::TOKEN_END});

    std::string errVar = "problem";
    std::vector<std::shared_ptr<Stmt>> catchBody;

    if (match(TokenType::TOKEN_ON)) {
        consume(TokenType::TOKEN_ERROR, "Expected 'error' after 'on' in 'on error <variable>'");
        if (check(TokenType::TOKEN_IDENTIFIER)) {
            errVar = advance().text;
        }
        match(TokenType::TOKEN_COLON);
        match(TokenType::TOKEN_NEWLINE);
        catchBody = parseBlockUntil({TokenType::TOKEN_END});
    }

    consume(TokenType::TOKEN_END, "Expected 'end' to close 'attempt' block");
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<AttemptStmt>(tryBody, errVar, catchBody, line);
}

std::shared_ptr<Stmt> Parser::parseExpressionOrAssignStatement() {
    int line = peek().line;

    // Check if this is an assignment: name = expr OR name[idx] = expr
    if (check(TokenType::TOKEN_IDENTIFIER)) {
        size_t save = current;
        Token nameTok = advance();

        // Case 1: name = expr
        if (match(TokenType::TOKEN_ASSIGN)) {
            auto val = parseExpression();
            match(TokenType::TOKEN_NEWLINE);
            return std::make_shared<SetStmt>(nameTok.text, val, nullptr, line);
        }

        // Case 2: name[idx] = expr
        if (match(TokenType::TOKEN_LBRACKET)) {
            auto idx = parseExpression();
            if (match(TokenType::TOKEN_RBRACKET) && match(TokenType::TOKEN_ASSIGN)) {
                auto val = parseExpression();
                match(TokenType::TOKEN_NEWLINE);
                return std::make_shared<SetStmt>(nameTok.text, val, idx, line);
            }
        }

        // Revert back
        current = save;
    }

    auto expr = parseExpression();
    match(TokenType::TOKEN_NEWLINE);
    return std::make_shared<ExprStmt>(expr, line);
}

std::vector<std::shared_ptr<Stmt>> Parser::parseBlockUntil(std::initializer_list<TokenType> endTokens) {
    std::vector<std::shared_ptr<Stmt>> statements;
    skipNewlines();

    auto isEnd = [&]() {
        if (isAtEnd()) return true;
        for (TokenType type : endTokens) {
            if (check(type)) return true;
        }
        return false;
    };

    while (!isEnd()) {
        auto stmt = parseStatement();
        if (stmt) {
            statements.push_back(stmt);
        }
        skipNewlines();
    }

    return statements;
}

std::shared_ptr<Expr> Parser::parseExpression() {
    return parseLogicalOr();
}

std::shared_ptr<Expr> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();

    while (match(TokenType::TOKEN_OR)) {
        int line = previous().line;
        auto right = parseLogicalAnd();
        expr = std::make_shared<BinaryExpr>(expr, TokenType::TOKEN_OR, "or", right, line);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::parseLogicalAnd() {
    auto expr = parseComparison();

    while (match(TokenType::TOKEN_AND)) {
        int line = previous().line;
        auto right = parseComparison();
        expr = std::make_shared<BinaryExpr>(expr, TokenType::TOKEN_AND, "and", right, line);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::parseComparison() {
    auto expr = parseAdditive();

    if (match({TokenType::TOKEN_EQUAL, TokenType::TOKEN_NOT_EQUAL,
               TokenType::TOKEN_LT, TokenType::TOKEN_GT,
               TokenType::TOKEN_LTE, TokenType::TOKEN_GTE})) {
        TokenType op = previous().type;
        std::string opStr = previous().text;
        int line = previous().line;
        auto right = parseAdditive();
        return std::make_shared<BinaryExpr>(expr, op, opStr, right, line);
    }

    if (match(TokenType::TOKEN_IS)) {
        int line = previous().line;
        if (match(TokenType::TOKEN_NOT)) {
            auto right = parseAdditive();
            return std::make_shared<BinaryExpr>(expr, TokenType::TOKEN_NOT_EQUAL, "!=", right, line);
        }
        if (match(TokenType::TOKEN_GREATER)) {
            consume(TokenType::TOKEN_THAN, "Expected 'than' after 'is greater'");
            auto right = parseAdditive();
            return std::make_shared<BinaryExpr>(expr, TokenType::TOKEN_GT, ">", right, line);
        }
        if (match(TokenType::TOKEN_LESS)) {
            consume(TokenType::TOKEN_THAN, "Expected 'than' after 'is less'");
            auto right = parseAdditive();
            return std::make_shared<BinaryExpr>(expr, TokenType::TOKEN_LT, "<", right, line);
        }
        if (match(TokenType::TOKEN_AT)) {
            if (match(TokenType::TOKEN_LEAST)) {
                auto right = parseAdditive();
                return std::make_shared<BinaryExpr>(expr, TokenType::TOKEN_GTE, ">=", right, line);
            }
            if (match(TokenType::TOKEN_MOST)) {
                auto right = parseAdditive();
                return std::make_shared<BinaryExpr>(expr, TokenType::TOKEN_LTE, "<=", right, line);
            }
            throw HammerError(
                peek().line,
                peek().column,
                "Syntax Error",
                "Expected 'least' or 'most' after 'is at'.",
                "In Hammer Script, use 'is at least' for >= and 'is at most' for <=.",
                "Specify 'least' or 'most'.",
                "if age is at least 18 then\n    show \"Adult\"\nend"
            );
        }

        auto right = parseAdditive();
        return std::make_shared<BinaryExpr>(expr, TokenType::TOKEN_EQUAL, "==", right, line);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::parseAdditive() {
    auto expr = parseMultiplicative();

    while (match({TokenType::TOKEN_PLUS, TokenType::TOKEN_MINUS})) {
        TokenType op = previous().type;
        std::string opStr = previous().text;
        int line = previous().line;
        auto right = parseMultiplicative();
        expr = std::make_shared<BinaryExpr>(expr, op, opStr, right, line);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::parseMultiplicative() {
    auto expr = parsePower();

    while (match({TokenType::TOKEN_STAR, TokenType::TOKEN_SLASH, TokenType::TOKEN_PERCENT})) {
        TokenType op = previous().type;
        std::string opStr = previous().text;
        int line = previous().line;
        auto right = parsePower();
        expr = std::make_shared<BinaryExpr>(expr, op, opStr, right, line);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::parsePower() {
    auto expr = parseUnary();

    if (match(TokenType::TOKEN_CARET)) {
        int line = previous().line;
        auto right = parsePower();
        expr = std::make_shared<BinaryExpr>(expr, TokenType::TOKEN_CARET, "^", right, line);
    }

    return expr;
}

std::shared_ptr<Expr> Parser::parseUnary() {
    if (match({TokenType::TOKEN_MINUS, TokenType::TOKEN_NOT})) {
        TokenType op = previous().type;
        int line = previous().line;
        auto right = parseUnary();
        return std::make_shared<UnaryExpr>(op, right, line);
    }

    return parseCallOrIndex();
}

std::shared_ptr<Expr> Parser::parseCallOrIndex() {
    auto expr = parsePrimary();

    while (true) {
        if (match(TokenType::TOKEN_LPAREN)) {
            int line = previous().line;
            std::vector<std::shared_ptr<Expr>> args;
            if (!check(TokenType::TOKEN_RPAREN)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::TOKEN_COMMA));
            }
            if (!match(TokenType::TOKEN_RPAREN)) {
                throw HammerError(
                    peek().line,
                    peek().column,
                    "Syntax Error",
                    "Missing ')' after function arguments.",
                    "Function calls opened with '(' must be closed with ')'.",
                    "Add ')' at the end of the argument list.",
                    "greet(\"Alice\")\nadd(5, 10)"
                );
            }
            expr = std::make_shared<CallExpr>(expr, args, line);
        } else if (match(TokenType::TOKEN_LBRACKET)) {
            int line = previous().line;
            auto index = parseExpression();
            std::shared_ptr<Expr> sliceEnd = nullptr;
            if (match(TokenType::TOKEN_TO) || match(TokenType::TOKEN_COLON)) {
                sliceEnd = parseExpression();
            }
            if (!match(TokenType::TOKEN_RBRACKET)) {
                throw HammerError(
                    peek().line,
                    peek().column,
                    "Syntax Error",
                    "Missing ']' after index or slice expression.",
                    "Square bracket index access must be closed with ']'.",
                    "Add ']' after your index or slice expression.",
                    "item = my_list[0]\nsub = my_list[1 to 4]\nval = config[\"theme\"]"
                );
            }
            expr = std::make_shared<IndexExpr>(expr, index, sliceEnd, line);
        } else {
            break;
        }
    }

    return expr;
}

std::shared_ptr<Expr> Parser::parseAskExpression() {
    int line = previous().line;
    std::shared_ptr<Expr> prompt = nullptr;
    if (!check(TokenType::TOKEN_NEWLINE) && !check(TokenType::TOKEN_RPAREN) && !isAtEnd()) {
        prompt = parseExpression();
    }
    return std::make_shared<AskExpr>(prompt, line);
}

std::shared_ptr<Expr> Parser::parsePrimary() {
    int line = peek().line;
    int col = peek().column;

    if (match(TokenType::TOKEN_ASK)) {
        return parseAskExpression();
    }

    if (match(TokenType::TOKEN_NUMBER)) {
        double val = std::stod(previous().text);
        return std::make_shared<LiteralExpr>(Value::makeNumber(val), line);
    }

    if (match(TokenType::TOKEN_RAW_STRING)) {
        return std::make_shared<LiteralExpr>(Value::makeString(previous().text), line);
    }

    if (match(TokenType::TOKEN_STRING)) {
        std::string raw = previous().text;
        size_t openBrace = raw.find('{');
        if (openBrace != std::string::npos && raw.find('}', openBrace) != std::string::npos) {
            std::vector<std::shared_ptr<Expr>> parts;
            size_t idx = 0;
            bool interpolationValid = true;
            while (idx < raw.size()) {
                size_t bracePos = raw.find('{', idx);
                if (bracePos == std::string::npos) {
                    if (idx < raw.size()) {
                        parts.push_back(std::make_shared<LiteralExpr>(Value::makeString(raw.substr(idx)), line));
                    }
                    break;
                }
                if (bracePos > idx) {
                    parts.push_back(std::make_shared<LiteralExpr>(Value::makeString(raw.substr(idx, bracePos - idx)), line));
                }
                size_t endBrace = raw.find('}', bracePos);
                if (endBrace == std::string::npos) {
                    parts.push_back(std::make_shared<LiteralExpr>(Value::makeString(raw.substr(bracePos)), line));
                    break;
                }
                std::string exprCode = raw.substr(bracePos + 1, endBrace - bracePos - 1);
                try {
                    Lexer exprLexer(exprCode);
                    auto exprTokens = exprLexer.tokenize();
                    Parser exprParser(exprTokens);
                    auto parsedSubExpr = exprParser.parseExpression();
                    if (parsedSubExpr && exprParser.isAtEnd()) {
                        parts.push_back(parsedSubExpr);
                    } else {
                        interpolationValid = false;
                        break;
                    }
                } catch (...) {
                    interpolationValid = false;
                    break;
                }
                idx = endBrace + 1;
            }
            if (interpolationValid && !parts.empty()) {
                return std::make_shared<InterpolatedStringExpr>(parts, line);
            }
        }
        return std::make_shared<LiteralExpr>(Value::makeString(raw), line);
    }

    if (match(TokenType::TOKEN_BOOLEAN)) {
        std::string txt = previous().text;
        bool b = (txt == "true" || txt == "yes" || txt == "True" || txt == "Yes" || txt == "TRUE" || txt == "YES");
        return std::make_shared<LiteralExpr>(Value::makeBoolean(b), line);
    }

    if (match(TokenType::TOKEN_NOTHING)) {
        return std::make_shared<LiteralExpr>(Value::makeNothing(), line);
    }

    if (match(TokenType::TOKEN_IDENTIFIER)) {
        return std::make_shared<IdentifierExpr>(previous().text, line);
    }

    // List literal [1, 2, 3]
    if (match(TokenType::TOKEN_LBRACKET)) {
        std::vector<std::shared_ptr<Expr>> elements;
        skipNewlines();
        if (!check(TokenType::TOKEN_RBRACKET)) {
            do {
                skipNewlines();
                elements.push_back(parseExpression());
                skipNewlines();
            } while (match(TokenType::TOKEN_COMMA));
        }
        skipNewlines();
        if (!match(TokenType::TOKEN_RBRACKET)) {
            throw HammerError(
                peek().line,
                peek().column,
                "Syntax Error",
                "Missing ']' at the end of list literal.",
                "A list started with '[' must be closed with ']'.",
                "Add a closing bracket ']' at the end of your list.",
                "my_list = [1, 2, 3, 4]"
            );
        }
        return std::make_shared<ListExpr>(elements, line);
    }

    // Map literal {"a": 1, "b": 2}
    if (match(TokenType::TOKEN_LBRACE)) {
        std::vector<std::pair<std::string, std::shared_ptr<Expr>>> entries;
        skipNewlines();
        if (!check(TokenType::TOKEN_RBRACE)) {
            do {
                skipNewlines();
                std::string key;
                if (match(TokenType::TOKEN_STRING) || match(TokenType::TOKEN_IDENTIFIER)) {
                    key = previous().text;
                } else {
                    throw HammerError(
                        peek().line,
                        peek().column,
                        "Syntax Error",
                        "Expected string or identifier key in map.",
                        "Keys in a map must be strings or identifiers, followed by a colon ':'.",
                        "Use quotes or an identifier for each key in your map.",
                        "profile = {\"name\": \"Alice\", \"age\": 25}"
                    );
                }
                if (!match(TokenType::TOKEN_COLON)) {
                    throw HammerError(
                        peek().line,
                        peek().column,
                        "Syntax Error",
                        "Missing ':' after map key '" + key + "'.",
                        "In map entries, keys and values must be separated by a colon ':'.",
                        "Add a colon ':' after the key name.",
                        "user = {\"username\": \"bob\", \"active\": true}"
                    );
                }
                auto val = parseExpression();
                entries.emplace_back(key, val);
                skipNewlines();
            } while (match(TokenType::TOKEN_COMMA));
        }
        skipNewlines();
        if (!match(TokenType::TOKEN_RBRACE)) {
            throw HammerError(
                peek().line,
                peek().column,
                "Syntax Error",
                "Missing '}' at the end of map literal.",
                "A map started with '{' must be closed with '}'.",
                "Add a closing curly brace '}' to complete the map.",
                "settings = {\"theme\": \"dark\", \"sound\": true}"
            );
        }
        return std::make_shared<MapExpr>(entries, line);
    }

    // Parenthesized expression
    if (match(TokenType::TOKEN_LPAREN)) {
        auto expr = parseExpression();
        if (!match(TokenType::TOKEN_RPAREN)) {
            throw HammerError(
                peek().line,
                peek().column,
                "Syntax Error",
                "Missing ')' after expression.",
                "An expression wrapped in '(' must be closed with ')'.",
                "Add a closing parenthesis ')'.",
                "result = (10 + 20) * 2"
            );
        }
        return expr;
    }

    // Suggest typos if any
    std::string typo = DiagnosticEngine::findClosestMatch(peek().text, KNOWN_KEYWORDS);
    std::string hint = typo.empty() ? "Check if there's a typo or missing operator here." : "Did you mean the keyword '" + typo + "'?";

    throw HammerError(
        line,
        col,
        "Syntax Error",
        "Unexpected token '" + peek().text + "'.",
        "Hammer Script could not understand this token in this position.",
        hint,
        "show \"Hello!\"\nset score to 100"
    );
}

} // namespace Hammer
