/*
 * Parser.h
 *
 *  Created on: 2014年6月6日
 *      Author: lunatic
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <vector>
#include <unordered_map>
#include "lexer.h"
#include <cstring>


#define EXIT 1
#define GOTO 2
#define CALL 3
#define SAY 4
#define SET 5
#define READ 6
#define EQ 7
#define  NE 8
#define  LE 9
#define  GE 10
#define  GT 11
#define  LS 12
#define RET 13
#define ERR 14


namespace Script {
    typedef long long INT;
    typedef long double FLOAT;

    class Closure {
    public:
        std::vector<std::string> args;
        std::string name;
        size_t start;
        size_t end;

        Closure() : Closure("lambda", 0, 0) {
        }

        Closure(const std::string &_name, size_t _start, size_t _end) : name(_name), start(_start), end(_end) {
        }
    };

    enum Type {
        kVAR, kSTRING, kINT, kFLOAT
    };

    class Value {
        union {
            char *s;
            INT num;
            FLOAT real;
        } val;
    public:
        Position pos;
        Type type;
        static Value NIL;

        Value() : pos(Position::NULL_POS) {
        }

        Value(const std::string &str, const Position &_pos) : type(kSTRING), pos(_pos) {
            set_string(str);
        }

        Value(INT n, const Position &_pos) : type(kINT), pos(_pos) {
            val.num = n;
        }

        Value(FLOAT n, const Position &_pos) : type(kFLOAT), pos(_pos) {
            val.real = n;
        }

        Value(Type _tag, const Position &_pos) : type(_tag), pos(_pos) {
        }

        Value(const Position &_pos) : pos(_pos) {
        }

        ~Value() {
            if (type == kSTRING)
                delete val.s;
        }

        void set_string(const std::string str, Type _tag = kSTRING) {
            val.s = new char[str.length() + 1];
            size_t len = str.copy(val.s, str.length(), 0);
            val.s[len] = '\0';
            type = _tag;
        }

        void set_int(INT n) {
            val.num = n;
            type = kINT;
        }

        void set_float(FLOAT real) {
            val.real = real;
            type = kFLOAT;
        }

        bool operator==(const Value &v) const {
            return equals(v);
        }

        bool less_equals(const Value &v) const {
            if (type != v.type || (type != kINT && type != kFLOAT)) return false;
            return type == kINT ? val.num <= v.val.num : val.real <= v.val.real;
        }

        bool greater_equals(const Value &v) const {
            if (type != v.type || (type != kINT && type != kFLOAT)) return false;
            return type == kINT ? val.num >= v.val.num : val.real >= v.val.real;
        }

        bool less(const Value &v) const {
            if (type != v.type || (type != kINT && type != kFLOAT)) return false;
            return type == kINT ? val.num < v.val.num : val.real < v.val.real;
        }

        bool greater(const Value &v) const {
            if (type != v.type || (type != kINT && type != kFLOAT)) return false;
            return type == kINT ? val.num > v.val.num : val.real > v.val.real;
        }

        bool equals(const Value &v) const {
            int t = (int) type;
            switch (t) {
                case -1:
                    return v.type == -1;
                case kVAR:
                case kSTRING:
                    return !std::strcmp(val.s, v.val.s);
                case kINT:
                    return val.num == v.val.num;
                case kFLOAT:
                    return val.real == val.real;
            }
        }

        std::string repr() const {
            std::stringstream ss;
            switch (type) {
                case kVAR:
                case kSTRING:
                    return std::string(val.s);
                case kINT:
                    ss << val.num;
                    break;
                case kFLOAT:
                    ss << val.real;
                    break;
                default:
                    break;
            }
            return ss.str();
        }
    };

    class Instruction {
    public:
        int opcode;
        std::vector<Value> params;
        Position pos;

        Instruction() {
        }

        ~Instruction() {
        }

        std::string to_str() {
            std::stringstream ss;
            ss << "Instruction：" << opcode;
//            for (std::vector<Token>::iterator it = params.begin();
//                 it != params.end(); ++it) {
//                ss << "\t" << it->to_str();
//            }
            return ss.str();
        }

    };

    class Parser {
    private:
        Lexer &lexer;
        std::vector<Instruction> &insts;
        std::unordered_map<size_t, size_t> &labels;
        std::vector<Token> tokens;
        std::unordered_map<std::string, Closure> &closures;

    public:
        Parser(Lexer &_lexer, std::vector<Instruction> &_insts, std::unordered_map<size_t, size_t> &labels, std::unordered_map<std::string, Closure> &_closures);

        virtual ~Parser();

        void build_inst(Instruction &inst);

        void def();

        void match(Tag t);

        void embed_stmts();

        void stmts();

        void move_tokens();

        void parse();
    };

} /* namespace Script */

#endif /* PARSER_H_ */
