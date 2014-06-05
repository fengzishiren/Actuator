/*
 * Alarm.cc
 *
 *      Author: Lunatic
 */
#include <iostream>
#include "alarm.h"

namespace Script {

void error(const std::string& err, const Position& pos) {
    std::cerr << err << ": " << pos.to_str() << std::endl;
    std::exit(-1);
}

void error(const std::string& err) {
    std::cerr << err << std::endl;
    std::exit(-1);
}


} /* namespace Script */
