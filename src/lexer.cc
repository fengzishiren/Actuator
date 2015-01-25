/*
 * Lexer.cc
 *
 *      Author: Lunatic
 */

#include "lexer.h"

#include <unordered_map>

namespace Script {

    static std::unordered_map<std::string, Tag> keywords;
    Position Position::NULL_POS = Position();

    static void fill() {
        keywords["def"] = kDef;
        keywords["end"] = kEnd;
        keywords["return"] = kRet;
    }

    static Tag find(const std::string &keyword) {
        std::unordered_map<std::string, Tag>::iterator it;
        return (it = keywords.find(keyword)) != keywords.end() ? it->second : kName;
    }

    Lexer::Lexer(const std::string &_text) :
            text(_text), offset(0), row(0), col(0) {
        fill();
    }

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


    Token Lexer::next_token() {
        Token tok(row, col);
        while (skip_space() || skip_comment());

        if (finish()) {
            tok.tag = kEOF;
            return tok;
        };

        char peek = text[offset];
        switch (peek) {
            case ':':
            case '(':
            case ')':
            case '{':
            case '}':
            case ',':
                tok.content += peek;
                tok.tag = (Tag) peek;
                forward();
                break;
            case '\n':
                tok.tag = kLF;
                forward();
                break;
            case '=':
                tok.content += text[offset];
                forward();
                if (finish())
                    error("unexpected EOF！", row, col);
                if (text[offset] == '=') {
                    tok.tag = kCmp;
                    tok.content += text[offset];
                } else tok.tag = kAssign;
                forward();
                break;
            case '!':
                tok.content += text[offset];
                forward();
                if (finish())
                    error("unexpected EOF！", row, col);
                if (text[offset] == '=') {
                    tok.tag = kCmp;
                    tok.content += text[offset];
                } else tok.tag = kNot;
                forward();
                break;
            case '<':
            case '>':
                tok.content += text[offset];
                forward();
                if (finish())
                    error("unexpected EOF！", row, col);
                if (text[offset] == '=') {
                    tok.content += text[offset];
                    forward();
                }
                tok.tag = kCmp;
            case '"':
                forward();
                do {
                    tok.content += text[offset];
                    forward();
                } while (!finish() && text[offset] != '\"');
                forward();
                tok.tag = kString;
                break;
        }
        if (std::isalnum(peek)) {
            if (std::isdigit(peek)) {
                do {
                    tok.content += text[offset];
                    forward();
                } while (!finish() && std::isdigit(text[offset]));
                if (text[offset] == '.') {
                    do {
                        tok.content += text[offset];
                        forward();
                    } while (!finish() && std::isdigit(text[offset]));
                    tok.tag = kReal;
                } else
                    tok.tag = kInt;
            } else if (isalpha(peek) || peek == '_') {
                do {
                    tok.content += text[offset];
                    forward();
                } while (!finish()
                        && (isalnum(text[offset]) || text[offset] == '_'
                        || text[offset] == '-'));
                tok.tag = find(tok.content);
            } else error(format("unexpected char:'%c'", peek), tok.pos);
        }
        return tok;
    }


} /* namespace Script */
