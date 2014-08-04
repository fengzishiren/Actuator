//============================================================================
// Name        : Actuator.cpp
// Author      : Lunatic
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>

#include "alarm.h"
#include "engine.h"

std::string& get_code(const std::string& file, std::string& code) {
	std::ifstream is(file.c_str());
	std::string s;
	while (std::getline(is, s)) {
		code = code.append(s) + '\n'; //注意： 各平台的换行符不能统一 在读取的时候统一用'\n'替换
	}
	return code;
}

int main(int argc, char **argv) {
	Script::Log::debug("Start:");

	std::string text;
	get_code("2.se", text);
	Script::Env env; //执行环境
	env.load(text);

	Script::Engine engine; //执行器
	try {
		engine.launch(env); //执行
	} catch (const char *s) {
		std::cout << s << std::endl;
	}

	Script::Log::debug("End!");

}

