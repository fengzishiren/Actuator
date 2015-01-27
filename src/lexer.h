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
#include "tool.h"


namespace Script {

    enum Tag {
        kInt, kReal, kString, kRet, kCmp, kIf, kLoop, kNot, kAssign, kDef, kName, kLF, kEnd, kEOF
    };


    class Visualable {
    public:

        virtual ~Visualable() {
        }

        virtual std::string repr() const = 0;

        virtual std::string str() const = 0;
    };

    /*
     *
     * 定位到文件的具体位置
     */
    class Position : public Visualable {
    public:
        int x, y;

        Position() :
                x(-1), y(-1) {
        }

        Position(size_t _x, size_t _y) :
                x((int) _x), y((int) _y) {
        }

        static Position NULL_POS;

        void set(size_t _x, size_t _y) {
            x = (int) _x;
            y = (int) _y;
        }

        std::string str() const {
            return repr();
        }

        std::string repr() const {
            std::stringstream ss;
            //Note: 从0开始
            ss << "(" << x + 1 << ", " << y + 1 << ")";
            return ss.str();
        }
    };

    class Token {
    public:
        Tag tag;
        std::string content;
        Position pos;

        Token() {
        }

        Token(size_t x, size_t y) : pos(x, y) {
        }

        Token(Tag _type) :
                tag(_type) {
        }

        Token(const std::string content, Tag _type, const Position &_pos) :
                content(content), tag(_type), pos(_pos) {

        }

        std::string str() const {
            return repr();
        }

        std::string repr() const {
            std::stringstream ss;
            ss << "<" << '"' << content << "\", " << tag << ' ' << pos.repr() << '>';
            return ss.str();
        }
    };

    class Lexer {
    public:
        const std::string &text;
        size_t offset;
        size_t row, col; //Location:[x, y]

        Lexer(const std::string &_text);

        virtual ~Lexer();

        bool finish() {
            return offset == text.size();
        }

        bool skip_space();

        bool skip_comment();

        void forward();

        Token next_token();
    };

} /* namespace Script */

#endif /* LEXER_H_ */
