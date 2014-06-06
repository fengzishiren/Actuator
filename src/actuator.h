/*
 * Actuator.h
 *
 *      Author: Lunatic
 */

#ifndef ACTUATOR_H_
#define ACTUATOR_H_

#include <string>
#include <sstream>
#include <vector>
#include <map>

#include "parser.h"

namespace Script {

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
 * 执行引擎
 */
class Actuator {
private:
	Parser& parser;
	//待执行指令集
	std::vector<Instruction> insts;
	//指令label索引
	std::map<std::string, size_t> labels;
public:
	Actuator(Parser& parser);
	virtual ~Actuator();
	//解析并加载所有指令
	void load();
	//void load(const std::string& script_code);
	void run(Env& env);
};

} /* namespace Script */

#endif /* ACTUATOR_H_ */
