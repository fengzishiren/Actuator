/*
 * Actuator.cc
 *
 *      Author: Lunatic
 */

#include <cstring>
#include <cstdlib>

#include "actuator.h"
#include "lexer.h"
#include "alarm.h"

namespace Script {

bool Env::contains(const std::string& name) {
	return vars.find(name) != vars.end();
}
std::string Env::get(const std::string& name) {
	return vars.find(name)->second;
}
void Env::put(const std::string& name, const std::string& value) {
	vars[name] = value;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

Actuator::Actuator(Parser& _parser) :
		parser(_parser) {
}

Actuator::~Actuator() {
}

void Actuator::load() {
	Instruction inst;
	while (parser.has_next()) {
		parser.next(inst);
		if (inst.type == kLabel) {
			std::map<std::string, size_t>::iterator it = labels.find(inst.name);
			if (it == labels.end())
				labels[inst.name] = insts.size();
		} else
			insts.push_back(inst);
	}

}

/*
 * 将Token转换为Instruction{type, name, params}
 *

void Actuator::load(const std::string& script_code) {
	Lexer lexer(script_code);
	std::vector<Token> tokens;
	while (!lexer.finish()) {
		Log::info("新的一行");

		Instruction inst;
		tokens.clear();

		for (;;) {
			Token token;
			int stat = lexer.next_token(token);
			if (stat >= 0)
				break;
			Log::info(token.to_str());
			tokens.push_back(token);
		}

		if (tokens.empty())
			continue;
		if (tokens[0].type != kName) {
			error("语法错误load", tokens[0].pos);
		}

		inst.name = tokens[0].token;
		inst.pos = tokens[0].pos;

		if (tokens.size() == 1) {
			inst.type = kInstruction;
		} else if (tokens[1].type == kColon) {
			inst.type = kLabel;
		} else { //处理指令参数
			inst.type = kInstruction;
			for (size_t i = 1; i < tokens.size(); ++i) {
				switch (tokens[i].type) {
				case kInt:
				case kReal:
				case kString:
				case kName:
				case KCmp:
					inst.params.push_back(tokens[i]);
					break;
				default:
					Log::error("%zu", i);
					Log::error(tokens[i].to_str());
					error("参数不符合要求！", tokens[i].pos);
				}
			}
		}

		if (inst.type == kLabel) {
			std::map<std::string, size_t>::iterator it = labels.find(inst.name);
			if (it == labels.end())
				labels[inst.name] = insts.size();
		} else
			insts.push_back(inst);

	}
	Log::info("加载完毕！");
}
*/

/*
 * 如果是变量 存在则直接取出 否则报错 ？不太妥当
 * 其他直接返回
 *
 */
std::string get_val_or_var(Env& env, Token& token) {
	if (token.type == kName) { //处理变量
		if (env.contains(token.token)) {
			return env.get(token.token);
		} else
			error("变量未定义" + token.token, token.pos); //？不太妥当
	}
	return token.token;
}

bool is_real(const std::string& number) {
	return std::string::npos != number.find('.');
}

bool is_int(const std::string& number) {
	return !is_real(number);
}

long long str2int(const std::string& val) {
//char *err = NULL;
//long long retval = std::strtoll(val.c_str(), &err, 10);
//if (*err == '\0')
//return retval;

//忽略错误
	return std::strtoll(val.c_str(), NULL, 10);
}

long double str2double(const std::string& val) {
//char *err = NULL;
//long double retval = std::strtold(val.c_str(), &err);
//if (*err == '\0')
//return retval;
//else
//
//error!
//return 0.0;
	return std::strtold(val.c_str(), NULL);

}
//3.3333

bool is_zero(const std::string& vfloat) {
	bool left = true, right = true;
	size_t start = 0, pos = vfloat.find('.');

	if (pos == std::string::npos)
		pos = vfloat.size();

	while (start < pos) {
		if (vfloat[start++] != '0') {
			left = false;
		}
	}
	while (++pos < vfloat.size()) {
		if (vfloat[pos] != '0') {
			right = false;
		}
	}
	return left && right;

}

std::string eval(const std::string& cmd, const std::string& arg1,
		const std::string& arg2) {

#define EVAL(RES, X, Y, OP)                          \
	do {                                             \
		if(is_real(X) || is_real(Y)) {               \
			RES << (str2double(X) OP str2double(Y)); \
		} else {                                     \
			RES << (str2int(X) OP str2int(Y));       \
		}                                            \
	}while(0)                                        \

	std::stringstream ss;
	if (cmd == "add") { // +
		EVAL(ss, arg1, arg2, +);
	} else if (cmd == "min") { //-
		EVAL(ss, arg1, arg2, -);
	} else if (cmd == "mul") { // *
		EVAL(ss, arg1, arg2, *);
	} else if (cmd == "div") { // /
		if (is_zero(arg2)) {
			error("除数不能为0！");
		}
		EVAL(ss, arg1, arg2, /);
	}
	return ss.str();

#undef EVAL
}

/*
 *
 * 依次处理指令
 * exit
 * goto
 * set
 *
 *
 *
 */
void Actuator::run(Env& env) {
	for (size_t idx = 0; idx < insts.size(); /*++idx*/) {
		Instruction& pc = insts[idx]; //模拟PC寄存器
		Log::info(pc.to_str());

		if (pc.name == "exit") {
			if (pc.params.empty())
				break;
			else
				error("exit不能有参数", pc.pos);
		} else if (pc.name == "goto") {
			if (pc.params.size() == 1 && pc.params[0].type == kName) {
				//处理指令跳转
				std::map<std::string, size_t>::iterator it = labels.find(
						pc.params[0].token);
				if (it == labels.end()) {
					error("找不到带跳转位置", pc.pos);
				} else {
					idx = it->second; //更新到下一条指令
					continue;
				}
			} else {
				error("goto语句后面必须带有一个跳转Label", pc.pos);
			}
		} else if (pc.name == "mov") {
			if (pc.params.size() == 2 && pc.params[0].type == kName) {
				std::string val = get_val_or_var(env, pc.params[1]);
				env.put(pc.params[0].token, val);
			} else {
				error("set语句必须是一个变量和一个对象参数", pc.pos);
			}
		} else if (pc.name == "add" || pc.name == "min" || pc.name == "mul"
				|| pc.name == "div") {
			//add a b 1 => a = b + 1
			if (pc.params.size() == 3 && pc.params[0].type == kName) {
				//把param1和param2求值 把结果放入param0为key结果为value的环境map中
				std::string arg1 = get_val_or_var(env, pc.params[1]);
				std::string arg2 = get_val_or_var(env, pc.params[2]);
				env.put(pc.params[0].token, eval(pc.name, arg1, arg2));
			} else {
				error(pc.name + "命令需要一个变量和两个参数", pc.pos);
			}
		} else if (pc.name == "if") { //if value1 [opcode value2] goto label
			bool jmp = false;
			if (pc.params.size() == 5 && pc.params[1].type == KCmp) { //if val goto label
				std::string left = get_val_or_var(env, pc.params[0]);
				std::string right = get_val_or_var(env, pc.params[2]);

				if (pc.params[1].token == "==") {
					jmp = left == right;
				} else if (pc.params[1].token == "!=") {
					jmp = left != right;
				} else if (pc.params[1].token == "<") {
					jmp = str2double(left) < str2double(right);
				} else if (pc.params[1].token == ">") {
					jmp = str2double(left) > str2double(right);
				} else if (pc.params[1].token == "<=") {
					jmp = str2double(left) <= str2double(right);
				} else if (pc.params[1].token == ">=") {
					jmp = str2double(left) < str2double(right);
				}

				if (jmp && pc.params[3].type == kName
						&& pc.params[3].token == "goto"
						&& pc.params[4].type == kName) {
					//处理指令跳转
					std::map<std::string, size_t>::iterator it = labels.find(
							pc.params[4].token);
					if (it == labels.end()) {
						error("找不到带跳转位置", pc.params[4].pos);
					} else {
						idx = it->second; //更新到下一条指令
						continue;
					}
				} else if (jmp) {
					error("goto语句后面必须带有一个跳转Label", pc.params[4].pos);
				}
			}
		} else if (pc.name == "print") {

			for (std::vector<Token>::iterator it = pc.params.begin();
					it != pc.params.end(); ++it) {
				if (it->type == kName) {
					if (env.contains(it->token)) {
						std::cout << env.get(it->token);
					} else
						error("变量未定义！", it->pos);
				} else
					std::cout << it->token;
				std::cout << std::endl;
			}
		} else if (pc.name == "read") {

			for (std::vector<Token>::iterator it = pc.params.begin();
					it != pc.params.end(); ++it) {
				if (it->type != kName) {
					error("必须是变量！", it->pos);
				}
				std::string temp;
				std::cin >> temp;
				env.put(it->token, temp);
			}
		}
		idx++;
	}

}

} /* namespace Script */
