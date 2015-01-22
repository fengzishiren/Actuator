/*
 * engine.cc
 *
 *      Author: Lunatic
 */
#include "engine.h"
#include "alarm.h"


namespace Script {

    const std::unordered_map<std::string, Closure> *Environment::closures = nullptr;

    const Closure *Environment::find_closure(const std::string &fun_name) {
        std::unordered_map<std::string, Closure>::const_iterator it;
        return (it = Environment::closures->find(fun_name)) != Environment::closures->end() ? &it->second : nullptr;
    }

    typedef Environment Env;

    static inline const Value &eval(Env &env, Value &val) {
        if (val.type == kVAR) { //处理变量
            const Value &v = env.get(val.repr());
            assert(v == Value::NIL, "变量未定义!", val.pos);
            return v;
        }
        return val;
    }


    Engine::Engine() : env(new Environment()) {
        Environment::closures = &this->closures;
        gc.push_back(env);
    }

    Engine::~Engine() {
        for (auto *e : gc)
            delete e;
    }

    void Engine::parse(const std::string &text) {
        Lexer lexer(text);
        Parser parser(lexer, insts, labels, closures);
        parser.parse();
    }

    static void do_say(Env &env, Instruction &pc) {
        assert(pc.params.size() > 0, "语法错误!", pc.pos);

        for (Value &arg : pc.params) {
            const Value &v = eval(env, arg);
            std::cout << v.repr();
        }
        std::cout << std::endl;
    }

    static void do_set(Env &env, Instruction &pc) {
        assert(pc.params.size() == 2 && pc.params[0].type == kVAR, "语法错误", pc.pos);
        env.set(pc.params[0].repr(), eval(env, pc.params[1]));
    }

    static void do_read(Env &env, Instruction &pc) {
        assert(pc.params.size() == 1 && pc.params[0].type == kVAR, "语法错误",
                pc.pos);
        std::string temp;
        std::getline(std::cin, temp);
        env.set(pc.params[0].repr(), Value(temp, pc.pos));
    }

    Value Engine::execute(Env &env, size_t start, size_t end) {
        Value val(Value::NIL);
        for (size_t i = start; i <= end; ++i) {
            Instruction &pc = insts[i];
            switch (pc.opcode) {
                case EXIT:
                    throw 0;
                case SAY:
                    do_say(env, pc);
                case SET:
                    do_set(env, pc);
                case READ:
                    do_read(env, pc);
                case EQ:
                    if (!(eval(env, pc.params[0]) == (eval(env, pc.params[1])))) {
                        i = labels[i] - 1;
                    }
                    break;
                case NE:
                    if (eval(env, pc.params[0]) == (eval(env, pc.params[1]))) {
                        i = labels[i] - 1;
                    }
                    break;
                case LE:
                    if (eval(env, pc.params[0]).greater(eval(env, pc.params[1])))
                        i = labels[i] - 1;
                    break;
                case GE:
                    if (eval(env, pc.params[0]).less(eval(env, pc.params[1])))
                        i = labels[i] - 1;;
                    break;
                case GT:
                    eval(env, pc.params[0]).less_equals(eval(env, pc.params[1]));

                    break;
                case LS:
                    eval(env, pc.params[0]).greater_equals(eval(env, pc.params[1]));
                    break;
                case CALL: {
                    Env *e = new_env(env);
                    const Closure *closure = Env::find_closure(pc.params[0].repr());
                    assert(pc.params.size() > 1 && pc.params.size() - 2 == closure->args.size(), "函数调用参数不匹配", pc.pos);
                    for (size_t i = 0; i < closure->args.size(); ++i) {
                        e->set(closure->args[i], pc.params[i + 2]);
                    }
                    execute(*e, closure->start, closure->end);
                };
                case RET:
                    val = eval(env, pc.params[0]);
                    return val;
                case ERR:
                    error(pc.params[0].repr(), pc.pos);
                default:
                    error("无法识别的指令", pc.pos);
            }
        }
        return val;
    }

    int Engine::launch() {
        try {
            execute(*env, 0, insts.size());
        } catch (int exit) {
            return exit;
        }
        return 0;
    }

}

