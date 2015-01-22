/*
 * Parser.h
 *
 *  Created on: 2014年6月6日
 *      Author: lunatic
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <unordered_map>
#include <vector>
#include "lexer.h"


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
#define ERR 13


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
        VAR, STRING, INT, FLOAT
    };

    class Argument {
    private:

    public:
        union {
            std::string s;
            INT num;
            FLOAT real;
        } val;
        Type tag;
    };

    class Instruction {
    public:
        int opcode;
        std::vector<Argument> params;
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

        void match(TokenType t);

        void embed_stmts();

        void stmts();

        void move_tokens();


        void parse();
    };

} /* namespace Script */

#endif /* PARSER_H_ */
