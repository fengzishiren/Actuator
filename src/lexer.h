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

namespace Script {

    enum Tag {
        kInt, kReal, kString, kRet, kCmp, kAssign, kDef, kName, kLF, kEnd, kEOF
    };

/*
 *
 * 定位到文件的具体位置
 */
    class Position {
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
            x = (int) x;
            y = (int) y;
        }

        std::string to_str() const {
            //assert(x >= 0 && y >= 0);
//		if (x < 0 || y < 0)
//			return std::string();
            std::stringstream ss;
            //Note: 从0开始
            ss << "[" << x + 1 << ", " << y + 1 << "]";
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

        Token(Tag _type) :
                tag(_type) {
        }

        std::string to_str() const {
            std::stringstream ss;
            static const char *typeinfo[] = {"kInt", "kReal", "kString", "KCmp",
                    "kName", "kColon"};
            ss << "Tag: " << typeinfo[tag] << "\t" << "Token: " << content
                    << "\t" << " Pos: " << pos.to_str();
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

        bool get_name(std::string &name);

        bool get_string(std::string &name);

        bool get_num(std::string &name, bool &is_int);

        bool get_int(std::string &name);

        bool get_assign(std::string &name);

        bool get_cmp(std::string &name);

        bool skip_space();

        bool skip_comment();

        void forward();

        Token &next_token(Token &token);
    };

} /* namespace Script */

#endif /* LEXER_H_ */
