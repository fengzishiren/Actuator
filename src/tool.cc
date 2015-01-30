/*
 * tool.cc
 *
 *      Author: fengzishiren
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstdarg>
#include "tool.h"


namespace Script {
#define STD_FORMAT "%s  %-6s  \t- %s\n"
#define STD_TAG_FORMAT "%s  %-6s [%s]  \t- %s\n"
    //Log::Level Log::level = Log::INFO;
    Log::Level Log::level = DEBUG;

    std::string join(const std::vector<std::string> &vt, char sep) {
        size_t size = vt.size();
        if (size == 0) return std::string();
        std::stringstream ss;
        ss << vt[0];
        for (size_t i = 1; i < size; ++i) {
            ss << sep << vt[i];
        }
        return ss.str();
    }

    void error(const std::string &err, size_t row, size_t col) {
        std::cerr << "error: " << err << "(" << row + 1 << ", " << col + 1 << ")" << std::endl;
        std::exit(-1);
    }

    void error(const std::string &err, const Position &pos) {
        std::cerr << "error: " << err << " " << pos.repr() << std::endl;
        std::exit(-1);
    }

    std::string format(const char *fmt, ...) {
        int n;
        int size = 100;     /* Guess we need no more than 100 bytes */
        char *p;
        va_list ap;
        p = new char[size];
        while (1) {
            /* Try to print in the allocated space */
            va_start(ap, fmt);
            n = vsnprintf(p, size, fmt, ap);
            va_end(ap);
            /* Check error code*/
            if (n < 0)
                return std::string();
            /* If that worked, return the string */
            if (n < size) {
                std::string s(p);
                delete[] p;
                return s;
            }
            /* Else try again with more space */
            size = n + 1;       /* Precisely what is needed */
            delete[]p;
            p = new char[size];
        }
    }


    void assert(bool cond, const std::string &msg, const Position &pos) {
        if (!cond)
            error(msg, pos);
    }


    static inline char *cur_time(char *buf) {
        time_t tt;
        struct tm *t;
        tt = time(NULL);
        t = localtime(&tt);
        strftime(buf, 20, "%Y-%m-%d %H:%M:%S", t);
        return buf;
    }

    static const char *levels[] = {"DEBUG", "INFO", "WARN", "ERROR"};


    std::string &lower(std::string &src) {
        for (auto &c : src) {
            c = c <= 'Z' && c >= 'A' ? 'a' - 'A' + c : c;
        }
        return src;
    }

    std::string &upper(std::string &src) {
        for (auto &c : src) {
            c = c <= 'z' && c >= 'a' ? c - 'a' + 'A' : c;
        }
        return src;
    }

    void Log::format(Level lv, const std::string &tag, const std::string &msg, va_list va) {
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

        if (n < 0) {    //error!
            delete[] buffer;
            return; /*ignore error!*/
        }
        //一般非大文本不会到此
        //假设失败 以每次增大一倍的容量再进行测试
        if (n >= length) {
            delete[] buffer;
            for (; ;) {
                /*
                 The  glibc  implementation  of  the  functions snprintf() and
                 vsnprintf() conforms to the C99 standard, that is, behaves as
                 described  above, since glibc version 2.1.*/
                length = n + 1; /* glibc 2.1 */
                buffer = new char[length];
                va_copy(args, va);
                n = vsnprintf(buffer, length, msg.c_str(), args);
                va_end(args);
                if (n < 0) {    //error!
                    delete[] buffer;
                    return; /*ignore error!*/
                }
                if (n < length)
                    break;
            }
        }
        if (tag.empty())
            fprintf(stderr, STD_FORMAT, cur_time(time), levels[lv], buffer);
        else
            fprintf(stderr, STD_TAG_FORMAT, cur_time(time), levels[lv], tag.c_str(), buffer);

        delete[] buffer;
    }

    void Log::debug(const std::string &msg, ...) {
        if (DEBUG < level)
            return;
        va_list va;
        va_start(va, msg);
        format(DEBUG, "", msg, va);
        va_end(va);
    }

    void Log::debug(const std::string tag, const std::string &msg, ...) {
        if (DEBUG < level)
            return;
        va_list va;
        va_start(va, msg);
        format(DEBUG, tag, msg, va);
        va_end(va);
    }

    void Log::info(const std::string &msg, ...) {
        if (INFO < level)
            return;
        va_list va;
        va_start(va, msg);
        format(INFO, "", msg, va);
        va_end(va);
    }

    void Log::info(const std::string tag, const std::string &msg, ...) {
        if (INFO < level)
            return;
        va_list va;
        va_start(va, msg);
        format(INFO, tag, msg, va);
        va_end(va);
    }

    void Log::warn(const std::string &msg, ...) {
        if (WARN < level)
            return;
        va_list va;
        va_start(va, msg);
        format(WARN, "", msg, va);
        va_end(va);
    }

    void Log::warn(const std::string tag, const std::string &msg, ...) {
        if (WARN < level)
            return;
        va_list va;
        va_start(va, msg);
        format(WARN, tag, msg, va);
        va_end(va);
    }

    void Log::error(const std::string &msg, ...) {
        va_list va;
        va_start(va, msg);
        format(ERROR, "", msg, va);
        va_end(va);
    }

    void Log::error(const std::string tag, const std::string &msg, ...) {
        va_list va;
        va_start(va, msg);
        format(ERROR, tag, msg, va);
        va_end(va);
    }
} /* namespace Script */
