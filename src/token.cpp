#include "../include/token.hpp"

namespace Hammer {

const char* tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::TOKEN_NUMBER: return "NUMBER";
        case TokenType::TOKEN_STRING: return "STRING";
        case TokenType::TOKEN_RAW_STRING: return "RAW_STRING";
        case TokenType::TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TokenType::TOKEN_BOOLEAN: return "BOOLEAN";
        case TokenType::TOKEN_NOTHING: return "NOTHING";
        case TokenType::TOKEN_SET: return "SET";
        case TokenType::TOKEN_TO: return "TO";
        case TokenType::TOKEN_SHOW: return "SHOW";
        case TokenType::TOKEN_PRINT: return "PRINT";
        case TokenType::TOKEN_ASK: return "ASK";
        case TokenType::TOKEN_IF: return "IF";
        case TokenType::TOKEN_THEN: return "THEN";
        case TokenType::TOKEN_ELSE: return "ELSE";
        case TokenType::TOKEN_ELSEIF: return "ELSEIF";
        case TokenType::TOKEN_END: return "END";
        case TokenType::TOKEN_IS: return "IS";
        case TokenType::TOKEN_NOT: return "NOT";
        case TokenType::TOKEN_AND: return "AND";
        case TokenType::TOKEN_OR: return "OR";
        case TokenType::TOKEN_GREATER: return "GREATER";
        case TokenType::TOKEN_LESS: return "LESS";
        case TokenType::TOKEN_THAN: return "THAN";
        case TokenType::TOKEN_AT: return "AT";
        case TokenType::TOKEN_LEAST: return "LEAST";
        case TokenType::TOKEN_MOST: return "MOST";
        case TokenType::TOKEN_REPEAT: return "REPEAT";
        case TokenType::TOKEN_TIMES: return "TIMES";
        case TokenType::TOKEN_WHILE: return "WHILE";
        case TokenType::TOKEN_FOR: return "FOR";
        case TokenType::TOKEN_EACH: return "EACH";
        case TokenType::TOKEN_IN: return "IN";
        case TokenType::TOKEN_ACTION: return "ACTION";
        case TokenType::TOKEN_FUNCTION: return "FUNCTION";
        case TokenType::TOKEN_GIVE: return "GIVE";
        case TokenType::TOKEN_RETURN: return "RETURN";
        case TokenType::TOKEN_STOP: return "STOP";
        case TokenType::TOKEN_BREAK: return "BREAK";
        case TokenType::TOKEN_SKIP: return "SKIP";
        case TokenType::TOKEN_CONTINUE: return "CONTINUE";
        case TokenType::TOKEN_INCLUDE: return "INCLUDE";
        case TokenType::TOKEN_ATTEMPT: return "ATTEMPT";
        case TokenType::TOKEN_ON: return "ON";
        case TokenType::TOKEN_ERROR: return "ERROR";
        case TokenType::TOKEN_WITH: return "WITH";
        case TokenType::TOKEN_FROM: return "FROM";
        case TokenType::TOKEN_STEP: return "STEP";
        case TokenType::TOKEN_PLUS: return "PLUS";
        case TokenType::TOKEN_MINUS: return "MINUS";
        case TokenType::TOKEN_STAR: return "STAR";
        case TokenType::TOKEN_SLASH: return "SLASH";
        case TokenType::TOKEN_PERCENT: return "PERCENT";
        case TokenType::TOKEN_CARET: return "CARET";
        case TokenType::TOKEN_ASSIGN: return "ASSIGN";
        case TokenType::TOKEN_EQUAL: return "EQUAL";
        case TokenType::TOKEN_NOT_EQUAL: return "NOT_EQUAL";
        case TokenType::TOKEN_LT: return "LT";
        case TokenType::TOKEN_GT: return "GT";
        case TokenType::TOKEN_LTE: return "LTE";
        case TokenType::TOKEN_GTE: return "GTE";
        case TokenType::TOKEN_LPAREN: return "LPAREN";
        case TokenType::TOKEN_RPAREN: return "RPAREN";
        case TokenType::TOKEN_LBRACKET: return "LBRACKET";
        case TokenType::TOKEN_RBRACKET: return "RBRACKET";
        case TokenType::TOKEN_LBRACE: return "LBRACE";
        case TokenType::TOKEN_RBRACE: return "RBRACE";
        case TokenType::TOKEN_COMMA: return "COMMA";
        case TokenType::TOKEN_COLON: return "COLON";
        case TokenType::TOKEN_DOT: return "DOT";
        case TokenType::TOKEN_NEWLINE: return "NEWLINE";
        case TokenType::TOKEN_EOF: return "EOF";
        case TokenType::TOKEN_UNKNOWN: default: return "UNKNOWN";
    }
}

} // namespace Hammer
