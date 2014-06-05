/*
 * Alarm.h
 *
 *      Author: Lunatic
 */

#ifndef ALARM_H_
#define ALARM_H_

#include <iostream>
#include <cstdlib>

#include "lexer.h"

namespace Script {
void error(const std::string& err, const Position& pos);

void error(const std::string& err);
} /* namespace Script */

#endif /* ALARM_H_ */
