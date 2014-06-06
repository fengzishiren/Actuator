/*
 * Parser.h
 *
 *  Created on: 2014年6月6日
 *      Author: lunatic
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <vector>

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

	Instruction() :
			type(kInstruction), pos(-1, -1) {
	}
	virtual ~Instruction() {
	}
	std::string to_str() {
		std::stringstream ss;
		ss << "Instruction：" << name << " type: " << (type == kInstruction) ?
				"指令" : "标签";
		for (std::vector<Token>::iterator it = params.begin();
				it != params.end(); ++it) {
			ss << "\t" << it->to_str();
		}
		return ss.str();
	}

};

class Parser {
private:
	Lexer& lexer;
public:
	Parser(Lexer& _lexer);
	virtual ~Parser();
	bool has_next();
	Instruction& next(Instruction& inst);
};

} /* namespace Script */

#endif /* PARSER_H_ */
