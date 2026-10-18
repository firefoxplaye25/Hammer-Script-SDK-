#ifndef HAMMER_TOKEN_HPP
#define HAMMER_TOKEN_HPP

#include <string>
#include <iostream>

namespace Hammer {

enum class TokenType {
    // Literals
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_RAW_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_BOOLEAN,
    TOKEN_NOTHING,

    // English Keywords
    TOKEN_SET,
    TOKEN_TO,
    TOKEN_SHOW,
    TOKEN_PRINT,
    TOKEN_ASK,
    TOKEN_IF,
    TOKEN_THEN,
    TOKEN_ELSE,
    TOKEN_ELSEIF,
    TOKEN_END,
    TOKEN_IS,
    TOKEN_NOT,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_GREATER,
    TOKEN_LESS,
    TOKEN_THAN,
    TOKEN_AT,
    TOKEN_LEAST,
    TOKEN_MOST,
    TOKEN_REPEAT,
    TOKEN_TIMES,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_EACH,
    TOKEN_IN,
    TOKEN_ACTION,
    TOKEN_FUNCTION,
    TOKEN_GIVE,
    TOKEN_RETURN,
    TOKEN_STOP,
    TOKEN_BREAK,
    TOKEN_SKIP,
    TOKEN_CONTINUE,
    TOKEN_INCLUDE,
    TOKEN_ATTEMPT,
    TOKEN_ON,
    TOKEN_ERROR,
    TOKEN_WITH,
    TOKEN_FROM,
    TOKEN_STEP,

    // Symbols and Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,
    TOKEN_CARET,
    TOKEN_ASSIGN,          // =
    TOKEN_EQUAL,           // ==
    TOKEN_NOT_EQUAL,       // !=
    TOKEN_LT,              // <
    TOKEN_GT,              // >
    TOKEN_LTE,             // <=
    TOKEN_GTE,             // >=
    TOKEN_LPAREN,          // (
    TOKEN_RPAREN,          // )
    TOKEN_LBRACKET,        // [
    TOKEN_RBRACKET,        // ]
    TOKEN_LBRACE,          // {
    TOKEN_RBRACE,          // }
    TOKEN_COMMA,           // ,
    TOKEN_COLON,           // :
    TOKEN_DOT,             // .
    TOKEN_NEWLINE,

    TOKEN_EOF,
    TOKEN_UNKNOWN
};

struct Token {
    TokenType type;
    std::string text;
    int line;
    int column;

    Token(TokenType type = TokenType::TOKEN_UNKNOWN, std::string text = "", int line = 1, int column = 1)
        : type(type), text(std::move(text)), line(line), column(column) {}
};

const char* tokenTypeToString(TokenType type);

} // namespace Hammer

#endif // HAMMER_TOKEN_HPP
