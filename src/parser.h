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


    enum Type {
        kVAR, kSTRING, kINT, kFLOAT, kNULL, kClosure
    };

    class Environment;

    class Value;

    class Closure;


    namespace GC {
        extern std::vector<Environment *> env_gc;
        extern std::vector<Value *> val_gc;

        void gc_vals();

        void gc_envs();
    }

    class Environment : public Visualable {
    private:
        std::unordered_map<std::string, Value *> symbols;
        Environment *outer;
    public:

        Environment() : outer(nullptr) {
            GC::env_gc.push_back(this);
        }

        Environment(Environment *_outer) : outer(_outer) {
            GC::env_gc.push_back(this);

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
        }

        Value(Type _type, const Position _pos) : type(_type), pos(_pos) {
        }

        virtual ~Value() {
        }

        virtual bool operator==(const Value &v) const = 0;

        virtual std::string str() const {
            return repr();
        }

        virtual std::string repr() const = 0;

    };


    class IntValue : public Value {
        INT val;
    public:
        IntValue(INT _val, const Position &_pos) : val(_val), Value(kINT, _pos) {
            GC::val_gc.push_back(this);
        }

        bool operator==(const Value &v) const {
            return v.type == type && v.repr() == repr();
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

    class FloatValue : public Value {
        FLOAT val;
    public:
        FloatValue(FLOAT _val, const Position &_pos) : val(_val), Value(kFLOAT, _pos) {
            GC::val_gc.push_back(this);
        }

        bool operator==(const Value &v) const {
            return v.type == type && v.repr() == repr();
        }

        std::string repr() const {
            std::stringstream ss;
            ss << val;
            return ss.str();
        }
    };

    class StrValue : public Value {
        std::string val;
    public:
        StrValue(std::string _val, const Position &_pos) : val(_val), Value(kSTRING, _pos) {
            GC::val_gc.push_back(this);
        }

        bool operator==(const Value &v) const {
            return v.type == type && v.repr() == repr();
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
            GC::val_gc.push_back(this);
        }


        bool operator==(const Value &v) const {
            return v.type == type && v.repr() == repr();
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
            GC::val_gc.push_back(this);
        }

        bool operator==(const Value &v) const {
            return v.type == type && v.repr() == repr();
        }

        std::string repr() const {
            return "<Fun: " + name + "(" + join(args, ',') + ")>";
        }
    };

    class NullValue : public Value {
    public:
        NullValue() : Value(kNULL, Position::NULL_POS) {
            GC::val_gc.push_back(this);
        }

        bool operator==(const Value &v) const {
            return v.type == kNULL;
        }

        std::string repr() const {
            return "NULL";
        }
    };

/*  class Value {
      union {
          char *s;
          INT num;
          FLOAT real;
      } val;
  public:
      Position pos;
      Type type;
      static Value NIL;

      Value();

      Value(const std::string &str, const Position &_pos);

      Value(INT n, const Position &_pos);

      Value(FLOAT n, const Position &_pos);

      Value(Type _tag, const Position &_pos);

      Value(const Position &_pos);

      Value(const Value &value);

      ~Value();

      void set_str(const char *str, Type _tag);

      void set_string(const std::string &str, Type _tag = kSTRING);

      void set_int(INT n);

      void set_float(FLOAT real);

      bool operator==(const Value &v) const;

      bool less_equals(const Value &v) const;

      bool greater_equals(const Value &v) const;

      bool less(const Value &v) const;

      bool greater(const Value &v) const;

      bool equals(const Value &v) const;

      std::string repr() const;
  };*/

    class Instruction {
    public:
        int opcode;
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
        std::unordered_map<size_t, size_t> &labels;
        std::vector<Token> tokens;
        //  std::unordered_map<std::string, Closure> &closures;

    public:
        Parser(Lexer &_lexer, std::vector<Instruction> &_insts, std::unordered_map<size_t, size_t> &labels);

        ~Parser();

        Instruction &gen_inst();

        void define();

        void match(Tag t);

        void embed_stmts();

        void stmts();

        void move();

        void parse();
    };


} /* namespace Script */

#endif /* PARSER_H_ */
