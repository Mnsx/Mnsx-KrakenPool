/** 
 * @file ILogHandler.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/11
 * @description 抽象类，日志消息处理器
 */
#ifndef ARGUSLOG_ILOGHANDLER_H
#define ARGUSLOG_ILOGHANDLER_H

#include "LogEvent.h"

namespace mnsx {
    namespace argus {
        /**
         * @brief 静态辅助函数，用来根据time_t类型返回tm结构体，根据当前系统的不同调用不同的函数
         * @param time_ptr
         * @param result
         */
        inline static void safe_localtime(const time_t* time_ptr, struct tm *result) {
#if defined(_WIN32) || defined(_WIN64)
            localtime_s(result, time_ptr); // 调用win的localtime_s
#else
            localtime_r(time_ptr, result); // 调用linux的localtime_r
#endif
        }

        class ILogHandler {
        public:
            /**
             * @brief 基类析构函数
             */
            virtual ~ILogHandler() = default;

            /**
             * @brief 虚函数，处理日志事件
             * @param event
             */
            virtual void handle(const LogEvent& event) = 0;

            /**
             * @brief 默认的解析日志事件的实现，子类可以重写
             * @param event
             * @return
             */
            virtual std::string formatEvent(const LogEvent &event);

        private:
            /**
             * @brief 在字符串右边添加空格
             * @param s
             * @param width 添加空格后所需的总长度
             * @return
             */
            std::string addRightSpace(const std::string& str, size_t width);
            /**
             * @brief 在字符串左边添加空格
             * @param s
             * @param width 添加空格后所需的总长度
             * @return
             */
            std::string addLeftSpace(const std::string& str, size_t width);
        };

        inline std::string ILogHandler::addRightSpace(const std::string &str, size_t width) {
            return str.length() < width ? str + std::string(width - str.length(), ' ') : str;
        }

        inline std::string ILogHandler::addLeftSpace(const std::string &str, size_t width) {
            return str.length() < width ? std::string(width - str.length(), ' ') + str : str;
        }
    }
}

#endif //ARGUSLOG_ILOGHANDLER_H