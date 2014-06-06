//============================================================================
// Name        : Actuator.cpp
// Author      : Lunatic
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>

#include "actuator.h"
#include "alarm.h"
#include "lexer.h"

std::string& get_code(const std::string& file, std::string& code) {
	std::ifstream is(file.c_str());
	std::string s;
	while (std::getline(is, s)) {
		code = code.append(s) + '\n'; //注意： 各平台的换行符不能统一 在读取的时候统一用'\n'替换
	}
	return code;
}

int main(int argc, char **argv) {
	std::string text;
	Script::Env env;//执行环境

	Script::Lexer lexer(get_code("sample.se", text));//词法分析器

	Script::Parser parser(lexer);//解析器

	Script::Actuator actuator(parser);//执行器

	actuator.load();//加载指令
	actuator.run(env);//执行
//
//	Script::Log::debug("Start:");
//	Script::Actuator actuator;
//
//	actuator.load(get_code("sample.se", text));
//	actuator.run(env);

	Script::Log::debug("End!:");
}

