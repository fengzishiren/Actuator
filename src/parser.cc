/*
 * Parser.cc
 *
 *  Created on: 2014年6月6日
 *      Author: lunatic
 */

#include "parser.h"
#include "alarm.h"

namespace Script {

Parser::Parser(Lexer& _lexer) :
		lexer(_lexer) {
}
Parser::~Parser() {
}
void Parser::parse(std::vector<Instruction>& insts,
		std::map<std::string, size_t>& labels) const {
	bool loop = true;
	for (; loop;) {
		Instruction inst;
		std::vector<Token> tokens;
		for (;;) {
			Token token;
			lexer.next_token(token);
			if (token.type == kEnd) {
				loop = false;
				break;
			} else if (token.type == kLF)
				break;
			else
				tokens.push_back(token);
		}
		if (tokens.empty())
			continue;

		if (tokens[0].type != kName) {
			error("语法错误load", tokens[0].pos);
		}

		inst.name = tokens[0].content;
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
					Log::error(tokens[i].to_str());
					error("参数不符合要求！", tokens[i].pos);
				}
			}
		}
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

} /* namespace Script */
