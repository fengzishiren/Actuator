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
	while (parser.has_next()) {
		Instruction inst;
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
 * 如果是变量 存在则直接取出 否则报错 ？不太妥当
 * 其他直接返回
 *
 */
std::string get_val(Env& env, Token& token) {
	if (token.type == kName) { //处理变量
		if (env.contains(token.content)) {
			return env.get(token.content);
		} else
			error("变量未定义" + token.content, token.pos); //？不太妥当
	}
	return token.content;
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
		const std::string& arg2, const Position& pos) {

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
			error("除数不能为0！", pos);
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
 * mov
 * ...
 *
 *
 *
 */
void Actuator::run(Env& env) {
	for (size_t idx = 0; idx < insts.size(); /*++idx*/) {
		Instruction& pc = insts[idx]; //模拟PC寄存器
		Log::debug("正在执行的指令： %s", pc.to_str().c_str());

		if (pc.name == "exit") {
			if (pc.params.empty())
				break;
			else
				error("exit不能有参数", pc.pos);
		} else if (pc.name == "goto") {
			if (pc.params.size() == 1 && pc.params[0].type == kName) {
				//处理指令跳转
				std::map<std::string, size_t>::iterator it = labels.find(
						pc.params[0].content);
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
				std::string val = get_val(env, pc.params[1]);
				env.put(pc.params[0].content, val);
			} else {
				error("set语句必须是一个变量和一个对象参数", pc.pos);
			}
		} else if (pc.name == "add" || pc.name == "min" || pc.name == "mul"
				|| pc.name == "div") {
			//add a b 1 => a = b + 1
			if (pc.params.size() == 3 && pc.params[0].type == kName) {
				//把param1和param2求值 把结果放入param0为key结果为value的环境map中
				std::string arg1 = get_val(env, pc.params[1]);
				std::string arg2 = get_val(env, pc.params[2]);
				env.put(pc.params[0].content, eval(pc.name, arg1, arg2, pc.params[2].pos));
			} else {
				error(pc.name + "命令需要一个变量和两个参数", pc.pos);
			}
		} else if (pc.name == "if") { //if value1 [opcode value2] goto label
			bool jmp = false;
			if (pc.params.size() == 5 && pc.params[1].type == KCmp) { //if val goto label
				std::string left = get_val(env, pc.params[0]);
				std::string right = get_val(env, pc.params[2]);

				if (pc.params[1].content == "==") {
					jmp = left == right;
				} else if (pc.params[1].content == "!=") {
					jmp = left != right;
				} else if (pc.params[1].content == "<") {
					jmp = str2double(left) < str2double(right);
				} else if (pc.params[1].content == ">") {
					jmp = str2double(left) > str2double(right);
				} else if (pc.params[1].content == "<=") {
					jmp = str2double(left) <= str2double(right);
				} else if (pc.params[1].content == ">=") {
					jmp = str2double(left) < str2double(right);
				}

				if (jmp && pc.params[3].type == kName
						&& pc.params[3].content == "goto"
						&& pc.params[4].type == kName) {
					//处理指令跳转
					std::map<std::string, size_t>::iterator it = labels.find(
							pc.params[4].content);
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
					if (env.contains(it->content)) {
						std::cout << env.get(it->content);
					} else
						error("变量未定义！", it->pos);
				} else
					std::cout << it->content;
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
				env.put(it->content, temp);
			}
		} else {
			error("无法识别的指令", pc.pos);
		}
		idx++;
	}

}

} /* namespace Script */
