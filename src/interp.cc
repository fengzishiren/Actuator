/*
 * engine.cc
 *
 *      Author: Lunatic
 */
#include "interp.h"
#include "tool.h"


namespace Script {
    static const std::string TAG = "interp";
    typedef Environment Env;

    static inline Value *eval(Env &env, Value *val) {
        if (val->type == kVAR) { //处理变量
            Value *v = env.get(val->repr());
            assert(v != nullptr, "变量未定义!", val->pos);
            return v;
        }
        return val;
    }


    Engine::Engine() : env(new Environment()) {

    }

    Engine::~Engine() {

    }

    void Engine::parse(const std::string &text) {
        Lexer lexer(text);
        Parser parser(lexer, insts, labels);
        parser.parse();
    }

    static void do_say(Env &env, Instruction &pc) {
        assert(pc.params.size() > 0, "syntax error!", pc.pos);
        for (Value *arg : pc.params) {
            Value *v = eval(env, arg);
            std::cerr << v->str();
        }
        std::cerr << std::endl;
    }

    static void do_set(Env &env, Instruction &pc) {
        assert(pc.params.size() == 2 && pc.params[0]->type == kVAR, "syntax error!", pc.pos);
        env.set(pc.params[0]->repr(), eval(env, pc.params[1]));
    }

    static void do_read(Env &env, Instruction &pc) {
        assert(pc.params.size() == 1 && pc.params[0]->type == kVAR, "syntax error!",
                pc.pos);
        std::string temp;
        std::getline(std::cin, temp);
        env.set(pc.params[0]->repr(), new StrValue(temp, pc.pos));
    }

    Value *Engine::execute(Env &env, size_t start, size_t end) {
        Value *val = Value::NIL;
        for (size_t i = start; i <= end; ++i) {
            Instruction &pc = insts[i];
            Log::debug(TAG, "Executing inst: %s", pc.repr().c_str());
            switch (pc.opcode) {
                case GOTO:
                    i = (size_t) (((IntValue *) pc.params[0])->get_val()) - 1;
                    break;
                case EXIT:
                    throw 0;
                case SAY:
                    do_say(env, pc);
                    break;
                case SET:
                    do_set(env, pc);
                    break;
                case READ:
                    do_read(env, pc);
                    break;
                case EQ:
                    if (!(*eval(env, pc.params[0]) == *eval(env, pc.params[1]))) {
                        i = labels[i] - 1;
                    }
                    break;
                case NE:
                    if (*eval(env, pc.params[0]) == *eval(env, pc.params[1])) {
                        i = labels[i] - 1;
                    }
                    break;
                    /*   case LE:
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
                           break;*/
                case CALL: {
                    Log::debug(TAG, "env: " + env.repr());
                    Env *e = new Environment(env);
                    const Closure *closure = e->find_closure(pc.params[0]->repr());
                    Log::debug(TAG, closure->repr());
                    assert(pc.params.size() > 1 && pc.params.size() - 1 == closure->args.size(), "函数调用参数不匹配", pc.pos);
                    for (size_t i = 0; i < closure->args.size(); ++i) {
                        e->set(closure->args[i], pc.params[i + 1]);
                    }
                    execute(*e, closure->start, closure->end);
                    break;
                };
                case RET:
                    val = eval(env, pc.params[0]);
                    return val;
                case ERR:
                    error(pc.params[0]->repr(), pc.pos);
                default:
                    std::cerr << pc.opcode << std::endl;
                    //error(format("无法识别的指令:%s", pc.repr().c_str()), pc.pos);
            }
        }
        return val;
    }

    static void finalize() {
        GC::gc_vals();
        GC::gc_envs();
    }

    int Engine::launch() {
        Log::debug(TAG, "will be exe inst size:%d", insts.size());
        Log::info(TAG, "will be exe inst lists:");
        std::cerr << join(insts, '\n') << std::endl;
        try {
            execute(*env, 0, insts.size() - 1);
        } catch (int exit) {
            return exit;
        }
        finalize();
        return 0;
    }

}

