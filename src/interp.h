/*
 * interp.h
 *
 *      Author: fengzishiren
 */
#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include "parser.h"
#include <cstddef>
#include <unordered_map>


namespace Script {


    class Engine {
    private:
        std::vector<Instruction> insts;
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
