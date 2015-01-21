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

    Parser::Parser(Lexer &_lexer, std::vector <Instruction> &_insts, std::unordered_map <std::string, Closure> &_closures)
            :
            lexer(_lexer), insts(_insts), closures(_closures) {
        init();
    }

    Parser::~Parser() {
    }


    static void expect(size_t real_type, size_t expected_type, const Position &pos) {
        if (real_type != expected_type) error("语法错误！", pos);
    }


    // def fun(a, b, c)
    //   ....
    // end
    void Parser::def() {
        closure.start = insts.size();
        expect(tokens[1].type, kName, tokens[1].pos);
        closure.name = tokens[1].content;
        for (int i = 2; i < tokens.size() - 1; ++i) {
            if (i % 2) {
                expect(tokens[i].type, ',', tokens[i].pos);
            } else {
                expect(tokens[i].type, kName, tokens[i].pos);
                closure.args.push_back(tokens[i].content);
            }
        }
        expect(tokens.back().type, ')', tokens.front().pos);
        stmts();
        expect(kEnd, tokens.front().type, tokens.front().pos);
        closures[closure.name] = closure;

    }

    void build_inst(Instruction &inst) {
        const std::string &op = tokens[0].content;
        inst.opcode = inst_table[op];
        inst.pos = tokens[0].pos;

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
        }
    }

    void Parser::move_tokens() {
        bool eof = false;
        for (; !eof;) {
            Token token;
            lexer.next_token(token);
            if (token.type == kEnd) {
                eof = true;
                break;
            } else if (token.type == kLF)
                break;
            else
                tokens.push_back(token);
        }
        if (eof)
            tokens.push_back(Token(kEOF));
        else if (tokens.empty())
            do_tokens();
    }

    void Parser::stmts() {

        do {
            Instruction inst;
            move_tokens();
            ///tokens
            Token &tok = tokens.front();
            if (tok.type == kName) {
                if (tokens.size() == 3 && tokens[1].type == kAssign) {
                    tokens[1] = tokens[0];
                    tokens[0].content = "set";
                } else if (tokens.size() >= 3 && tokens[1].type == '(' && tokens.back().type == ')') {
                    inst.opcode = inst_table["call"];
                }
            }
            switch (tok.type) {
                case kDef:
                    def();
                    break;
                case kIf: {
                    size_t before = insts.size();
                    build_inst(inst);
                    size_t after = insts.size();
                    label[size_t] = std::make_pair<size_t, size_t>(before, after);
                    break;
                }
                case kEOF:
                    break;
                default:
                    build_inst(inst);
            };
            insts.push_back(inst);

        } while ((tokens.front().type != kEOF));
    }


    void Parser::parse() const {
        stmts();
    }

} /* namespace Script */
