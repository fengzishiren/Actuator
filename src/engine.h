/*
 * env.h
 *
 *  Created on: 2014年8月4日
 *      Author: epay111
 */

#ifndef ENV_H_
#define ENV_H_
#include <map>
#include <vector>
#include <cstddef>

#include "parser.h"

namespace Script {

class Env {
private:
	size_t idx;//指令指针
	//待执行指令集
	std::vector<Instruction> insts;
	//指令label索引
	std::map<std::string, size_t> labels;
	std::map<std::string, std::string> vars;
public:
	Env() :
			idx(0) {
	}
	bool existVar(const std::string& name);
	std::string getVar(const std::string& name);
	void setVar(const std::string& name, const std::string& value);

	int getGotoIdx(const std::string& label) {
		std::map<std::string, size_t>::iterator it = labels.find(label);
		return it == labels.end() ? -1 : it->second;
	}
	void setIdx(size_t _idx) {
		idx = _idx;
	}

	bool finish() {
		return idx == insts.size();
	}

	Instruction& nextInst() {
		return insts[idx++];
	}

	void load(const std::string& text);

};

class Engine {
private:
	typedef void (*how)(Env&, Instruction&);
	std::map<std::string, how> cmds;
public:
	Engine();

	void launch(Env& env);

	how getCmd(const std::string& name) {
		std::map<std::string, how>::iterator it = cmds.find(name);
		return it == cmds.end() ? NULL : it->second;
	}

};

}  // namespace Script

#endif /* ENV_H_ */
