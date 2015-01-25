/**
*/
#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include "parser.h"
#include <cstddef>
#include <unordered_map>


namespace Script {


    class Engine {
    private:
        size_t idx; //指令指针
        //待执行指令集
        std::vector<Instruction> insts;
        //  std::unordered_map<std::string, Closure> closures;
        std::unordered_map<size_t, size_t> labels;
        //   std::vector<Environment *> gc;
        Environment *env;


    public:
        Engine();

        ~Engine();


        void parse(const std::string &text);

        Value *execute(Environment &env, size_t start, size_t end);

        int launch();
    };

}  // namespace Script

#endif /* ENV_H_ */
