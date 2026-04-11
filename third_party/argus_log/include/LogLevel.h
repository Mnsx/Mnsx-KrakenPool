/** 
 * @file LogLevel.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/7
 * @description 日志级别枚举类
 */
#ifndef ARGUSLOG_LOGLEVEL_H
#define ARGUSLOG_LOGLEVEL_H

#include <string>

namespace mnsx {
    namespace argus {

        enum class LogLevel {
            DEBUG,
            INFO,
            WARN,
            ERROR,
            FATAL
        };

        /**
         * @brief 将日志级别转换为字符串
         * @param level 日志级别
         * @return 日志级别字符串
         */
        inline std::string levelToString(LogLevel level) {
            switch (level) {
                case LogLevel::DEBUG: return "DEBUG";
                case LogLevel::INFO:  return "INFO";
                case LogLevel::WARN:  return "WARN";
                case LogLevel::ERROR: return "ERROR";
                case LogLevel::FATAL: return "FATAL";
                default:              return "UNKNOWN";
            }
        }

    }
}

#endif //ARGUSLOG_LOGLEVEL_H