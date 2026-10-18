#ifndef HAMMER_LEXER_HPP
#define HAMMER_LEXER_HPP

#include "token.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace Hammer {

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t cursor = 0;
    int line = 1;
    int column = 1;
    std::unordered_map<std::string, TokenType> keywords;

    void initKeywords();
    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);

    Token scanToken();
    Token scanNumber();
    Token scanString(char quote);
    Token scanIdentifier();
    void skipComment();
    void skipNoteBlock();
};

} // namespace Hammer

#endif // HAMMER_LEXER_HPP
