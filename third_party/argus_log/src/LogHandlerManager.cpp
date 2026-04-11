/** 
 * @file LogHandlerManager.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/7
 */
#include "LogHandlerManager.h"
#include "LogEvent.h"

#include <cstring>

#include "handler/ConsoleLogHandler.h"
#include "handler/FileLogHandler.h"

namespace mnsx {
    namespace argus {
        
        void LogHandlerManager::addHandler(const std::shared_ptr<ILogHandler>& handler) {
            if (handler != nullptr) {
                handlers_.push_back(handler);
            }
        }

        void LogHandlerManager::pushEvent(const LogEvent &log_event) {
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);

                event_queue_.push(log_event);
            }
            wake_up_.notify_one(); // 唤醒工作线程
        }

        LogHandlerManager::LogHandlerManager() {
            // 修改状态
            is_stop_.store(false);

            // 添加默认处理器
            addHandler(std::make_shared<ConsoleLogHandler>());

            // 创建工作线程
            worker_ = std::thread([this]() {
                while (true) {
                    LogEvent current_event(LogLevel::INFO, "", 0, "");
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        // 判断如果manager还在运作，并且队列为空，那么阻塞等待
                       wake_up_.wait(lock, [this]() {
                            return !event_queue_.empty() || is_stop_.load();
                        });
                        // 如果队列为空，且manager已经挂了，就退出
                        if (event_queue_.empty() == true && is_stop_.load() == true) {
                            break;
                        }

                        current_event = std::move(event_queue_.front());
                        event_queue_.pop();
                    }

                    // 执行分发处理
                    postEvent(current_event);
                }
            });
        }

        LogHandlerManager::~LogHandlerManager() {
            is_stop_.store(true);
            // 唤醒工作线程
            wake_up_.notify_one();
            // 必须等待所有工作线程执行结束，才退出
            if (worker_.joinable()) {
                worker_.join();
            }
        }

        void LogHandlerManager::postEvent(const LogEvent &log_event) {
            if (handlers_.empty() == true) {
                return;
            }

            // 进行处理
            for (auto& handler : handlers_) {
                handler->handle(log_event);
            }
        }

    }
}
