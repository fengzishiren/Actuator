/*
 * Alarm.cc
 *
 *      Author: Lunatic
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "alarm.h"
#include "lexer.h"

namespace Script {

void error(const std::string& err, const Position& pos) {
	std::cerr << err << ": " << pos.to_str() << std::endl;
	std::exit(-1);
}

#define STD_FORMAT "%s  %-6s  \t- %s\n"

static inline char *cur_time(char *buf) {
	time_t tt;
	struct tm *t;
	tt = time(NULL);
	t = localtime(&tt);
	strftime(buf, 20, "%Y-%m-%d %H:%M:%S", t);
	return buf;
}

static const char *levels[] = { "DEBUG", "INFO", "WARN", "ERROR" };

Log::Level Log::level = DEBUG;

void Log::format(Level lv, const std::string& msg, va_list va) {
	char *buffer;
	char time[20];
	int n, length = 1024;

	va_list args;
	buffer = new char[length];
	//
	//假设一次OK
	va_copy(args, va);
	n = vsnprintf(buffer, length, msg.c_str(), args);
	va_end(args);

	if (n < 0) {	//error!
		delete[] buffer;
		return; /*ignore error!*/
	}
	//一般非大文本不会到此
	//假设失败 以每次增大一倍的容量再进行测试
	if (n >= length) {
		delete[] buffer;
		for (;;) {
			/*
			 The  glibc  implementation  of  the  functions snprintf() and
			 vsnprintf() conforms to the C99 standard, that is, behaves as
			 described  above, since glibc version 2.1.*/
			length = n + 1; /* glibc 2.1 */
			buffer = new char[length];
			va_copy(args, va);
			n = vsnprintf(buffer, length, msg.c_str(), args);
			va_end(args);
			if (n < 0) {	//error!
				delete[] buffer;
				return; /*ignore error!*/
			}
			if (n < length)
				break;
		}
	}
	fprintf(stdout, STD_FORMAT, cur_time(time), levels[lv], buffer);
	delete[] buffer;
}

void Log::debug(const std::string& msg, ...) {
	if (DEBUG < level)
		return;
	va_list va;
	va_start(va, msg);
	format(DEBUG, msg, va);
	va_end(va);
}
void Log::info(const std::string& msg, ...) {
	if (INFO < level)
		return;
	va_list va;
	va_start(va, msg);
	format(INFO, msg, va);
	va_end(va);
}
void Log::warn(const std::string& msg, ...) {
	if (WARN < level)
		return;
	va_list va;
	va_start(va, msg);
	format(WARN, msg, va);
	va_end(va);
}
void Log::error(const std::string& msg, ...) {
	va_list va;
	va_start(va, msg);
	format(ERROR, msg, va);
	va_end(va);
}

} /* namespace Script */
