/*
 * interp.cc
 *
 *      Author: fengzishiren
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
        //parser verify pc.params count
        //must must be not less than 1
        //assert(pc.params.size() > 0, "syntax error!", pc.pos);
        std::cerr << eval(env, pc.params[0])->str();
        for (size_t i = 1; i < pc.params.size(); ++i) {
            std::cerr << ' ' << eval(env, pc.params[i])->str();
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
        if (pc.opcode == DIV)\
            assert(((IntValue *) val)->get_val() != 0,  "div 0 error!", pc.pos);\
        if (var->type == kFLOAT || val->type == kFLOAT) {\
            result = new FloatValue(((IntValue *) var)->get_val() op ((FloatValue *) val)->get_val(), pc.pos);\
        } else {\
            result = new IntValue(((IntValue *) var)->get_val() op ((IntValue *) val)->get_val(), pc.pos);\
        }\
        env.set(name, result);\
        break;\
    };

    Value *Engine::execute(Env &env, size_t start, size_t end) {
        char flags[2048] = {0};
        int fp = -1;
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
                    fp++;
                    if (left->type == kINT) {
                        if (((IntValue *) left)->operator==((IntValue *) right))
                            flags[fp] = EQ_ON | GE_ON | LE_ON;
                        else if (((IntValue *) left)->operator>((IntValue *) right))
                            flags[fp] = GT_ON | GE_ON | NE_ON;
                        else
                            flags[fp] = LS_ON | LE_ON | NE_ON;
                    } else if (left->type == kFLOAT) {
                        if (((FloatValue *) left)->operator==((FloatValue *) right))
                            flags[fp] = EQ_ON | GE_ON | LE_ON;
                        else if (((FloatValue *) left)->operator>((FloatValue *) right))
                            flags[fp] = GT_ON | GE_ON | NE_ON;
                        else
                            flags[fp] = LS_ON | LE_ON | NE_ON;
                    } else {
                        if (((StrValue *) left)->operator==((StrValue *) right))
                            flags[fp] |= EQ_ON;
                    }
                    break;
                };
                case JEQ:
                    if (flags[fp] & EQ_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        flags[fp] = 0;
                        fp--;

                    }
                    break;
                case JNE:
                    if (flags[fp] & NE_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        fp--;
                    }
                    break;
                case JGE:
                    if (flags[fp] & GE_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        fp--;
                    }
                    break;
                case JLE:
                    if (flags[fp] & LE_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        fp--;
                    }
                    break;
                case JGT:
                    if (flags[fp] & GT_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        fp--;
                    }
                case JLS:
                    if (flags[fp] & LS_ON) {
                        i = (size_t) ((IntValue *) eval(env, pc.params[0]))->get_val() - 1;
                        fp--;
                    }
                    break;
                case CALL: {
                    Log::debug(TAG, "env: " + env.repr());
                    Env *e = new Environment(env);
                    const Closure *closure = e->find_closure(pc.params[0]->repr());
                    Log::debug(TAG, closure->repr());
                    assert(pc.params.size() - 1 == closure->args.size(),
                            format("Function call parameters do not match! expect %zu, but given %zu",
                                    closure->args.size(), pc.params.size() - 1), pc.pos);
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
            if (insts.size() > 0)
                execute(*env, 0, insts.size() - 1);
        } catch (int exit) {
            return exit;
        }
        return 0;
    }

}

