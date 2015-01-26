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
/*
    static void do_read(Env &env, Instruction &pc) {
        assert(pc.params.size() == 1 && pc.params[0]->type == kVAR, "syntax error!",
                pc.pos);
        std::string temp;
        std::getline(std::cin, temp);
        env.set(pc.params[0]->repr(), new StrValue(temp, pc.pos));
    }*/

    /**
    * exclude: == and !=
    */
    static inline void CMP(Environment &env, Instruction &pc, std::unordered_map<size_t, size_t> &labels, int op, size_t &i) {
        Value *left = eval(env, pc.params[0]);
        Value *right = eval(env, pc.params[1]);
        assert(left->type == right->type &&
                (left->type == kINT || left->type == kFLOAT), "type mismatch", pc.pos);
        //Log::debug(TAG, format("left:%s right: %s", left->str().c_str(), right->str().c_str()));
        switch (op) {
            case 0:
                if (!(*((PrimValue *) left) >= *right))
                    i = labels[i] - 1;
                break;
            case 1:
                if (!(*((PrimValue *) left) <= *right))
                    i = labels[i] - 1;
                break;
            case 2:
                if (!((PrimValue *) left)->operator>(*right))
                    i = labels[i] - 1;
                break;
            case 3:
                if (!(*((PrimValue *) left) < *right))
                    i = labels[i] - 1;
                break;
        }

    };

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
                case ADD: ARITH(env, pc, +)
                case SUB: ARITH(env, pc, -)
                case MUL: ARITH(env, pc, *)
                case DIV: ARITH(env, pc, /)
//                case READ:
//                    do_read(env, pc);
//                    break;
                case EQ:
                    if (!(*eval(env, pc.params[0]) == *eval(env, pc.params[1]))) {
                        i = labels[i] - 1;
                    }
                    break;
                case NE:
                    if ((*eval(env, pc.params[0]) == *eval(env, pc.params[1]))) {
                        i = labels[i] - 1;
                    }
                    break;
                case GE:
                    CMP(env, pc, labels, 0, i);
                    break;
                case LE:
                    CMP(env, pc, labels, 1, i);
                    break;
                case GT:
                    CMP(env, pc, labels, 2, i);
                    break;
                case LS:
                    CMP(env, pc, labels, 3, i);
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
        try {
            execute(*env, 0, insts.size() - 1);
        } catch (int exit) {
            return exit;
        }
        return 0;
    }

}

