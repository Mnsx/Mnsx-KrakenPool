/** 
 * @file Logger.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/11
 * @description 日志系统的调用者
 */
#ifndef ARGUSLOG_LOGGER_H
#define ARGUSLOG_LOGGER_H

#include "LogLevel.h"

#include <sstream>

namespace mnsx {
    namespace argus {

        class Logger {
        public:
            /**
             * @brief 构造函数
             * @param level
             * @param file_path
             * @param line
             */
            Logger(LogLevel level, const char* file_path, int line);
            /**
             * @brief 析构函数
             */
            ~Logger();

            /**
             * @brief 返回字符串流，方便链式调用
             * @return
             */
            std::stringstream& stream();

        private:
            LogLevel level_; // 日志级别
            const char* file_path_; // 文件路径
            int line_; // 发出日志的行号
            std::stringstream stream_; // 日志内容字符串流
        };

        inline std::stringstream& Logger::stream() {
            return stream_;
        }

    }
}

#define LOG_DEBUG ::mnsx::argus::Logger(::mnsx::argus::LogLevel::DEBUG, __FILE__, __LINE__).stream()
#define LOG_INFO  ::mnsx::argus::Logger(::mnsx::argus::LogLevel::INFO,  __FILE__, __LINE__).stream()
#define LOG_ERROR ::mnsx::argus::Logger(::mnsx::argus::LogLevel::ERROR, __FILE__, __LINE__).stream()
#define LOG_WARN  ::mnsx::argus::Logger(::mnsx::argus::LogLevel::WARN,  __FILE__, __LINE__).stream()

#endif //ARGUSLOG_LOGGER_H