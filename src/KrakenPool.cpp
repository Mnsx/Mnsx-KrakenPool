/** 
 * @file KrakenPool.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/4
 * @description KrakenPool核心类接口定义
 */

#include "KrakenPool.h"

#include <utility>

// 构造函数
KrakenPool::KrakenPool(size_t num_threads, size_t max_queue_size, RejectHandler reject_handler) :
    max_queue_size_(max_queue_size), reject_handler_(std::move(reject_handler)), tasks_(max_queue_size) {
    for (auto i = 0; i < num_threads; ++i) {
        // 使用emplace_back直接在vector的内部构造线程
        workers_.emplace_back([this]() {
            // 死循环，让所有线程待命
            while (true) {
                DelayedTask delayed_task(std::chrono::steady_clock::now(), nullptr);

                // 使用代码块缩小锁的生命周期
                {
                    std::unique_lock<std::mutex> lock(this->pool_mutex_);

                    while (true) {

                        // 如果线程池已经停止，线程则退出循环
                        if (this->stop_.load() && this->tasks_.empty()) {
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
                            this->tasks_.tryPop(delayed_task);
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
                    delayed_task.task_();
                }
            }
        });
    }
}

// 析构函数
KrakenPool::~KrakenPool() {

    // 原子写入
    this->stop_.store(true);

    // 唤醒所有队列退出循环
    this->condition_.notify_all();

    // 等待所有线程执行完工作
    for (std::thread& worker : this->workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}
