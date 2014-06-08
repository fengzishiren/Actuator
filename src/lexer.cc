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

/*
 * return =0 finish
 * 		  >0 newLine 具体的数值代表遇到几个换行符
 *
 */
int Lexer::next_token(Token& token) {
	std::string name;
	bool isint;
	size_t x = row, y = col;
	skip_space();//跳过所有的无效字符 空白换行等。。。

	if (finish())
		return 0;
	else if (row != x) {
		return row - x;
	}

	if (get_name(name))
		token.init(kName, name, x, y);

	else if (get_string(name))
		token.init(kString, name, x, y);

	else if (get_num(name, isint))
		token.init(isint ? kInt : kReal, name, x, y);

	else if (get_cmp(name))
		token.init(KCmp, name, x, y);

	else if (text[offset] == ':') {
		forward();
		token.init(kColon, name += ':', x, y);
	} else
		error("语法错误next_token", Position(x, y));
	return -1; //理论不可达
}

} /* namespace Script */
