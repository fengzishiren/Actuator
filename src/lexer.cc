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
	//Log::info("x, y %d, %d", row, col);
}

void Lexer::skip_space() {
	while (text[offset] == ' ' || text[offset] == '\t' || text[offset] == '\n') {
		forward();
	}
}

bool Lexer::get_name(std::string& name) {
	bool found = false;
	if (('a' <= text[offset] && text[offset] <= 'z')
			|| ('A' <= text[offset] && text[offset] <= 'Z')
			|| text[offset] == '_') {
		found = true;
		do {
			name += text[offset];
			forward();
		} while (!finish()
				&& (('a' <= text[offset] && text[offset] <= 'z')
						|| ('A' <= text[offset] && text[offset] <= 'Z')
						|| text[offset] == '_'
						|| ('0' <= text[offset] && text[offset] <= '9')));

	}
	return found;
}
bool Lexer::get_string(std::string& name) {
	bool found = false;
	if (text[offset] == '\"') {
		found = true;
		forward(); //skip '\"'
		do {
			name += text[offset];
			forward();
		} while (!finish() && text[offset] != '\"');
		if (!finish()) {
			forward(); //skip '\"'
		}
	}
	return found;
}

bool Lexer::get_num(std::string& name, bool& is_int) {
	bool found = false;
	is_int = true;
	if ((found = get_int(name)) && text[offset] == '.') {
		is_int = false;
		name += '.';
		forward();

		if (!finish() && get_int(name)) {
			found = true;
		} else
			error("语法错误get_num", Position(row, col));
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
	}
	return found;
}
// > < = >= <= !=
bool Lexer::get_cmp(std::string& name) {
	switch (text[offset]) {
	case '=':
		name += text[offset];
		forward();
		if (text[offset] != '=')
			error("语法错误get_cmp", Position(row, col));
		name += text[offset];
		forward();
		break;
	case '!':
		name += text[offset];
		forward();
		if (text[offset] != '=')
			error("语法错误get_cmp", Position(row, col));
		name += text[offset];
		forward();
		break;
	case '<':
	case '>':
		name += text[offset];
		forward();
		if (text[offset + 1] == '=') {
			forward();
			name += text[offset];
		}
		forward();
	}

	return !name.empty();
}

Token Lexer::next_token() {
	std::string name;
	bool isint;
	size_t x = row, y = col;
	skip_space();

	if (get_name(name))
		return Token(kName, name, x, y);

	if (get_string(name))
		return Token(kString, name, x, y);

	if (get_num(name, isint))
		return isint ? Token(kInt, name, x, y) : Token(kReal, name, x, y);

	if (get_cmp(name))
		return Token(KCmp, name, x, y);

	if (text[offset] == ':') {
		forward();
		return Token(kColon, name += ':', x, y);
	}
	if (finish())
		return Token(kString, name, x, y); //理论不可达
	error("语法错误next_token", Position(x, y));
	return Token(kString, name, x, y); //理论不可达
}

} /* namespace Script */
