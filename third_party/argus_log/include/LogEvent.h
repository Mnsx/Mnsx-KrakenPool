/** 
 * @file LogEvent.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/7
 * @description 日志事件结构体，封装需要输出的日志信息
 */
#ifndef ARGUSLOG_LOGEVENT_H
#define ARGUSLOG_LOGEVENT_H

#include "LogLevel.h"

#include <thread>

namespace mnsx {
    namespace argus {

        struct LogEvent {
            LogLevel level; // 日志级别
            std::string filename; // 发出日志的文件路径
            int line_number; // 发出日志的文件行数
            std::thread::id thread_id; // 发出日志的线程编号
            std::chrono::system_clock::time_point timestamp; // 发出日志的时间戳
            std::string content; // 日志内容

            /**
             * @brief 空参构造函数
             */
            LogEvent() = default;

            /**
             * @brief 构造函数
             */
            LogEvent(LogLevel level, std::string file, int line, std::string msg)
                : level(level), filename(std::move(file)), line_number(line),
                thread_id(std::this_thread::get_id()),
                timestamp(std::chrono::system_clock::now()),
                content(std::move(msg)) {}
        };

    }
}

#endif //ARGUSLOG_LOGEVENT_H