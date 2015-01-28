/*
 * Parser.h
 *
 *  Created on: 2014年6月6日
 *      Author: lunatic
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <vector>
#include <sstream>
#include <unordered_map>
#include "lexer.h"
#include "tool.h"

//Note: forbidden define 0
/*
#define EXIT 1
#define JMP 2
#define CALL 3
#define SAY 4
#define SET 5
#define EQ 6
#define  NE 7
#define  LE 8
#define  GE 9
#define  GT 10
#define  LS 11
#define RET 12
#define ADD 13
#define SUB 14
#define MUL 15
#define DIV 16
#define ERR 17
*/


namespace Script {

    enum INST {
        EXIT, JMP, CALL, SAY, SET, CMP, JEQ, JNE, JLE, JGE, JGT, JLS, RET, ADD, SUB, MUL, DIV, ERR
    };

    typedef long long INT;
    typedef long double FLOAT;


    enum Type {
        kVAR, kSTRING, kINT, kFLOAT, kNULL, kClosure
    };

    class Environment;

    class Value;

    class Closure;


    namespace GC {
        extern std::vector<Value *> val_gc;

        void gc_vals();

    }

    class Environment : public Visualable {
    private:
        std::unordered_map<std::string, Value *> symbols;
        Environment *outer;
        //TODO
        //set<Value*> gc
    public:

        Environment() : outer(nullptr) {
        }

        Environment(Environment *_outer) : outer(_outer) {
        }

        Closure *find_closure(const std::string &fun_name);

        bool contains(const std::string &name);

        Value *get(const std::string &name);

        std::string str() const {
            return repr();
        }

        std::string repr() const;

        void set(const std::string &name, Value *value);
    };

    class Value : public Visualable {
    public:
        Type type;
        Position pos;
        static Value *NIL;

        Value() : type(kNULL), pos(Position::NULL_POS) {
            GC::val_gc.push_back(this);
        }

        Value(Type _type, const Position _pos) : type(_type), pos(_pos) {
            GC::val_gc.push_back(this);
        }

        virtual ~Value() {
        }

        virtual bool operator==(const Value *v) const = 0;

        virtual std::string str() const {
            return repr();
        }

        virtual std::string repr() const = 0;

    };


    class PrimValue : public Value {
    public:
        PrimValue() : Value() {
        }

        PrimValue(Type _type, const Position _pos) : Value(_type, _pos) {
        }

        virtual bool operator==(const Value *v) const {
            return false;
        }

        virtual bool operator!=(const Value *v) const {
            return false;
        }

        virtual bool operator>=(const Value *v) const {
            return false;
        }

        virtual bool operator<=(const Value *v) const {
            return false;
        }

        virtual bool operator>(const Value *v) const {
            return false;
        }

        virtual bool operator<(const Value &v) const {
            return false;
        }

    };

    class IntValue : public PrimValue {
        INT val;
    public:
        IntValue() : PrimValue() {
        }

        IntValue(INT _val, const Position &_pos) : val(_val), PrimValue(kINT, _pos) {

        }

        bool operator==(const Value *v) const {
            return v->type == type && val == ((IntValue *) v)->val;
        }

        bool operator!=(const Value *v) const {
            return v->type != type || val != ((IntValue *) v)->val;
        }

        bool operator>=(const Value *v) const {
            return v->type == type && val >= ((IntValue *) v)->val;
        }

        bool operator<=(const Value *v) const {
            return v->type == type && val <= ((IntValue *) v)->val;
        }

        bool operator>(const Value *v) const {
            return v->type == type && val > ((IntValue *) v)->val;
        }

        bool operator<(const Value *v) const {
            return v->type == type && val < ((IntValue *) v)->val;
        }

        void set_val(INT _val) {
            val = _val;
        }

        INT get_val() {
            return val;
        }

        std::string repr() const {
            std::stringstream ss;
            ss << val;
            return ss.str();
        }

    };

    class FloatValue : public PrimValue {
        FLOAT val;
    public:
        FloatValue(FLOAT _val, const Position &_pos) : val(_val), PrimValue(kFLOAT, _pos) {
        }

        bool operator==(const Value *v) const {
            return v->type == type && v->repr() == repr();
        }

        void set_val(FLOAT _val) {
            val = _val;
        }

        FLOAT get_val() {
            return val;
        }

        std::string repr() const {
            std::stringstream ss;
            ss << val;
            return ss.str();
        }
    };

    class StrValue : public PrimValue {
        std::string val;
    public:
        StrValue(const std::string &_val, const Position &_pos) : val(_val), PrimValue(kSTRING, _pos) {

        }

        bool operator==(const Value *v) const {
            return v->type == type && v->str() == str();
        }

        std::string repr() const {
            return "\"" + val + "\"";
        }

        std::string str() const {
            return val;
        }
    };


    class VarValue : public Value {
        std::string val;
    public:
        VarValue(std::string _val, const Position &_pos) : val(_val), Value(kVAR, _pos) {

        }

        bool operator==(const Value *v) const {
            return v->type == type && v->str() == str();
        }

        std::string repr() const {
            return val;
        }
    };

    class Closure : public Value {
    public:
        std::vector<std::string> args;
        std::string name;
        size_t start;
        size_t end;

        Closure(const Position &_pos) : Closure("lambda", 0, 0, _pos) {
        }

        Closure(const std::string &_name, size_t _start, size_t _end, const Position &_pos)
                : name(_name), start(_start), end(_end), Value(kClosure, _pos) {

        }

        bool operator==(const Value *v) const {
            return v->type == type && v->str() == str();
        }

        std::string repr() const {
            return "<Fun: " + name + "(" + join(args, ',') + ")>";
        }
    };

    class NullValue : public Value {
    public:
        NullValue() : Value(kNULL, Position::NULL_POS) {

        }

        bool operator==(const Value *v) const {
            return v->type == kNULL;
        }

        std::string repr() const {
            return "NULL";
        }
    };

    class Instruction {
    public:
        INST opcode;
        std::vector<Value *> params;
        Position pos;

        Instruction() {
        }

        ~Instruction() {
        }

        std::string repr() const;

    };

    class Parser {
    private:
        Lexer &lexer;
        std::vector<Instruction> &insts;
        std::vector<Token> tokens;

    public:
        Parser(Lexer &_lexer, std::vector<Instruction> &_insts);

        ~Parser();

        void define();

        void match(Tag t);

        void loop();

        void caj(IntValue *end);

        void cond();

        void cond_stmts();

        void fun_stmts();

        void stmts();

        void move();

        void parse();

    };


} /* namespace Script */

#endif /* PARSER_H_ */
