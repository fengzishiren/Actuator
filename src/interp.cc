/*
 * engine.cc
 *
 *      Author: Lunatic
 */
#include "interp.h"
#include "tool.h"

#define EQ_ON 1<<0
#define NE_ON 1<<1
#define GE_ON 1<<2
#define LE_ON 1<<3
#define GT_ON 1<<4
#define LS_ON 1<<5

namespace Script {
    static const std::string TAG = "interp";
    typedef Environment Env;

    static inline Value *eval(Env &env, Value *val) {
        if (val->type == kVAR) { //处理变量
            Value *v = env.get(val->repr());
            assert(v != nullptr, format("var not define: %s!", val->repr().c_str()), val->pos);
            return v;
        }
        return val;
    }


    Engine::Engine() : env(new Environment()) {

    }

    Engine::~Engine() {
        GC::gc_vals();
        delete env;
    }

    void Engine::parse(const std::string &text) {
        Lexer lexer(text);
        Parser parser(lexer, insts);
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

#define ARITH(env, pc, op) \
    {\
        std::string name = pc.params[0]->repr();\
        Value *var = env.get(name);\
        assert(var != nullptr, format("var not define: %s", name.c_str()), pc.pos);\
        Value *val = eval(env, pc.params[1]);\
        assert((var->type == kINT || var->type == kFLOAT)\
                && (val->type == kINT || val->type == kFLOAT), "not number type", pc.pos);\
        Value *result;\
        if (var->type == kINT) {\
            if (val->type == kINT)\
                result = new IntValue(((IntValue *) var)->get_val() op ((IntValue *) val)->get_val(), pc.pos);\
            else\
                result = new FloatValue(((IntValue *) var)->get_val() op ((FloatValue *) val)->get_val(), pc.pos);\
        } else {\
            assert(((IntValue *) val)->get_val() == 0, "div 0 error!", pc.pos);\
            if (val->type == kINT)\
                result = new FloatValue(((FloatValue *) var)->get_val() op ((IntValue *) val)->get_val(), pc.pos);\
            else\
                result = new FloatValue(((FloatValue *) var)->get_val() op ((FloatValue *) val)->get_val(), pc.pos);\
        }\
        env.set(name, result);\
        break;\
    };

    Value *Engine::execute(Env &env, size_t start, size_t end) {
        int flag = 0;
        Value *val = Value::NIL;
        for (size_t i = start; i <= end; ++i) {
            Instruction &pc = insts[i];
            Log::debug(TAG, "Executing inst: %s", pc.repr().c_str());
            switch (pc.opcode) {
                case JMP:
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
                case ADD: ARITH(env, pc, +)
                case SUB: ARITH(env, pc, -)
                case MUL: ARITH(env, pc, *)
                case DIV: ARITH(env, pc, /)
                case CMP: {
                    Value *left = eval(env, pc.params[0]);
                    Value *right = eval(env, pc.params[1]);
                    assert(left->type == right->type &&
                            (left->type == kINT || left->type == kFLOAT || left->type == kSTRING), "type mismatch", pc.pos);
                    Log::debug(TAG, format("left:%s right: %s", left->str().c_str(), right->str().c_str()));
                    if (left->type == kINT) {
                        if (((IntValue *) left)->operator==((IntValue *) right))
                            flag = EQ_ON | GE_ON | LE_ON;
                        else if (((IntValue *) left)->operator>((IntValue *) right))
                            flag = GT_ON | GE_ON;
                        else
                            flag = LS_ON | LE_ON;
                    } else if (left->type == kFLOAT) {
                        if (((FloatValue *) left)->operator==((FloatValue *) right))
                            flag = EQ_ON | GE_ON | LE_ON;
                        else if (((FloatValue *) left)->operator>((FloatValue *) right))
                            flag = GT_ON | GE_ON;
                        else
                            flag = LS_ON | LE_ON;
                    } else {
                        if (((StrValue *) left)->operator==((StrValue *) right))
                            flag |= EQ_ON;
                    }
                    break;
                };
                case JEQ:
                    if (flag & EQ_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        flag = 0;
                        break;
                    }

                case JNE:
                    if (flag & EQ_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        flag = 0;
                        break;
                    }

                case JGE:
                    if (flag & GE_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        flag = 0;
                    }
                    break;
                case JLE:
                    if (flag >> 3 & 1) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        flag = 0;
                    }
                    break;
                case JGT:
                    if (flag & GT_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        flag = 0;
                    }
                case JLS:
                    if (flag & LS_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        flag = 0;
                    }
                    break;
                case CALL: {
                    Log::debug(TAG, "env: " + env.repr());
                    Env *e = new Environment(env);
                    const Closure *closure = e->find_closure(pc.params[0]->repr());
                    Log::debug(TAG, closure->repr());
                    assert(pc.params.size() > 1 && pc.params.size() - 1 == closure->args.size(), "函数调用参数不匹配", pc.pos);
                    for (size_t i = 0; i < closure->args.size(); ++i) {
                        e->set(closure->args[i], eval(env, pc.params[i + 1]));
                    }
                    execute(*e, closure->start, closure->end);
                    delete e;
                    break;
                };
                case RET:
                    val = eval(env, pc.params[0]);
                    return val;
                case ERR:
                    error(pc.params[0]->repr(), pc.pos);
                default:
                    error(format("unrecognize :%s", pc.repr().c_str()), pc.pos);
            }
        }

        return val;
    }


    int Engine::launch() {
        Log::debug(TAG, "will be exe inst size:%d", insts.size());
        Log::info(TAG, "will be exe inst lists:");
        std::cerr << join(insts, '\n') << std::endl;

        //throw 0;
        try {
            execute(*env, 0, insts.size() - 1);
        } catch (int exit) {
            return exit;
        }
        return 0;
    }

}

