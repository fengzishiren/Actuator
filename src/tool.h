/*
 * Alarm.h
 *
 *      Author: Lunatic
 */

#ifndef TOOL_H_
#define TOOL_H_

#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include "lexer.h"


namespace Script {

    template<class T>
    std::string repr(const T &t) {
        return t.repr();
    }


    template<class T>
    std::string join(const std::vector<T> &vt, char sep) {
        size_t size = vt.size();
        if (size == 0) return std::string();
        std::stringstream ss;
        ss << repr(vt[0]);
        for (size_t i = 1; i < size; ++i) {
            ss << sep << repr(vt[i]);
        }
        return ss.str();
    }

    std::string join(const std::vector<std::string> &vt, char sep);


    template<class T>
    std::string join(const std::vector<T *> &vt, char sep) {
        size_t size = vt.size();
        if (size == 0) return std::string();

        std::stringstream ss;
        ss << repr(*vt[0]);
        for (size_t i = 1; i < size; ++i) {
            ss << sep << repr(*vt[i]);

        }
        return ss.str();
    }

    void error(const std::string &err, size_t row, size_t col);

    class Position;

    void error(const std::string &err, const Position &pos);

    static inline size_t now() {
        return (size_t) std::time(nullptr);
    }

    void assert(bool cond, const std::string &msg, const Position &pos);

    std::string format(const char *fmt, ...);

    class Log {
    public:
        enum Level {
            DEBUG, INFO, WARN, ERROR
        };
    private:

        static void format(Level lv, const std::string &tag, const std::string &msg, va_list va);

    public:
        static Level level;

        static void debug(const std::string &msg, ...);

        static void debug(const std::string tag, const std::string &msg, ...);

        static void info(const std::string &msg, ...);

        static void info(const std::string tag, const std::string &msg, ...);

        static void warn(const std::string &msg, ...);

        static void warn(const std::string tag, const std::string &msg, ...);

        static void error(const std::string &msg, ...);

        static void error(const std::string tag, const std::string &msg, ...);

    };

} /* namespace Script */

#endif /* ALARM_H_ */
