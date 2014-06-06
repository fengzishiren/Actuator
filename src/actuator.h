/*
 * Actuator.h
 *
 *      Author: Lunatic
 */

#ifndef ACTUATOR_H_
#define ACTUATOR_H_

//#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "lexer.h"

namespace Script {

enum InstructionType {
	kInstruction, kLabel
};

class Instruction {
public:
	InstructionType type;
	std::string name;
	std::vector<Token> params;
	Position pos;

	Instruction():type(kInstruction), pos(-1, -1){
	}
	virtual ~Instruction(){}
};

/*
 *
 * 环境变量
 */
class Env {
private:
	std::map<std::string, std::string> vars;
public:
	bool contains(const std::string& name);
	std::string get(const std::string& name);
	void put(const std::string& name, const std::string& value);

};
//
//class Plugin {
//public:
//	virtual void execute(const Instruction& inst, Env& e) = 0;
//	virtual ~Plugin() {}
//};

/**
 * ִ������
 */
class Actuator {
private:
	//待执行指令集
	std::vector<Instruction> insts;
	//指令label索引
	std::map<std::string, size_t> labels;
public:
	Actuator();
	virtual ~Actuator();

	void load(const std::string& script_code);
	void run(Env& env);
};

} /* namespace Script */

#endif /* ACTUATOR_H_ */
