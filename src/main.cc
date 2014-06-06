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

	Script::Log::debug("-------------");
	Script::Actuator actuator;
	Script::Env env;

	actuator.load(get_code("sample.se", text));
	actuator.run(env);

	std::cout << "End!" << std::endl;
}

