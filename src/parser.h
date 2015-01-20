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

namespace Script {

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

    enum InstructionType {
        kInstruction, kLabel, kFun
    };

    class Instruction {
    public:
        InstructionType type;
        std::string name;
        std::vector<Token> params;
        Position pos;

        Instruction() :
                type(kInstruction) {
        }

        virtual ~Instruction() {
        }

        std::string to_str() {
            std::stringstream ss;
            ss << "Instruction：" << name << " type: "
                    << ((type != kInstruction) ? "标签" : "指令");
            for (std::vector<Token>::iterator it = params.begin();
                 it != params.end(); ++it) {
                ss << "\t" << it->to_str();
            }
            return ss.str();
        }

    };

    class Parser {
    private:
        Lexer &lexer;
    public:
        Parser(Lexer &_lexer);

        virtual ~Parser();

        void parse(std::vector<Instruction> &insts,
                std::unordered_map<std::string, Closure> &labels) const;
    };

} /* namespace Script */

#endif /* PARSER_H_ */
