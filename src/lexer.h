/*
 * Lexer.h
 *
 *      Author: Lunatic
 */

#ifndef LEXER_H_
#define LEXER_H_
#include <iostream>
#include <string>
#include <sstream>

#include "lexer.h"

namespace Script {

enum TokenType {
    kInt, kReal, kString, kName, kColon
};


class Position {
public:
    size_t x, y;
    Position(size_t _x, size_t _y) :
        x(_x), y(_y) {
    }

    std::string to_str() const {
        std::stringstream ss;
        ss << "[" << x << ", " << y << "]" << std::endl;
        return ss.str();
    }
};


class Token {
public:
    TokenType type;
    std::string token;
    Position pos;

    Token(TokenType _type, std::string _token, size_t row, size_t col) :
        type(_type), token(_token), pos(row, col) {
    }
};

class Lexer {
public:
    std::string text;
    size_t offset, col, row;

    Lexer(const std::string& _text) :
        text(_text), offset(0), col(0), row(0) {
    }
    virtual ~Lexer();

    bool finish() {
        return offset == text.size();
    }

    bool get_name(std::string& name);
    bool get_string(std::string& name);
    bool get_real(std::string& name);
    bool get_int(std::string& name);
    void skip_space();
    void forward();
    Token next_token();
};

} /* namespace Script */

#endif /* LEXER_H_ */
