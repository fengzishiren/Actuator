/*
 * Parser.cc
 *
 *  Created on: 2014年6月6日
 *      Author: lunatic
 */

#include <cstdlib>
#include <cstring>

#include "parser.h"
#include "tool.h"

static const std::string TAG = "parser";

namespace Script {
    namespace GC {
        std::vector<Environment *> env_gc;
        std::vector<Value *> val_gc;

        void gc_vals() {
            for (Value *v : val_gc)
                delete v;
        }

        void gc_envs() {
            for (Environment *e : env_gc)
                delete e;
        }

    }
    Value *Value::NIL = new NullValue();

    static std::unordered_map<std::string, int> inst_table;
    static const std::string cmps[] = {"==", "!=", "<=", ">=", "<", ">"};

    Closure *Environment::find_closure(const std::string &fun_name) {
        Value *v = get(fun_name);
        return v == nullptr || v->type != kClosure ? nullptr : (Closure *) v;
    }

    Value *Environment::get(const std::string &name) {
        Environment *syms = this;
        auto pos = syms->symbols.find(name);
        if (pos == syms->symbols.end())
            do {
                syms = this->outer;
            } while (syms && (pos = syms->symbols.find(name)) == syms->symbols.end());
        return syms == nullptr ? nullptr : pos->second;
    }


    void Environment::set(const std::string &name, Value *value) {
        symbols[name] = value;
    }

    std::string Environment::repr() const {
        std::stringstream ss;
        ss << '{';
        if (outer)
            ss << "\"outer\": $ref,";
        for (auto &pair: symbols) {
            ss << '"' << pair.first << '"' << ':' << pair.second->repr() << ',';
        }
        ss.seekp((long) ss.tellp() - 1);
        ss << '}';
        return ss.str();
    }

    std::string Instruction::repr() const {
        static const char *decs[] = {"EXIT", "GOTO", "CALL", "SAY", "SET",
                "READ", "EQ", "NE", "LE", "GE", "GT", "LS", "RET", "ADD",
                "SUB", "MULL", "DIV" "ERR"};
        std::stringstream ss;
        ss << "<Inst: " << decs[opcode - 1] << '(';
        ss << join(params, ',');
        ss << ")>";
        return ss.str();
    }


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
        inst_table["add"] = ADD;
        inst_table["sub"] = SUB;
        inst_table["mul"] = MUL;
        inst_table["div"] = DIV;
        inst_table["error"] = ERR;
    }

    static bool is_cmp(const std::string &cmp) {
        for (auto &s: cmps) {
            if (cmp == s) return true;
        }
        return false;
    }

    Parser::Parser(Lexer &_lexer, std::vector<Instruction> &_insts, std::unordered_map<size_t, size_t> &_labels) :
            lexer(_lexer), insts(_insts), labels(_labels) {
        init();
    }

    Parser::~Parser() {
    }

    template<class T>
    static void expect(T real, T expected, const Position &pos) {
        if (real != expected) error("syntax error!！", pos);
    }

    static INT str2int(const std::string &val) {
        return std::strtoll(val.c_str(), NULL, 10);
    }

    static FLOAT str2float(const std::string &val) {
        return std::strtold(val.c_str(), NULL);
    }

    static Value *tok2arg(Token &tok) {
        Value *v;
        switch (tok.tag) {
            case kName:
                v = new VarValue(tok.content, tok.pos);
                break;
            case kString:
                v = new StrValue(tok.content, tok.pos);
                break;
            case kInt:
                v = new IntValue(str2int(tok.content), tok.pos);
                break;
            case kReal:
                v = new FloatValue(str2float(tok.content), tok.pos);
                break;
            default:
                error(format("illegal arg: %s！", tok.content.c_str()), tok.pos);

        }
        return v;
    }

// def fun(a, b, c)
//   ....
// end

// -> set fun = (a,b,c){}
    void Parser::define() {
        Instruction inst;
        IntValue *after = new IntValue(0, Position::NULL_POS);
        Instruction goto_inst;
        goto_inst.opcode = GOTO;
        goto_inst.pos = tokens[0].pos;
        goto_inst.params.push_back(after);
        insts.push_back(goto_inst);

        Closure *closure = new Closure(tokens[1].pos);
        closure->start = insts.size();
        expect(tokens[1].tag, kName, tokens[1].pos);
        closure->name = tokens[1].content;
        expect(tokens[2].tag, (Tag) '(', tokens.front().pos);

        for (int i = 3; i < tokens.size() - 1; ++i) {
            if (i % 2) {
                expect(tokens[i].tag, kName, tokens[i].pos);
                closure->args.push_back(tokens[i].content);
            } else {
                expect(tokens[i].tag, (Tag) ',', tokens[i].pos);
            }
        }
        expect(tokens.back().tag, (Tag) ')', tokens.front().pos);
        embed_stmts();
        match(kEnd);
        closure->end = insts.size();
        after->set_val((INT) closure->end);
        inst.opcode = SET;
        inst.pos = tokens[0].pos;
        inst.params.push_back(new VarValue(closure->name, closure->pos));
        inst.params.push_back(closure);
        insts.push_back(inst);
        Log::debug(TAG, "inst closure: " + inst.repr());
    }

    static inline Instruction &check_args(Instruction &inst) {
        switch (inst.opcode) {
            //TODO
        }
        return inst;

    }

    Instruction &Parser::gen_inst() {
        Instruction inst;
        const std::string &op = tokens[0].content;
        inst.opcode = inst_table[op];
        assert(inst.opcode != 0, format("unrecognized \"%s\"", op.c_str()), tokens[0].pos);
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
                    Log::error(tokens[i].repr());
                    error(format("illegal arg: %s！", tokens[i].content.c_str()), tokens[i].pos);
            }
        }

        insts.push_back(check_args(inst));
        return insts.back();
    }

    void Parser::move() {
        static bool eof = false;
        tokens.clear();
        for (; !eof;) {
            Token token = lexer.next_token();
            if (token.tag == kEOF) {
                eof = true;
                break;
            } else if (token.tag == kLF) {
                if (tokens.empty()) continue;
                else break;
            } else
                tokens.push_back(token);
        }
        if (eof)
            tokens.push_back(Token(kEOF));
        Log::debug(TAG, "moved! size: %zu tokens: %s", tokens.size(), join(tokens, ' ').c_str());
    }

    static inline void sugar(std::vector<Token> &tokens) {
        Token &tok = tokens.front();
        if (tok.tag == kName) {
            if (tokens.size() == 3 && tokens[1].tag == kAssign) {
                tokens[1] = tokens[0];
                tokens[0].content = "set";
            } else if (tokens.size() >= 3 && tokens[1].tag == '(' && tokens.back().tag == ')') {
                tokens.insert(tokens.begin(), Token("call", kName, tok.pos));
                tokens.erase(tokens.begin() + 2);
                tokens.erase(tokens.end() - 1);
            } else if (tok.content == "if" && is_cmp(tokens[2].content)) {
                // if a == b
                // == a b
                assert(tokens.size() == 4, "syntax error!", tok.pos);
                tokens[0] = tokens[2];
                tokens.erase(tokens.end() - 2);
            }
        } else if (tok.tag == kLoop) {
            // loop a == b
            // == a b
            assert(tokens.size() == 4, "syntax error!", tok.pos);
            tokens[0] = tokens[2];
            tokens[0].tag = kLoop;
            tokens.erase(tokens.end() - 2);
        }
    }

    void Parser::match(Tag t) {
        Token &tok = tokens.front();
        if (tok.tag != t) {
            error(format("unexpected %s", tok.content.c_str()), tok.pos);
        }
        move();
    }

    void Parser::embed_stmts() {
        move();
        do {
            sugar(tokens);
            Token &tok = tokens.front();
            switch (tok.tag) {
                case kCmp: {
                    size_t before = insts.size();
                    Instruction &inst = gen_inst();
                    assert(inst.params.size() == 2, "syntax error!", tok.pos);
                    embed_stmts();
                    match(kEnd);
                    size_t after = insts.size();
                    labels[before] = after;
                    break;
                }
                case kLoop: {
                    size_t before = insts.size();
                    Instruction &inst = gen_inst();
                    assert(inst.params.size() == 2, "syntax error!", tok.pos);
                    embed_stmts();
                    match(kEnd);
                    size_t after = insts.size();
                    labels[before] = after + 1;
                    IntValue *front = new IntValue(before, tok.pos);
                    Instruction goto_inst;
                    goto_inst.opcode = GOTO;
                    goto_inst.pos = tokens[0].pos;
                    goto_inst.params.push_back(front);
                    insts.push_back(goto_inst);
                    break;
                };
                case kName:
                    gen_inst();
                    move();
                    break;
                case kRet: {
                    Instruction &inst = gen_inst();
                    assert(inst.params.size() == 1, "only one value returned", tok.pos);
                    move();
                    break;
                };
                default:
                    error(format("unexpected \"%s\"", tok.content.c_str()), tok.pos);
            };
        } while (tokens.front().tag != kEnd);
    }

    void Parser::stmts() {
        move();
        do {
            sugar(tokens);
            Token &tok = tokens.front();
            switch (tok.tag) {
                case kDef:
                    define();
                    break;
                case kCmp: {
                    Log::debug(TAG, join(tokens, ','));
                    size_t before = insts.size();
                    Instruction &inst = gen_inst();
                    Log::debug(TAG, inst.repr());
                    assert(inst.params.size() == 2, "syntax error!", tok.pos);
                    embed_stmts();
                    match(kEnd);
                    size_t after = insts.size();
                    labels[before] = after;
                    break;
                }
                case kLoop: {
                    size_t before = insts.size();
                    Instruction &inst = gen_inst();
                    assert(inst.params.size() == 2, "syntax error!", tok.pos);
                    embed_stmts();
                    match(kEnd);
                    size_t after = insts.size();
                    labels[before] = after + 1;
                    IntValue *front = new IntValue(before, tok.pos);
                    Instruction goto_inst;
                    goto_inst.opcode = GOTO;
                    goto_inst.pos = tokens[0].pos;
                    goto_inst.params.push_back(front);
                    insts.push_back(goto_inst);
                    break;
                };
                case kEOF:
                    continue;
                case kName: {
                    Instruction &inst = gen_inst();
                    move();
                    break;
                };
                default:
                    error(format("unexpected \"%s\"", tok.content.c_str()), tok.pos);
            };
        } while (tokens.front().tag != kEOF);
    }


    void Parser::parse() {
        stmts();
    }


} /* namespace Script */
