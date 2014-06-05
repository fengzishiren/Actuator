/*
 * Lexer.cc
 *
 *      Author: Lunatic
 */

#include "lexer.h"
#include "alarm.h"

namespace Script {

Lexer::~Lexer() {
}

void Lexer::forward() {
	if (text[offset++] == '\n') {
		row++;
		col = 0;
	} else {
		col++;
	}
}

void Lexer::skip_space() {
	while (text[offset] == ' ' || text[offset] == '\t') {
		forward();
	}
}

bool Lexer::get_name(std::string& name) {
	bool found = false;
	if (('a' <= text[offset] && text[offset] <= 'z') || text[offset] == '_') {
		found = true;
		do {
			name += text[offset];
			forward();
		} while (!finish()
				&& (('a' <= text[offset] && text[offset] <= 'z')
						|| text[offset] == '_'
						|| ('0' <= text[offset] && text[offset] <= '9')));
		if (finish()) {
			error("语法错误");
		}
	}
	return found;
}
bool Lexer::get_string(std::string& name) {
	bool found = false;
	if (text[offset] == '\"') {
		found = true;
		do {
			name += text[offset];
			forward();
		} while (!finish() && text[offset] != '\"');
		if (finish()) {
			error("语法错误");
		}
	}
	return found;
}

//bool Lexer::get_real(std::string& name) {
//	bool found = false;
//	if (get_int(name) && text[offset] == '.') {
//		name += '.';
//		forward();
//		if (finish()) {
//			error("语法错误");
//		}
//		if (get_int(name)) {
//			found = true;
//		}
//	}
//	return found;
//}

bool Lexer::get_num(std::string& name, bool& is_int) {
	bool found = false;
	is_int = true;
	if ((found = get_int(name)) && text[offset] == '.') {
		is_int = false;
		name += '.';
		forward();
		if (finish()) {
			error("语法错误", Position(row, col));
		}
		if (get_int(name)) {
			found = true;
		} else
			error("语法错误", Position(row, col));
	}
	return found;
}

bool Lexer::get_int(std::string& name) {
	bool found = false;
	if ('0' <= text[offset] && text[offset] <= '9') {
		found = true;
		do {
			name += text[offset];
			forward();
		} while (!finish() && '0' <= text[offset] && text[offset] <= '9');
		if (finish()) {
			error("语法错误", Position(row, col));
		}
	}
	return found;
}

Token Lexer::next_token() {
	std::string name;
	skip_space();

	bool isint;

	if (get_name(name))
		return Token(kName, name, row, col);
	if (get_string(name))
		return Token(kString, name, row, col);

	if (get_num(name, isint))
		return isint ?
				Token(kInt, name, row, col) : Token(kReal, name, row, col);

	if (text[offset] == ':')
		return Token(kString, name, row, col);
	else
		error("语法错误", Position(row, col));
	return Token(kString, name, row, col); //���ɴ�
}

} /* namespace Script */
