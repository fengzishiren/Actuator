/*
 * Lexer.h
 *
 *      Author: Lunatic
 */

#ifndef LEXER_H_
#define LEXER_H_

#include <iostream>
#include <string>
#include <sstream>

#include "lexer.h"


namespace Script {

enum TokenType {
	kInt, kReal, kString, KCmp, kName, kColon
};

class Position {
public:
	size_t x, y;
	Position(size_t _x, size_t _y) :
			x(_x), y(_y) {
	}

	std::string to_str() const {
		if (x < 0 || y < 0)
			return std::string();
		std::stringstream ss;
		//Note: 从0开始
		ss << "[" << x + 1 << ", " << y + 1 << "]";
		return ss.str();
	}
};

class Token {
public:
	TokenType type;
	std::string token;
	Position pos;

	Token() :
			type(kInt), pos(-1, -1) {
	}

	void init(TokenType _type, const std::string& _token, size_t row,
			size_t col) {
		type = _type;
		token = _token;
		pos = Position(row, col);
	}

	std::string to_str() const {
		std::stringstream ss;
		static const char *typeinfo[] = { "kInt", "kReal", "kString", "KCmp",
				"kName", "kColon" };
		ss << "TokenType: " << typeinfo[type] << "\t" << "Token: " << token
				<< "\t" << " Pos: " << pos.to_str();
		return ss.str();
	}
};

class Lexer {
public:
	const std::string& text;
	size_t offset, col, row;

	Lexer(const std::string& _text) :
			text(_text), offset(0), col(0), row(0) {
	}
	virtual ~Lexer();

	bool finish() {
		return offset == text.size();
	}

	bool get_name(std::string& name);
	bool get_string(std::string& name);
	bool get_num(std::string& name, bool& is_int);
	bool get_int(std::string& name);
	bool get_cmp(std::string& name);
	void skip_space();
	void forward();
	int next_token(Token& token);
};

} /* namespace Script */

#endif /* LEXER_H_ */
