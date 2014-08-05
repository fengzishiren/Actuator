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
//#include <cassert>

namespace Script {

enum TokenType {
	kInt, kReal, kString, KCmp, kName, kColon, kLF, kEnd
};
/*
 *
 * 定位到文件的具体位置
 */
class Position {
public:
	size_t x, y;
	Position() :
			x(0), y(0) {
	}
	Position(size_t _x, size_t _y) :
			x(_x), y(_y) {
	}
	std::string to_str() const {
		//assert(x >= 0 && y >= 0);
//		if (x < 0 || y < 0)
//			return std::string();
		std::stringstream ss;
		//Note: 从0开始
		ss << "[" << x + 1 << ", " << y + 1 << "]";
		return ss.str();
	}
};

class Token {
public:
	TokenType type;
	std::string content;
	Position pos;

	Token() :
			type(kInt) {
	}

	std::string to_str() const {
		std::stringstream ss;
		static const char *typeinfo[] = { "kInt", "kReal", "kString", "KCmp",
				"kName", "kColon" };
		ss << "TokenType: " << typeinfo[type] << "\t" << "Token: " << content
				<< "\t" << " Pos: " << pos.to_str();
		return ss.str();
	}
};

class Lexer {
public:
	const std::string& text;
	size_t offset;
	size_t row, col; //Location:[x, y]

	Lexer(const std::string& _text) :
			text(_text), offset(0), row(0), col(0) {
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
	bool skip_space();
	bool skip_comment();

	void forward();
	Token& next_token(Token& token);
};

} /* namespace Script */

#endif /* LEXER_H_ */
