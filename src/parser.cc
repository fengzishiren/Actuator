/*
 * Parser.cc
 *
 *  Created on: 2014年6月6日
 *      Author: lunatic
 */

#include <cstdlib>

#include "parser.h"
#include "alarm.h"

namespace Script {
    Value Value::NIL = Value((Type) -1, Position::NULL_POS);
    static std::unordered_map<std::string, int> inst_table;
    static const std::string cmps[] = {"==", "!=", "<=", ">=", "<", ">"};

    static void init() {
        inst_table["exit"] = EXIT;
        inst_table["goto"] = GOTO;
        inst_table["call"] = CALL;
        inst_table["say"] = SAY;
        inst_table["print"] = SAY;
        inst_table["set"] = SET;
        inst_table["read"] = READ;
        inst_table["=="] = EQ;
        inst_table["!="] = NE;
        inst_table["<="] = LE;
        inst_table[">="] = GE;
        inst_table["<"] = GT;
        inst_table[">"] = LS;
        inst_table["return"] = RET;
        inst_table["error"] = ERR;
    }

    static bool is_cmp(const std::string &cmp) {
        for (auto &s: cmps) {
            if (cmp == s) return true;
        }
        return false;
    }

    Parser::Parser(Lexer &_lexer, std::vector<Instruction> &_insts, std::unordered_map<size_t, size_t> &_labels, std::unordered_map<std::string, Closure> &_closures)
            :
            lexer(_lexer), insts(_insts), labels(_labels), closures(_closures) {
        init();
    }

    Parser::~Parser() {
    }

    template<class T>
    static void expect(T real, T expected, const Position &pos) {
        if (real != expected) error("语法错误！", pos);
    }

    static INT str2int(const std::string &val) {
        //char *err = NULL;
        //long long retval = std::strtoll(val.c_str(), &err, 10);
        //if (*err == '\0')
        //return retval;
        return std::strtoll(val.c_str(), NULL, 10);
    }

    static FLOAT str2float(const std::string &val) {
        return std::strtold(val.c_str(), NULL);
    }

    static Value tok2arg(Token &tok) {
        Value arg(tok.pos);
        switch (tok.tag) {
            case kName:
                arg.set_string(tok.content, kVAR);
                break;
            case kString:
                arg.set_string(tok.content);
                break;
            case kInt:
                arg.set_int(str2int(tok.content));
                break;
            case kReal:
                arg.set_float(str2float(tok.content));
                break;
        }
        return arg;
    }

    // def fun(a, b, c)
    //   ....
    // end
    void Parser::def() {
        Closure closure;
        closure.start = insts.size();
        expect(tokens[1].tag, kName, tokens[1].pos);
        closure.name = tokens[1].content;
        for (int i = 2; i < tokens.size() - 1; ++i) {
            if (i % 2) {
                expect(tokens[i].tag, (Tag) ',', tokens[i].pos);
            } else {
                expect(tokens[i].tag, kName, tokens[i].pos);
                closure.args.push_back(tokens[i].content);
            }
        }
        expect(tokens.back().tag, (Tag) ')', tokens.front().pos);
        stmts();
        match(kEnd);
        closures[closure.name] = closure;

    }

    void Parser::build_inst(Instruction &inst) {
        const std::string &op = tokens[0].content;
        inst.opcode = inst_table[op];
        inst.pos = tokens[0].pos;

        for (size_t i = 1; i < tokens.size(); ++i) {
            switch (tokens[i].tag) {
                case kInt:
                case kReal:
                case kString:
                case kName:
                case kCmp:
                    inst.params.push_back(tok2arg(tokens[i]));
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
            if (token.tag == kEnd) {
                eof = true;
                break;
            } else if (token.tag == kLF)
                break;
            else
                tokens.push_back(token);
        }
        if (eof)
            tokens.push_back(Token(kEOF));
        else if (tokens.empty())
            move_tokens();
    }

    static void sugar(std::vector<Token> &tokens, Instruction &inst) {
        Token &tok = tokens.front();
        if (tok.tag == kName) {
            if (tokens.size() == 3 && tokens[1].tag == kAssign) {
                tokens[1] = tokens[0];
                tokens[0].content = "set";
            } else if (tokens.size() >= 3 && tokens[1].tag == '(' && tokens.back().tag == ')') {
                inst.opcode = inst_table["call"];
            } else if (tok.content == "if" && is_cmp(tokens[2].content)) {
                // if a == b
                assert(tokens.size() == 4, "语法错误", tok.pos);
                tokens[0] = tokens[2];
                tokens.erase(tokens.end() - 2);
            }
        }
    }

    void Parser::match(Tag t) {
        Token &tok = tokens.front();
        if (tok.tag != t) {
            error(format("unexpected %s", tok.content.c_str()), tok.pos);
        }
        move_tokens();
    }

    void Parser::embed_stmts() {
        move_tokens();
        do {
            Instruction inst;
            ///tokens
            sugar(tokens, inst);
            Token &tok = tokens.front();
            switch (tok.tag) {
                case kCmp: {
                    size_t before = insts.size();
                    build_inst(inst);
                    assert(inst.params.size() == 2, "if语句语法错误", tok.pos);
                    embed_stmts();
                    match(kEnd);
                    size_t after = insts.size();
                    labels[before] = after;
                    break;
                }
                case kName:
                    build_inst(inst);
                    move_tokens();
                    break;
                case kRet:
                    build_inst(inst);
                    assert(inst.params.size() == 1, "仅支持返回一个值", tok.pos);
                    move_tokens();
                case kEnd:
                    continue;
                case kEOF:
                    error("unexpected EOF", tok.pos);
                default:
                    error(format("unexpected %s", tok.content.c_str()), tok.pos);
            };
            insts.push_back(inst);

        } while ((tokens.front().tag != kEnd));
    }

    void Parser::stmts() {

        move_tokens();
        do {
            Instruction inst;
            ///tokens
            sugar(tokens, inst);
            Token &tok = tokens.front();
            switch (tok.tag) {
                case kDef:
                    def();
                    break;
                case kCmp: {
                    size_t before = insts.size();
                    build_inst(inst);
                    assert(inst.params.size() == 2, "if语句语法错误", tok.pos);
                    embed_stmts();
                    match(kEnd);
                    size_t after = insts.size();
                    labels[before] = after;
                    break;
                }
                case kEnd:
                    move_tokens();
                case kEOF:
                    continue;
                case kName:
                    build_inst(inst);
                default:
                    error(format("unexpected %s", tok.content.c_str()), tok.pos);
            };
            insts.push_back(inst);

        } while (tokens.front().tag != kEOF);
    }


    void Parser::parse() {
        stmts();
    }

} /* namespace Script */
