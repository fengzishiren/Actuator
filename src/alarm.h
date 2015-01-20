/*
 * Alarm.h
 *
 *      Author: Lunatic
 */

#ifndef ALARM_H_
#define ALARM_H_

#include <string>
#include <cstdarg>

class Position;
namespace Script {

    void error(const std::string &err, const Position &pos);

    class Log {
    public:
        enum Level {
            DEBUG, INFO, WARN, ERROR
        };
    private:
        static void format(Level lv, const std::string &msg, va_list va);

    public:
        static Level level;

        static void debug(const std::string &msg, ...);

        static void info(const std::string &msg, ...);

        static void warn(const std::string &msg, ...);

        static void error(const std::string &msg, ...);
    };

} /* namespace Script */

#endif /* ALARM_H_ */
