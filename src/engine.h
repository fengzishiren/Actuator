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

#define EXIT "exit"
#define GOTO "goto"
#define CALL "goto"
#define SAY "say"
#define SET "set"
#define PRINT "print"
#define READ "read"
#define IF "if"
#define DECL "def"

namespace Script {


    class Environment {
        std::unordered_map<std::string, std::string> symbols;
        Environment *outer;
    public:
        Environment() : outer(nullptr) {
        }

        Environment(Environment *_outer) : outer(_outer) {
        }

        bool contains(const std::string &name) {
            auto *syms = this;
            while (syms and syms->symbols.find(name) == syms->symbols.end()) {
                syms = this->outer;
            }
            return syms != nullptr;
        }

        std::string get(const std::string &name) {
            auto *syms = this;
            auto pos;
            while (syms and (pos = syms->symbols.find(name)) == syms->symbols.end()) {
                syms = this->outer;
            }
            return syms == nullptr ? "" : pos->second;
        }

        void set_var(const std::string &name, const std::string &value) {
            symbols[name] = value;
        }
    };


    class Engine {
    private:
        typedef void (*func)(Env &, Instruction &);

        size_t idx; //指令指针
        //待执行指令集
        std::vector<Instruction> insts;
        std::unordered_map<std::string, Closure> closures;
        std::unordered_map<std::string, func> cmds;

        std::vector<Environment *> gc;
    public:
        Environment *new_env(Environment *env) {
            Environment *e = new Environment(*env);
            gc.push_back(e);
            return e;
        }

        Environment *new_env() {
            return new_env(nullptr);
        }

        void finalize() {
            for (auto *e : gc)
                delete e;
        }

        Engine();

        void parse(const std::string &text);

        void launch(Environment &env);

        func get_cmd(const std::string &name) {
            std::map<std::string, func>::iterator it = cmds.find(name);
            return it == cmds.end() ? NULL : it->second;
        }

    };

}  // namespace Script

#endif /* ENV_H_ */
