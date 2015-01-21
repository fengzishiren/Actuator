/*
 * Parser.cc
 *
 *  Created on: 2014年6月6日
 *      Author: lunatic
 */


#include "parser.h"
#include "alarm.h"

namespace Script {

    static std::unordered_map<std::string, int> inst_table;

    static void init() {
        inst_table["exit"] = EXIT;
        inst_table["goto"] = GOTO;
        inst_table["call"] = CALL;
        inst_table["say"] = SAY;
        inst_table["print"] = SAY;
        inst_table["set"] = SET;
        inst_table["read"] = READ;
        inst_table["if"] = IF;
    }

    Parser::Parser(Lexer &_lexer) :
            lexer(_lexer) {
        init();
    }

    Parser::~Parser() {
    }


    static void expect(size_t real_type, size_t expected_type, const Position &pos) {
        if (real_type != expected_type) error("语法错误！", pos);
    }

    void Parser::parse(std::vector<Instruction> &insts,
            std::unordered_map<std::string, Closure> &labels) const {
        Closure closure;
        for (bool loop = true; loop;) {
            Instruction inst;
            std::vector<Token> tokens;
            for (; ;) {
                Token token;
                lexer.next_token(token);
                if (token.type == kEnd) {
                    loop = false;
                    break;
                } else if (token.type == kLF)
                    break;
                else
                    tokens.push_back(token);
            }
            if (tokens.empty())
                continue;
            expect(tokens[0].type, kName, tokens[0].pos);
            ///
            // def fun(a, b, c)
            //   ....
            // end
            if (tokens[0].type == kDef) {
                closure.start = insts.size();
                expect(tokens[1].type, kName, tokens[1].pos);
                closure.name = tokens[1].content;
                //parser_args(inst, tokens, 2, tokens.size() - 1);
                for (int i = 2; i < tokens.size() - 1; ++i) {
                    if (i % 2) {
                        expect(tokens[i].type, ',', tokens[i].pos);
                    } else {
                        expect(tokens[i].type, kName, tokens[i].pos);
                        closure.args.push_back(tokens[i].content);
                    }
                }
                expect(tokens.back().type, ')', tokens.front().pos);
            } else if (tokens[0].type == kEnd) {
                closure.end = insts.size();
                labels[closure.name] = closure;
            }
            //add syntax rule:
            //a = 10 => set a 10
            if (tokens.size() == 3 && tokens[1].type == kAssign) {
                tokens[1] = tokens[0];
                tokens[0].content = "set";
            } else if (tokens.size() >= 3 && tokens[1].type == '(' && tokens.back().type == ')') {
                inst.opcode = inst_table["call"];
            }
            const std::string &op = tokens[0].content;
            inst.opcode = inst_table[op];
            inst.pos = tokens[0].pos;

//            if (tokens.size() == 1) {
//                inst.type = kInstruction;
//            } else if (tokens[1].type == ':') {
//                inst.type = kLabel;
//            } else { //处理指令参数
//                inst.type = kInstruction;
            for (size_t i = 1; i < tokens.size(); ++i) {
                switch (tokens[i].type) {
                    case kInt:
                    case kReal:
                    case kString:
                    case kName:
                    case KCmp:
                        inst.params.push_back(tokens[i]);
                        break;
                    default:
                        Log::error(tokens[i].to_str());
                        error("参数不符合要求！", tokens[i].pos);
                }
            };
            insts.push_back(inst);
        }
    }

} /* namespace Script */
