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
        std::vector<Value *> val_gc;

        void gc_vals() {
            for (Value *v : val_gc)
                delete v;
        }


    }
    Value *Value::NIL = new NullValue();

    static std::unordered_map<std::string, INST> inst_table;
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
        //        EXIT, JMP, CALL, SAY, SET, CMP, JEQ, JNE, JLE, JGE, JGT, JLS, RET, ADD, SUB, MUL, DIV, ERR

        static const char *decs[] = {"EXIT", "JMP", "CALL", "SAY", "SET",
                "CMP", "JEQ", "JNE", "JLE", "JGE", "JGT", "JLS", "RET", "ADD",
                "SUB", "MULL", "DIV" "ERR"};
        std::stringstream ss;
        ss << "<Inst: " << decs[opcode] << '(';
        ss << join(params, ',');
        ss << ")>";
        return ss.str();
    }


    static void init() {
        inst_table["exit"] = EXIT;
        inst_table["goto"] = JMP;
        inst_table["call"] = CALL;
        inst_table["say"] = SAY;
        inst_table["print"] = SAY;
        inst_table["set"] = SET;
        inst_table["cmp"] = CMP;
        /* not cond*/
        inst_table["=="] = JNE;
        inst_table["!="] = JEQ;
        inst_table["<="] = JGT;
        inst_table[">="] = JLS;
        inst_table["<"] = JGE;
        inst_table[">"] = JLE;
        /**/
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

    Parser::Parser(Lexer &_lexer, std::vector<Instruction> &_insts) :
            lexer(_lexer), insts(_insts) {
        init();
    }

    Parser::~Parser() {
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

    template<class T>
    static void expect(T real, T expected, const Position &pos) {
        if (real != expected) error("syntax error!！", pos);
    }

    // def fun(a, b, c)
    //   ....
    // end

    // -> set fun = (a,b,c){}
    void Parser::define() {
        Instruction inst;
        IntValue *after = new IntValue(0, Position::NULL_POS);
        Instruction goto_inst;
        goto_inst.opcode = JMP;
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

    // l <= x < h
    struct range {
        size_t l;
        size_t h;
    };
    /*
     "EXIT", "JMP", "CALL", "SAY", "SET","CMP", "EQ",
     "NE", "LE", "GE", "GT", "LS", "RET", "ADD", "SUB",
     "MULL", "DIV" "ERR"
    */
    static const range arg_ranges[] = {{1, 2}, {1, 2}, {2, (size_t) -1}, {1, (size_t) -1}, {2, 3}, {2, 3}, {1, 2},
            {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {2, 3}, {2, 3},
            {2, 3}, {2, 3}, {2, 3},
    };

    static Instruction gen_inst(std::vector<Token> &tokens, size_t start, size_t end) {
        Instruction inst;
        const std::string &op = tokens[0].content;
        auto itop = inst_table.find(op);
        assert(itop != inst_table.end(), format("unrecognized \"%s\"", op.c_str()), tokens[0].pos);
        inst.opcode = itop->second;
        inst.pos = tokens[0].pos;

        for (size_t i = start; i < end; ++i) {
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
        range rg = arg_ranges[inst.opcode];
        assert(rg.l <= inst.params.size() && rg.h > inst.params.size(), "illegal args count", inst.pos);
        return inst;
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
            }
        } else if (tok.tag == kIf) {
            // if a == b
            // == a b
            assert(tokens.size() == 4 && is_cmp(tokens[2].content), "syntax error!", tok.pos);
            tokens[0].content = "cmp";
            std::swap(tokens[2], tokens[3]);

        } else if (tok.tag == kLoop) {
            // loop a == b
            // == a b
            assert(tokens.size() == 4, "syntax error!", tok.pos);
            tokens[0].content = "cmp";
            std::swap(tokens[2], tokens[3]);
        }
    }

    void Parser::match(Tag t) {
        Token &tok = tokens.front();
        if (tok.tag != t) {
            error(format("unexpected %s", tok.content.c_str()), tok.pos);
        }
        move();
    }


    static Instruction gen_jmp_inst(const std::string &name, IntValue *after, Position pos) {
        Instruction jmp_inst;
        jmp_inst.opcode = inst_table[name];
        jmp_inst.pos = pos;
        jmp_inst.params.push_back(after);
        return jmp_inst;
    }

    /**
    *
    * if a == b
    * =>
    * cmp a b
    * geq after
    */
    void Parser::cmp() {
        Token &tok = tokens.front();
        Instruction cmp_inst = gen_inst(tokens, 1, tokens.size() - 1);
        IntValue *after = new IntValue();
        Instruction jmp_inst = gen_jmp_inst(tokens.back().content, after, tok.pos);
        insts.push_back(cmp_inst);
        insts.push_back(jmp_inst);
        embed_stmts();
        match(kEnd);
        after->set_val((INT) insts.size());

    }

    /**
    * loop a== b
    *
    * end
    *
    *
    * L:cmp a b
    * jne x
    *
    * goto L
    */

    void Parser::loop() {
        Token &tok = tokens.front();
        IntValue *before = new IntValue((INT) insts.size(), tok.pos);
        IntValue *after = new IntValue();
        Instruction cmp_inst = gen_inst(tokens, 1, tokens.size() - 1);
        Instruction jmp_inst = gen_jmp_inst(tokens.back().content, after, tok.pos);
        insts.push_back(cmp_inst);
        insts.push_back(jmp_inst);
        embed_stmts();
        match(kEnd);
        Instruction goto_inst = gen_jmp_inst("goto", before, tok.pos);
        insts.push_back(goto_inst);
        after->set_val((INT) insts.size());
    }

    void Parser::embed_stmts() {
        move();
        do {
            sugar(tokens);
            Token &tok = tokens.front();
            switch (tok.tag) {
                case kIf:
                    cmp();
                    break;
                case kLoop:
                    loop();
                    break;
                case kName: {
                    Instruction inst = gen_inst(tokens, 1, tokens.size());
                    insts.push_back(inst);
                    move();
                    break;
                };
                case kRet: {
                    Instruction inst = gen_inst(tokens, 1, tokens.size());
                    insts.push_back(inst);
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
                case kIf:
                    cmp();
                    break;
                case kLoop:
                    loop();
                    break;
                case kEOF:
                    continue;
                case kName: {
                    Instruction inst = gen_inst(tokens, 1, tokens.size());
                    insts.push_back(inst);
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


}; /* namespace Script */
