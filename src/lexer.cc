/*
 * Lexer.cc
 *
 *      Author: Lunatic
 */

#include "lexer.h"
#include "alarm.h"

#include <cctype>

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

bool Lexer::skip_space() {
	size_t old = offset;
	while (offset != text.size() && std::isspace(text[offset])) {
		forward();
	}
	return offset != old;

}

bool Lexer::skip_comment() {
	size_t old = offset;
	if (text[offset] == ';')
		do {
			forward();
		} while (offset != text.size() && text[offset] != '\n');
	return offset != old;
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

	if (text[offset] == '-') {// for args
		name += text[offset];
		forward();
		while (!finish() && !std::isspace(text[offset])) {
			name += text[offset];
			forward();
		}
		if (finish())
			error("语法错误！", Position(row, col));
	} else if (text[offset] == '\"') {
		forward(); //skip '\"'
		while (!finish() && text[offset] != '\"') {
			name += text[offset];
			forward();
		}
		if (finish())
			error("字符串缺少\"", Position(row, col));
		forward(); //skip '\"'
	}
	return !name.empty();
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
		if (finish())
			error("语法错误！", Position(row, col));
		if (text[offset] != '=')
			error("语法错误get_cmp", Position(row, col));
		name += text[offset];
		forward();
		break;
	case '!':
		name += text[offset];
		forward();
		if (finish())
			error("语法错误！", Position(row, col));
		if (text[offset] != '=')
			error("语法错误get_cmp", Position(row, col));
		name += text[offset];
		forward();
		break;
	case '<':
	case '>':
		name += text[offset];
		forward();
		if (finish())
			error("语法错误！", Position(row, col));
		if (text[offset] == '=') {
			name += text[offset];
			forward();
		}
	}
	return !name.empty();
}

/*
 * return =0 finish
 * 		  >0 newLine 具体的数值代表遇到几个换行符
 * 		  -1 ok
 *
 */
int Lexer::next_token(Token& token) {
	bool isint;
	size_t x = row;
	skip_space(); //跳过所有的无效字符 空白换行等。。。

	while (skip_space() || skip_comment()) {
		;
	}
	if (finish())
		return 0;
	else if (row != x) {
		return row - x;
	}
	token.pos.x = row;
	token.pos.y = col;

	if (get_name(token.content))
		token.type = kName;
	else if (get_string(token.content))
		token.type = kString;
	else if (get_num(token.content, isint))
		token.type = isint ? kInt : kReal;
	else if (get_cmp(token.content))
		token.type = KCmp;
	else if (text[offset] == ':') {
		forward();
		token.content += ':';
		token.type = kColon;
	} else
		error("语法错误next_token", token.pos);

	return -1;
}

} /* namespace Script */
