/** 
 * @file ConsoleLogHandler.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/11
 */
#include "handler/ConsoleLogHandler.h"

#include <iostream>

using namespace mnsx::argus;

void ConsoleLogHandler::handle(const LogEvent &event) {
    std::string log_line = formatEvent(event);

    {
        std::lock_guard<std::mutex> lock(console_mutex_);

        if (event.level == LogLevel::ERROR || event.level == LogLevel::FATAL) {
            std::cerr << log_line << "\n";
        } else {
            std::cout << log_line << "\n";
        }
    }
}