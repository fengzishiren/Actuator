/*
 * Lexer.cc
 *
 *      Author: Lunatic
 */

#include "lexer.h"
#include "alarm.h"

#include <cctype>

namespace Script {

    Lexer::~Lexer() {
    }

    void Lexer::forward() {
        if (text[offset++] == '\n') {
            row++;
            col = 0;
        } else {
            col++;
        }
    }

/*
 * exclude '\n'
 */
    bool Lexer::skip_space() {
        size_t old = offset;
        while (offset != text.size() && text[offset] != '\n'
                && std::isspace(text[offset])) {
            forward();
        }
        return offset != old;
    }

    bool Lexer::skip_comment() {
        size_t old = offset;
        if (offset != text.size() && text[offset] == ';')
            do {
                forward();
            } while (offset != text.size() && text[offset] != '\n');
        return offset != old;
    }

    bool Lexer::get_name(std::string &name) {
        bool found = false;
        // letter or '_' 开头
        if (isalpha(text[offset]) || text[offset] == '_') {
            found = true;
            do {
                name += text[offset];
                forward();
            } while (!finish()
                    && (isalnum(text[offset]) || text[offset] == '_'
                    || text[offset] == '-'));
        }
        return found;
    }

    bool Lexer::get_string(std::string &name) {
        if (text[offset] == '\"') {
            forward(); //skip '\"'
            while (!finish() && text[offset] != '\"') {
                name += text[offset];
                forward();
            }
            if (finish())
                error("字符串缺少\"", Position(row, col));
            forward(); //skip '\"'
        }
        return !name.empty();
    }

    bool Lexer::get_num(std::string &name, bool &is_int) {
        bool found = false;
        is_int = true;
        if ((found = get_int(name)) && text[offset] == '.') {
            is_int = false;
            name += '.';
            forward();

            if (!finish() && get_int(name)) {
                found = true;
            } else
                error("语法错误get_num", Position(row, col));
        }
        return found;
    }

    bool Lexer::get_int(std::string &name) {
        bool found = false;
        if ('0' <= text[offset] && text[offset] <= '9') {
            found = true;
            do {
                name += text[offset];
                forward();
            } while (!finish() && '0' <= text[offset] && text[offset] <= '9');
        }
        return found;
    }


    bool Lexer::get_assign(std::string &name) {
        if (text[offset] == '=') {
            name += text[offset];
            forward();
            return true;
        }
        return false;

    }

// > < = >= <= !=
    bool Lexer::get_cmp(std::string &name) {
        switch (text[offset]) {
            case '=':
                name += text[offset];
                forward();
                if (finish())
                    error("语法错误！", Position(row, col));
                if (text[offset] != '=')
                    error("语法错误get_cmp", Position(row, col));
                name += text[offset];
                forward();
                break;
            case '!':
                name += text[offset];
                forward();
                if (finish())
                    error("语法错误！", Position(row, col));
                if (text[offset] != '=')
                    error("语法错误get_cmp", Position(row, col));
                name += text[offset];
                forward();
                break;
            case '<':
            case '>':
                name += text[offset];
                forward();
                if (finish())
                    error("语法错误！", Position(row, col));
                if (text[offset] == '=') {
                    name += text[offset];
                    forward();
                }
        }
        return !name.empty();
    }

    /*

     */
    Token &Lexer::next_token(Token &token) {
        bool isint;
        token.pos.x = row;
        token.pos.y = col;
        while (skip_space() || skip_comment());
        if (finish()) {
            token.type = kEOF;
        } else if (text[offset] == '\n') {
            token.type = kLF;
            forward();
        } else if (get_name(token.content)) {
            if (token.content == "def")
                token.type = kDef;
            else if (token.content == "end")
                token.type = kEnd;
            else
                token.type = kName;
        }
        else if (get_assign(token.content))
            token.type = kAssign;
        else if (get_string(token.content))
            token.type = kString;
        else if (get_num(token.content, isint))
            token.type = isint ? kInt : kReal;
        else if (get_cmp(token.content))
            token.type = kCmp;
        else {
            char peek = text[offset];
            switch (peek) {
                case ':':
                case '(':
                case ')':
                case '{':
                case '}':
                case ',':
                    token.content += peek;
                    token.type = (TokenType) peek;
                    forward();
                    break;
                default:
                    error("无法识别的字符！", token.pos);
            }

            return token;
        }

    }
} /* namespace Script */
