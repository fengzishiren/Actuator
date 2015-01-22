/*
 * engine.cc
 *
 *      Author: Lunatic
 */
#include "engine.h"
#include "alarm.h"


namespace Script {

    std::vector<Environment *> *Environment::gc = nullptr;
    const std::unordered_map<std::string, Closure> *Environment::closures = nullptr;

    Environment *Environment::new_env(Environment &outer) {
        Environment *e = new Environment(outer);
        Environment::gc->push_back(e);
        return e;
    }

    const Closure *Environment::find_closure(const std::string &fun_name) {
        std::unordered_map<std::string, Closure>::const_iterator it;
        return (it = Environment::closures->find(fun_name)) != Environment::closures->end() ? &it->second : nullptr;
    }

    typedef Environment Env;



    static long long str2int(const std::string &val) {
        //char *err = NULL;
        //long long retval = std::strtoll(val.c_str(), &err, 10);
        //if (*err == '\0')
        //return retval;
        return std::strtoll(val.c_str(), NULL, 10);
    }

    static std::string get_val(Env &env, Token &token) {
        if (token.type == kName) { //处理变量
            assert(env.contains(token.content), "变量未定义!", token.pos);
            return env.get(token.content);
        }
        return token.content;
    }



    //	static void add_cmd(Env& env) {
    //		return x + y;
    //	}

//    static void goto_cmd(Env &env, Instruction &pc) {
//        assert(pc.params.size() == 1 && pc.params[0].type == kName, "语法错误",
//                pc.pos);
//        Log::debug(pc.params[0].content);
//        int ret = env.get_goto_Idx(pc.params[0].content);
//        assert(ret != -1, "找不到跳转位置!", pc.pos);
//        env.set_idx((size_t) ret);
//    }
#if 0


    static void do_if(Env &env, Instruction &pc) {
        //if value1 [opcode value2] goto label
        bool jmp = false;
        assert(pc.params.size() == 5 && pc.params[1].type == KCmp, "goto语法错误!",
                pc.pos); //if val goto label
        std::string left = get_val(env, pc.params[0]);
        std::string right = get_val(env, pc.params[2]);

        if (pc.params[1].content == "==") {
            jmp = left == right;
        } else if (pc.params[1].content == "!=") {
            jmp = left != right;
        } else if (pc.params[1].content == "<") {
            jmp = str2int(left) < str2int(right);
        } else if (pc.params[1].content == ">") {
            jmp = str2int(left) > str2int(right);
        } else if (pc.params[1].content == "<=") {
            jmp = str2int(left) <= str2int(right);
        } else if (pc.params[1].content == ">=") {
            jmp = str2int(left) < str2int(right);
        }

        if (jmp && pc.params[3].type == kName && pc.params[3].content == "goto"
                && pc.params[4].type == kName) {
            Token copy = pc.params[4];
            pc.pos = pc.params[3].pos;
            pc.params.clear();
            pc.params.push_back(copy);
            goto_cmd(env, pc);
        } else if (jmp) {
            error("goto语句后面必须带有一个跳转Label", pc.params[4].pos);
        }
    }

    static void shell_cmd(Env &env, Instruction &pc) {
        assert(pc.params.size() > 0, "语法错误!", pc.pos);
        std::string command;
        std::vector<Token>::iterator it = pc.params.begin();
        for (; it != pc.params.end(); it++) {
            Log::debug("val name: %s", it->content.c_str());
            command.append(get_val(env, *it));
            command.append(" ");
        }
        command = command.substr(0, command.size() - 1);

        Log::debug("Shell： %s", command.c_str());
        std::system(command.c_str());
    }

#endif

    Engine::Engine() : env(new Environment()) {
        Environment::gc = &this->gc;
        Environment::closures = &this->closures;
        gc.push_back(env);

    }

    Engine::~Engine() {
        for (auto *e : gc)
            delete e;
    }

    void Engine::parse(const std::string &text) {
        Lexer lexer(text);
        //TODO
        std::unordered_map<size_t, size_t> _labels;
        Parser parser(lexer,insts,_labels, closures);
        parser.parse();
    }

    static void do_say(Env &env, Instruction &pc) {
        assert(pc.params.size() > 0, "语法错误!", pc.pos);
        for (std::vector<Token>::iterator it = pc.params.begin();
             it != pc.params.end(); ++it) {
            if (it->type == kName) {
                assert(env.contains(it->content), "变量未定义!", it->pos);
                std::cout << env.get(it->content) << ' '; //sep by ' '
            } else
                std::cout << it->content;
        }
        std::cout << std::endl;
    }

    static void do_set(Env &env, Instruction &pc) {
        assert(pc.params.size() == 2 && pc.params[0].type == kName, "语法错误", pc.pos);
        env.set(pc.params[0].content, get_val(env, pc.params[1]));
    }

    static void do_read(Env &env, Instruction &pc) {
        assert(pc.params.size() == 1 && pc.params[0].type == kName, "语法错误",
                pc.pos);
        std::string temp;
        std::getline(std::cin, temp);
        env.set(pc.params[0].content, temp);
    }

    static void execute(std::vector<Instruction> &insts, Env &env, size_t start, size_t end) {
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
                case CALL: {
                    Env *e = Env::new_env(env);
                    const Closure *closure = Env::find_closure(pc.params[0].content);
                    assert(pc.params.size() > 1 && pc.params.size() - 2 == closure->args.size(), "函数调用参数不匹配", pc.pos);
                    for (size_t i = 0; i < closure->args.size(); ++i) {
                        e->set(closure->args[i], pc.params[i + 2].content);
                    }
                    execute(insts, *e, closure->start, closure->end);
                };
                case ERR:
                    error(pc.params[0].content, pc.pos);
                default:
                    error("无法识别的指令", pc.pos);
            }
        }
    }

    int Engine::launch() {
        try {
            execute(insts, *env, 0, insts.size());
        } catch (int exit) {
            return exit;
        }
        return 0;
    }

}

