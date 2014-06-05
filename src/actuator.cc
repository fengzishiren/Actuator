/*
 * Actuator.cc
 *
 *      Author: Lunatic
 */

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

Actuator::Actuator() {

}

Actuator::~Actuator() {
}

void buildInstruction() {
}

/*
 * 将Token转换为Instruction{type, name, params}
 *
 */
void Actuator::load(const std::string& script_code) {
	Lexer lexer(script_code);
	std::vector<Token> tokens;
	Instruction inst;
	while (!lexer.finish()) {
		tokens.clear();
		size_t expect = lexer.col;
		do {
			tokens.push_back(lexer.next_token());
		} while (!lexer.finish() && expect == lexer.col);

		inst.name = tokens[0].token;
		inst.pos = tokens[0].pos;

		if (tokens.empty() || tokens[0].type != kName) {
			error("语法错误", tokens[0].pos);
		}
		if (tokens.size() == 1) {
			inst.type = kInstruction;
		} else if (tokens[1].type == kColon) {
			inst.type = kLabel;
		} else { //�����ָ����Ӳ���
			inst.type = kInstruction;
			for (size_t i = 1; i < tokens.size(); ++i) {
				switch (tokens[i].type) {
				case kInt:
				case kReal:
				case kString:
				case kName:
					inst.params.push_back(tokens[i]);
					break;
				default:
					error("语法错误 参数不符合要求！", tokens[0].pos);
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
}

std::string get_val_or_var(Env& env, Token& token) {
	if (token.type == kName) { //处理变量
		if (env.contains(token.token)) {
			return env.get(token.token);
		} else
			error("语法错误 变量未定义" + token.token, token.pos);
	}
	return token.token;
}

std::string eval(const std::string& cmd, const std::string& arg1,
		const std::string& arg2) {
	if (cmd == "add") { // +

	} else if (cmd == "min") { //-

	} else if (cmd == "mul") { // *

	} else if (cmd == "div") { // /

	}
	return std::string();
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
		if (pc.name == "exit") {
			if (pc.params.empty())
				break;
			else
				error("语法错误 exit不能有参数", pc.pos);
		} else if (pc.name == "goto") {
			if (pc.params.size() == 1 && pc.params[0].type == kName) {
				//处理指令跳转
				std::map<std::string, size_t>::iterator it = labels.find(
						pc.params[0].token);
				if (it == labels.end()) {
					error("找不到带跳转位置", pc.pos);
				} else
					idx = it->second; //更新到下一条指令
			} else {
				error("goto语句后面必须带有一个跳转Label", pc.pos);
			}
		} else if (pc.name == "set") {
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
				std::string arg1 = get_val_or_var(env, pc.params[1]);
				std::string arg2 = get_val_or_var(env, pc.params[2]);
				env.put(pc.params[0].token, eval(pc.name, arg1, arg2));
			} else {
				error(pc.name + "命令需要一个变量和两个参数", pc.pos);
			}
		} else if(pc.name == "if") { //if

		}

		///////////////////////////////////////////////////
		////////////////未完待续
		///////////////////////////////////////////////////

	}
}

} /* namespace Script */
