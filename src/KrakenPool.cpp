/** 
 * @file KrakenPool.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/4
 */
#include "KrakenPool.h"

#include <utility>

using namespace mnsx::kraken;

// 构造函数
KrakenPool::KrakenPool(size_t num_threads, size_t max_queue_size, RejectHandler reject_handler) :
    max_queue_size_(max_queue_size), reject_handler_(std::move(reject_handler)), tasks_(max_queue_size) {

    LOG_INFO << "[ Mnsx-KrakenPool ] Initializing thread pool with " << num_threads << " workers.";

    for (auto i = 0; i < num_threads; ++i) {
        // 使用emplace_back直接在vector的内部构造线程
        workers_.emplace_back([this, i]() {
            LOG_DEBUG << "[ Mnsx-KrakenPool ] Worker-" << i << " started.";
            // 死循环，让所有线程待命
            while (true) {
                DelayedTask delayed_task(std::chrono::steady_clock::now(), nullptr);

                // 使用代码块缩小锁的生命周期
                {
                    std::unique_lock<std::mutex> lock(this->pool_mutex_);

                    while (true) {

                        // 如果线程池已经停止，线程则退出循环
                        if (this->stop_.load() && this->tasks_.empty()) {
                            LOG_INFO << "[ Mnsx-KrakenPool ] Worker-" << i << " is exiting gracefully.";
                            return;
                        }

                        // 任务队列为空，等待
                        if (this->tasks_.empty()) {
                            this->condition_.wait(lock);
                            continue;
                        }

                        // 任务队列有任务，提取检测时间
                        DelayedTime execute_time;
                        {
                            // 使用队列内部锁保证数据安全
                            std::lock_guard<std::mutex> pq_lock(this->tasks_.getMutex());
                            auto& pq = this->tasks_.getUnderlyingQueue();
                            execute_time = pq.top().execute_time_;
                        }
                        // 获取现在的时间
                        auto now = std::chrono::steady_clock::now();

                        if (now >= execute_time || this->stop_.load()) {
                            // 任务时间到，执行任务
                            if (this->tasks_.tryPop(delayed_task) == true) {
                                LOG_DEBUG << "[ Mnsx-KrakenPool ] Worker-" << i << " picked up a task.";
                            }
                            break;
                        } else {
                            // 时间未到重新等待
                            this->condition_.wait_until(lock, execute_time);
                            continue;
                        }
                    }
                } // 锁生命周期结束，自动释放，线程执行任务时不会阻塞

                // 执行任务
                if (delayed_task.task_ != nullptr) {
                    try {
                        delayed_task.task_();
                    } catch (const std::exception& e) {
                        LOG_ERROR << "[ Mnsx-KrakenPool ] Worker-" << i << " caught unhandled exception: " << e.what();
                    } catch (...) {
                        LOG_ERROR << "[ Mnsx-KrakenPool ] Worker-" << i << " caught unknown fatal exception!";
                    }
                }
            }
        });
    }
}

// 析构函数
KrakenPool::~KrakenPool() {

    LOG_INFO << "[ Mnsx-KrakenPool ] Starting shutdown sequence...";

    // 原子写入
    this->stop_.store(true);

    // 唤醒所有队列退出循环
    this->condition_.notify_all();

    // 等待所有线程执行完工作
    for (int i = 0; i < workers_.size(); ++i) {
        if (workers_[i].joinable()) {
            workers_[i].join();
            LOG_DEBUG << "[ Mnsx-KrakenPool ] Worker-" << i << " joined.";
        }
    }

    LOG_INFO << "[ Mnsx-KrakenPool ] Pool has been completely destroyed.";
}
