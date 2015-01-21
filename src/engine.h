/*
 * env.h
 *
 *  Created on: 2014年8月4日
 *      Author: epay111
 */

#ifndef ENV_H_
#define ENV_H_

#include <map>
#include <unordered_map>
#include <vector>
#include <cstddef>

#include "parser.h"


namespace Script {


    class Environment {
        std::unordered_map<std::string, std::string> symbols;
        Environment *outer;

    public:
        static std::vector<Environment *> *gc;
        static const std::unordered_map<std::string, Closure> *closures;

        Environment() : outer(nullptr) {
        }

        Environment(Environment *_outer) : outer(_outer) {
        }

        static Environment *new_env(Environment &outer);

        static const Closure *find_closure(const std::string &fun_name);

        bool contains(const std::string &name) {
            auto *syms = this;
            while (syms and syms->symbols.find(name) == syms->symbols.end()) {
                syms = this->outer;
            }
            return syms != nullptr;
        }

        std::string get(const std::string &name) {
            auto *syms = this;
            auto pos = syms->symbols.find(name);
            while (syms && pos == syms->symbols.end()) {
                syms = this->outer;
                pos = syms->symbols.find(name);
            }
            return syms == nullptr ? "" : pos->second;
        }


        void set(const std::string &name, const std::string &value) {
            symbols[name] = value;
        }
    };


    class Engine {
    private:
        size_t idx; //指令指针
        //待执行指令集
        std::vector<Instruction> insts;
        std::unordered_map<std::string, Closure> closures;

        std::vector<Environment *> gc;
        Environment *env;
    public:
        Engine();

        ~Engine();


        void parse(const std::string &text);

        int launch();
    };

}  // namespace Script

#endif /* ENV_H_ */
