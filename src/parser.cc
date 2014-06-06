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
	// TODO Auto-generated destructor stub
}

bool Parser::has_next() {
	return !lexer.finish();

}

Instruction& Parser::next(Instruction& inst) {
	std::vector<Token> tokens; //收集一行中的token
	Log::info("新的一行");
	Token token;
	for (;;) {
		int stat = lexer.next_token(token);
		if (stat > 0 && tokens.empty()) //跳过一坨空白符
			continue;
		if (stat >= 0) //Eof
			break;
		Log::info(token.to_str());
		tokens.push_back(token);
	}

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

	/*if (inst.type == kLabel) {
	 std::map<std::string, size_t>::iterator it = labels.find(inst.name);
	 if (it == labels.end())
	 labels[inst.name] = insts.size();
	 } else
	 insts.push_back(inst);*/
	return inst;
}

} /* namespace Script */
