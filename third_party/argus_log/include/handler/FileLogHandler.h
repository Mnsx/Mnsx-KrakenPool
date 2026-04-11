/** 
 * @file FileLogHandler.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/11
 * @description 将日志信息输出到文件的处理器
 */
#ifndef ARGUSLOG_FILELOGHANDLER_H
#define ARGUSLOG_FILELOGHANDLER_H

#include "handler/ILogHandler.h"

#include <mutex>
#include <fstream>

namespace mnsx {
    namespace argus {

        class FileLogHandler : public ILogHandler {
        public:
            /**
             * @brief 构造函数
             * @param file_path
             */
            explicit FileLogHandler(const std::string& file_path);

            /**
             * @brief 析构函数
             */
            ~FileLogHandler() override;

            /**
             * @brief 将日志事件处理后写入文件中
             * @param event
             */
            void handle(const LogEvent &event) override;

        private:
            std::ofstream file_stream_; // 目标文件的输入流
            std::mutex file_mutex_; // 防止多线程写入，保护文件输入流的互斥锁
        };

    }
}

#endif //ARGUSLOG_FILELOGHANDLER_H