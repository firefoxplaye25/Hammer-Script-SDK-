#include "../include/lexer.hpp"
#include "../include/errors.hpp"
#include <cctype>
#include <algorithm>
#include <stdexcept>

namespace Hammer {

static std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return lower;
}

Lexer::Lexer(std::string source) : source(std::move(source)) {
    initKeywords();
}

void Lexer::initKeywords() {
    keywords["set"] = TokenType::TOKEN_SET;
    keywords["to"] = TokenType::TOKEN_TO;
    keywords["show"] = TokenType::TOKEN_SHOW;
    keywords["print"] = TokenType::TOKEN_PRINT;
    keywords["ask"] = TokenType::TOKEN_ASK;
    keywords["if"] = TokenType::TOKEN_IF;
    keywords["then"] = TokenType::TOKEN_THEN;
    keywords["else"] = TokenType::TOKEN_ELSE;
    keywords["elseif"] = TokenType::TOKEN_ELSEIF;
    keywords["elif"] = TokenType::TOKEN_ELSEIF;
    keywords["end"] = TokenType::TOKEN_END;
    keywords["is"] = TokenType::TOKEN_IS;
    keywords["not"] = TokenType::TOKEN_NOT;
    keywords["and"] = TokenType::TOKEN_AND;
    keywords["or"] = TokenType::TOKEN_OR;
    keywords["greater"] = TokenType::TOKEN_GREATER;
    keywords["less"] = TokenType::TOKEN_LESS;
    keywords["than"] = TokenType::TOKEN_THAN;
    keywords["at"] = TokenType::TOKEN_AT;
    keywords["least"] = TokenType::TOKEN_LEAST;
    keywords["most"] = TokenType::TOKEN_MOST;
    keywords["repeat"] = TokenType::TOKEN_REPEAT;
    keywords["times"] = TokenType::TOKEN_TIMES;
    keywords["while"] = TokenType::TOKEN_WHILE;
    keywords["for"] = TokenType::TOKEN_FOR;
    keywords["each"] = TokenType::TOKEN_EACH;
    keywords["in"] = TokenType::TOKEN_IN;
    keywords["action"] = TokenType::TOKEN_ACTION;
    keywords["function"] = TokenType::TOKEN_FUNCTION;
    keywords["def"] = TokenType::TOKEN_FUNCTION;
    keywords["give"] = TokenType::TOKEN_GIVE;
    keywords["return"] = TokenType::TOKEN_RETURN;
    keywords["stop"] = TokenType::TOKEN_STOP;
    keywords["break"] = TokenType::TOKEN_BREAK;
    keywords["skip"] = TokenType::TOKEN_SKIP;
    keywords["continue"] = TokenType::TOKEN_CONTINUE;

    keywords["include"] = TokenType::TOKEN_INCLUDE;
    keywords["use"] = TokenType::TOKEN_INCLUDE;
    keywords["import"] = TokenType::TOKEN_INCLUDE;
    keywords["attempt"] = TokenType::TOKEN_ATTEMPT;
    keywords["try"] = TokenType::TOKEN_ATTEMPT;
    keywords["on"] = TokenType::TOKEN_ON;
    keywords["error"] = TokenType::TOKEN_ERROR;
    keywords["catch"] = TokenType::TOKEN_ERROR;
    keywords["with"] = TokenType::TOKEN_WITH;
    keywords["from"] = TokenType::TOKEN_FROM;
    keywords["step"] = TokenType::TOKEN_STEP;

    // Booleans and nulls
    keywords["true"] = TokenType::TOKEN_BOOLEAN;
    keywords["false"] = TokenType::TOKEN_BOOLEAN;
    keywords["yes"] = TokenType::TOKEN_BOOLEAN;
    keywords["no"] = TokenType::TOKEN_BOOLEAN;
    keywords["nothing"] = TokenType::TOKEN_NOTHING;
    keywords["none"] = TokenType::TOKEN_NOTHING;
    keywords["null"] = TokenType::TOKEN_NOTHING;
}

bool Lexer::isAtEnd() const {
    return cursor >= source.size();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[cursor];
}

char Lexer::peekNext() const {
    if (cursor + 1 >= source.size()) return '\0';
    return source[cursor + 1];
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    char c = source[cursor++];
    column++;
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[cursor] != expected) return false;
    cursor++;
    column++;
    return true;
}

void Lexer::skipComment() {
    while (!isAtEnd() && peek() != '\n' && peek() != '\r') {
        advance();
    }
}

void Lexer::skipNoteBlock() {
    int startLine = line;
    int startCol = column;
    bool atLineStart = false;

    while (!isAtEnd()) {
        char c = peek();
        if (c == '\n') {
            advance();
            line++;
            column = 1;
            atLineStart = true;
            continue;
        }

        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
            continue;
        }

        if (atLineStart) {
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                size_t lookahead = cursor;
                std::string word;
                while (lookahead < source.size() && (std::isalnum(static_cast<unsigned char>(source[lookahead])) || source[lookahead] == '_')) {
                    word += source[lookahead++];
                }
                if (toLower(word) == "end") {
                    while (lookahead < source.size() && (source[lookahead] == ' ' || source[lookahead] == '\t')) {
                        lookahead++;
                    }
                    std::string nextWord;
                    while (lookahead < source.size() && (std::isalnum(static_cast<unsigned char>(source[lookahead])) || source[lookahead] == '_')) {
                        nextWord += source[lookahead++];
                    }
                    if (toLower(nextWord) == "note") {
                        while (cursor < lookahead) {
                            advance();
                        }
                        while (!isAtEnd() && (peek() == ' ' || peek() == '\t')) {
                            advance();
                        }
                        return;
                    }
                }
            }
            atLineStart = false;
        }

        advance();
    }

    throw HammerError(
        startLine, startCol, "Syntax Error",
        "Unterminated 'note' block comment.",
        "You opened a multi-line note block using 'note' but never closed it with 'end note'.",
        "Add 'end note' where you want the comment block to end.",
        "note\n    Your notes here\nend note"
    );
}

Token Lexer::scanNumber() {
    int startCol = column;
    size_t start = cursor;
    while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        advance(); // consume '.'
        while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    std::string text = source.substr(start, cursor - start);
    return Token(TokenType::TOKEN_NUMBER, text, line, startCol);
}

Token Lexer::scanString(char quote) {
    int startLine = line;
    int startCol = column;
    std::string value;

    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\n') {
            line++;
            column = 1;
            value += advance();
        } else if (peek() == '\\') {
            advance(); // consume backslash
            if (isAtEnd()) break;
            char escape = advance();
            switch (escape) {
                case 'n': value += '\n'; break;
                case 't': value += '\t'; break;
                case 'r': value += '\r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                case '\'': value += '\''; break;
                default: value += escape; break;
            }
        } else {
            value += advance();
        }
    }

    if (isAtEnd()) {
        throw HammerError(
            startLine,
            startCol,
            "Syntax Error",
            "Unterminated string literal.",
            "You opened a text string using quotes (" + std::string(1, quote) + ") but forgot to close it before the end of the file.",
            "Add a closing quote (" + std::string(1, quote) + ") at the end of your text.",
            "show " + std::string(1, quote) + "Hello, World!" + std::string(1, quote)
        );
    }

    advance(); // consume closing quote
    if (quote == '\'') {
        return Token(TokenType::TOKEN_RAW_STRING, value, startLine, startCol);
    }
    return Token(TokenType::TOKEN_STRING, value, startLine, startCol);
}

Token Lexer::scanIdentifier() {
    int startCol = column;
    size_t start = cursor;
    while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
        advance();
    }

    std::string text = source.substr(start, cursor - start);
    std::string lower = toLower(text);

    auto it = keywords.find(lower);
    if (it != keywords.end()) {
        TokenType type = it->second;
        return Token(type, text, line, startCol);
    }

    return Token(TokenType::TOKEN_IDENTIFIER, text, line, startCol);
}

Token Lexer::scanToken() {
    char c = advance();

    switch (c) {
        case '(': return Token(TokenType::TOKEN_LPAREN, "(", line, column - 1);
        case ')': return Token(TokenType::TOKEN_RPAREN, ")", line, column - 1);
        case '[': return Token(TokenType::TOKEN_LBRACKET, "[", line, column - 1);
        case ']': return Token(TokenType::TOKEN_RBRACKET, "]", line, column - 1);
        case '{': return Token(TokenType::TOKEN_LBRACE, "{", line, column - 1);
        case '}': return Token(TokenType::TOKEN_RBRACE, "}", line, column - 1);
        case ',': return Token(TokenType::TOKEN_COMMA, ",", line, column - 1);
        case ':': return Token(TokenType::TOKEN_COLON, ":", line, column - 1);
        case '.': return Token(TokenType::TOKEN_DOT, ".", line, column - 1);
        case '+': return Token(TokenType::TOKEN_PLUS, "+", line, column - 1);
        case '-': return Token(TokenType::TOKEN_MINUS, "-", line, column - 1);
        case '*': return Token(TokenType::TOKEN_STAR, "*", line, column - 1);
        case '/': return Token(TokenType::TOKEN_SLASH, "/", line, column - 1);
        case '%': return Token(TokenType::TOKEN_PERCENT, "%", line, column - 1);
        case '^': return Token(TokenType::TOKEN_CARET, "^", line, column - 1);
        case '=':
            if (match('=')) return Token(TokenType::TOKEN_EQUAL, "==", line, column - 2);
            return Token(TokenType::TOKEN_ASSIGN, "=", line, column - 1);
        case '!':
            if (match('=')) return Token(TokenType::TOKEN_NOT_EQUAL, "!=", line, column - 2);
            return Token(TokenType::TOKEN_NOT, "!", line, column - 1);
        case '<':
            if (match('=')) return Token(TokenType::TOKEN_LTE, "<=", line, column - 2);
            return Token(TokenType::TOKEN_LT, "<", line, column - 1);
        case '>':
            if (match('=')) return Token(TokenType::TOKEN_GTE, ">=", line, column - 2);
            return Token(TokenType::TOKEN_GT, ">", line, column - 1);
        default:
            return Token(TokenType::TOKEN_UNKNOWN, std::string(1, c), line, column - 1);
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        char c = peek();

        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
            continue;
        }

        if (c == '\n') {
            int lineNum = line;
            int colNum = column;
            advance();
            line++;
            column = 1;
            // Avoid duplicate consecutive newlines
            if (!tokens.empty() && tokens.back().type != TokenType::TOKEN_NEWLINE) {
                tokens.emplace_back(TokenType::TOKEN_NEWLINE, "\n", lineNum, colNum);
            }
            continue;
        }

        if (c == '#' || (c == '/' && peekNext() == '/')) {
            skipComment();
            continue;
        }

        if (c == '"' || c == '\'') {
            char quote = advance();
            tokens.push_back(scanString(quote));
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(scanNumber());
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            size_t checkPos = cursor;
            std::string idText;
            while (checkPos < source.size() && (std::isalnum(static_cast<unsigned char>(source[checkPos])) || source[checkPos] == '_')) {
                idText += source[checkPos++];
            }
            if (toLower(idText) == "note" && (tokens.empty() || tokens.back().type == TokenType::TOKEN_NEWLINE)) {
                if (checkPos >= source.size() || std::isspace(static_cast<unsigned char>(source[checkPos])) || source[checkPos] == '#') {
                    cursor = checkPos;
                    column += static_cast<int>(idText.size());
                    skipNoteBlock();
                    continue;
                }
            }
            tokens.push_back(scanIdentifier());
            continue;
        }

        tokens.push_back(scanToken());
    }

    tokens.emplace_back(TokenType::TOKEN_EOF, "", line, column);
    return tokens;
}

} // namespace Hammer
