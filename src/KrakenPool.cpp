/** 
 * @file KrakenPool.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/4
 * @description KrakenPool核心类接口定义
 */

#include "KrakenPool.h"

// 构造函数
KrakenPool::KrakenPool(size_t num_threads, size_t max_queue_size, const RejectHandler& reject_handler) :
    max_queue_size_(max_queue_size), reject_handler_(reject_handler), tasks_(max_queue_size) {
    for (auto i = 0; i < num_threads; ++i) {
        // 使用emplace_back直接在vector的内部构造线程
        workers_.emplace_back([this]() {
            // 死循环，让所有线程待命
            while (true) {
                Task task;

                // 使用代码块缩小锁的生命周期
                {
                    std::unique_lock<std::mutex> lock(this->pool_mutex_);

                    // 等待条件变量：线程池停止或者任务队列不为空
                    this->condition_.wait(lock, [this]() {
                        return this->stop_.load() || !this->tasks_.empty();
                    });

                    // 如果线程池已经停止，线程则退出循环
                    if (this->stop_.load() && this->tasks_.empty()) {
                        return;
                    }

                    // 从任务队列中取出任务，将function转换为右值引用，使用移动操作提高效率
                    this->tasks_.tryPop(task);
                } // 锁生命周期结束，自动释放，线程执行任务时不会阻塞

                // 执行任务
                if (task != nullptr) {
                    task();
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
