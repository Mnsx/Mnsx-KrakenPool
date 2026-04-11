/** 
 * @file ConsoleLogHandler.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/11
 * @description 将日志事件输出到控制台
 */
#ifndef MNSX_ARGUSLOG_CONSOLELOGHANDLER_H
#define MNSX_ARGUSLOG_CONSOLELOGHANDLER_H
#include "ILogHandler.h"

#include <mutex>

namespace mnsx {
    namespace argus {

        class ConsoleLogHandler : public ILogHandler {
        public:
            /**
             * @brief 构造函数
             */
            ConsoleLogHandler() = default;
            /**
             * @brief 析构函数
             */
            ~ConsoleLogHandler() override = default;

            /**
             * @brief 将日志事件写入标准输出
             * @param event
             */
            void handle(const LogEvent &event) override;

        private:
            std::mutex console_mutex_; // 互斥锁
        };

    }
}

#endif //MNSX_ARGUSLOG_CONSOLELOGHANDLER_H