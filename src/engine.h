/*
 * env.h
 *
 *  Created on: 2014年8月4日
 *      Author: epay111
 */

#ifndef ENV_H_
#define ENV_H_

#include "parser.h"
#include <cstddef>
#include <unordered_map>


namespace Script {


    class Environment {
        std::unordered_map<std::string, Value> symbols;
        Environment *outer;

    public:
        static const std::unordered_map<std::string, Closure> *closures;

        Environment() : outer(nullptr) {
        }

        Environment(Environment *_outer) : outer(_outer) {
        }


        static const Closure *find_closure(const std::string &fun_name);

        bool contains(const std::string &name) {
            auto *syms = this;
            while (syms and syms->symbols.find(name) == syms->symbols.end()) {
                syms = this->outer;
            }
            return syms != nullptr;
        }

        const Value &get(const std::string &name) {
            Environment *syms = this;
            auto pos = syms->symbols.find(name);
            while (syms != nullptr && pos == syms->symbols.end()) {
                syms = this->outer;
                pos = syms->symbols.find(name);
            }
            return syms == nullptr ? Value::NIL : pos->second;
        }


        void set(const std::string &name, const Value &value) {
            symbols[name] = value;
        }
    };


    class Engine {
    private:
        size_t idx; //指令指针
        //待执行指令集
        std::vector<Instruction> insts;
        std::unordered_map<std::string, Closure> closures;
        std::unordered_map<size_t, size_t> labels;
        std::vector<Environment *> gc;
        Environment *env;

        Environment *new_env(Environment &outer) {
            Environment *e = new Environment(&outer);
            gc.push_back(e);
            return e;
        }

    public:
        Engine();

        ~Engine();


        void parse(const std::string &text);

        Value execute(Environment &env, size_t start, size_t end);

        int launch();
    };

}  // namespace Script

#endif /* ENV_H_ */
