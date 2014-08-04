/*
 * engine.cc
 *
 *      Author: Lunatic
 */
#include "engine.h"
#include "alarm.h"

#include <cstdlib>

namespace Script {

static void assert(bool cond, const std::string& msg, const Position& pos) {
	if (!cond)
		error(msg, pos);
}

long long str2int(const std::string& val) {
	//char *err = NULL;
	//long long retval = std::strtoll(val.c_str(), &err, 10);
	//if (*err == '\0')
	//return retval;

	//忽略错误
	return std::strtoll(val.c_str(), NULL, 10);
}
std::string get_val(Env& env, Token& token) {
	if (token.type == kName) { //处理变量
		assert(env.existVar(token.content), "变量未定义!", token.pos);
		return env.getVar(token.content);
	}
	return token.content;
}
struct Commands {

	static void exit_cmd(Env& env, Instruction& pc) {
		//std::exit(0);
		throw "Bye-bye";
	}

//	static void add_cmd(Env& env) {
//		return x + y;
//	}

	static void goto_cmd(Env& env, Instruction& pc) {
		assert(pc.params.size() == 1 && pc.params[0].type == kName, "语法错误",
				pc.pos);
		Log::debug(pc.params[0].content);
		int ret = env.getGotoIdx(pc.params[0].content);
		assert(ret != -1, "找不到跳转位置!", pc.pos);
		env.setIdx((size_t) ret);
	}

	static void say_cmd(Env& env, Instruction& pc) {
		assert(pc.params.size() > 0, "语法错误!", pc.pos);
		for (std::vector<Token>::iterator it = pc.params.begin();
				it != pc.params.end(); ++it) {
			if (it->type == kName) {
				assert(env.existVar(it->content), "变量未定义!", it->pos);
				std::cout << env.getVar(it->content) << ' '; //sep by ' '
			} else
				std::cout << it->content;
		}
		std::cout << std::endl;
	}

	static void read_cmd(Env& env, Instruction& pc) {
		assert(pc.params.size() == 1 && pc.params[0].type == kName, "语法错误",
				pc.pos);
		std::string temp;
		std::cin >> temp;
		env.setVar(pc.params[0].content, temp);
	}

	static void if_cmd(Env& env, Instruction& pc) {
		//if value1 [opcode value2] goto label
		bool jmp = false;
		if (pc.params.size() == 5 && pc.params[1].type == KCmp) { //if val goto label
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

			if (jmp && pc.params[3].type == kName
					&& pc.params[3].content == "goto"
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
	}

	static void shell_cmd(Env& env, Instruction& pc) {
		assert(pc.params.size() > 0, "语法错误!", pc.pos);
		Log::debug("shello");
		std::string command;
		std::vector<Token>::iterator it = pc.params.begin();
		for (; it != pc.params.end(); it++) {
			command.append(it->content);
			command.append(" ");
		}
		command = command.substr(0, command.size() - 1);

		Log::debug("Shell： %s", command.c_str());
		std::system(command.c_str());
	}
};

bool Env::existVar(const std::string& name) {
	return vars.find(name) != vars.end();
}
std::string Env::getVar(const std::string& name) {
	return vars.find(name)->second;
}
void Env::setVar(const std::string& name, const std::string& value) {
	vars[name] = value;
}

void Env::load(const std::string& text) {
	Lexer lexer(text);
	Parser parser(lexer);
	while (parser.has_next()) {
		Instruction inst;
		parser.next(inst);
		if (inst.type == kLabel) {
			std::map<std::string, size_t>::iterator it = labels.find(inst.name);
			if (it == labels.end()) {
				Log::debug("label: %s, idx %zu", inst.name.c_str(),
						insts.size());
				labels[inst.name] = insts.size();
			}
		} else
			insts.push_back(inst);
	}
}

Engine::Engine() {
	cmds["exit"] = Commands::exit_cmd;
	cmds["goto"] = Commands::goto_cmd;
	cmds["say"] = Commands::say_cmd;
	cmds["read"] = Commands::read_cmd;
	cmds["if"] = Commands::if_cmd;
	cmds["shell"] = Commands::shell_cmd;
}

void Engine::launch(Env& env) {
	while (!env.finish()) {
		Instruction& pc = env.nextInst();
		Log::debug("当前执行指令：%s", pc.to_str().c_str());
		how handle = getCmd(pc.name);
		if (handle == NULL) {
			Log::error("无法识别的命令“%s”", pc.name.c_str());
			error("无法识别的命令！", pc.pos);
		}
		handle(env, pc);
	}
}
}

