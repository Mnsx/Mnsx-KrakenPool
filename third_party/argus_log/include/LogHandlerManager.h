/** 
 * @file LogHandlerManager.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/7
 * @description 日志处理器管理者
 */
#ifndef ARGUSLOG_LOGHANDLERMANAGER_H
#define ARGUSLOG_LOGHANDLERMANAGER_H

#include "handler/ILogHandler.h"

#include <memory>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace mnsx {
    namespace argus {

        class LogHandlerManager {
        public:
            /**
             * @brief 单例模式获取唯一实例
             * @return
             */
            static LogHandlerManager& getInstance();

            /**
             * @brief delete
             */
            LogHandlerManager(const LogHandlerManager&) = delete;
            /**
             * @brief delete
             */
            LogHandlerManager& operator=(const LogHandlerManager&) = delete;

            /**
             * @brief 添加处理器
             * @param handler
             */
            void addHandler(const std::shared_ptr<ILogHandler>& handler);

            /**
             * @brief 将日志事件添加到事件队列中等待工作线程执行
             * @param log_event
             */
            void pushEvent(const LogEvent& log_event);

        private:
            /**
             * @brief 私有构造函数
             */
            LogHandlerManager();

            /**
             * @brief 私有析构函数
             */
            ~LogHandlerManager();

            /**
             * @brief 工作线程分发事件给handler处理
             * @param log_event
             */
            void postEvent(const LogEvent& log_event);

            std::vector<std::shared_ptr<ILogHandler>> handlers_; // 存放日志处理器的函数

            std::thread worker_;

            std::queue<LogEvent> event_queue_; // 日志事件存放的队列
            std::mutex queue_mutex_; // 保护日志存放队列的互斥锁

            std::condition_variable wake_up_; // 条件变量，当有任务时唤醒工作线程

            std::atomic<bool> is_stop_{true}; // 原子变量表示状态
        };

        inline LogHandlerManager& LogHandlerManager::getInstance() {
            static LogHandlerManager logger;
            return logger;
        }
    }
}

#endif //ARGUSLOG_LOGHANDLERMANAGER_H