/** 
 * @file KrakenPool.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/4
 * @description KrakenPool核心类接口定义
 */

#include "KrakenPool.h"

// 构造函数
KrakenPool::KrakenPool(size_t num_threads) : stop_(false) {
    for (int i = 0; i < num_threads; ++i) {
        // 使用emplace_back直接在vector的内部构造线程
        workers_.emplace_back([this]() {
            // 死循环，让所有线程待命
            while (true) {
                std::function<void()> task;

                // 使用代码块缩小锁的生命周期
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex_);

                    // 等待条件变量：线程池停止或者任务队列不为空
                    this->condition_.wait(lock, [this]() {
                        return this->stop_ || !this->tasks_.empty();
                    });

                    // 如果线程池已经停止，线程则退出循环
                    if (this->stop_ && this->tasks_.empty()) {
                        return;
                    }

                    // 从任务队列中取出任务，将function转换为右值引用，使用移动操作提高效率
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                } // 锁生命周期结束，自动释放，线程执行任务时不会阻塞

                // 执行任务
                task();
            }
        });
    }
}

// 析构函数
KrakenPool::~KrakenPool() {

    {
        // 加锁设置线程池的状态
        std::unique_lock<std::mutex> lock(this->queue_mutex_);
        this->stop_ = true;
    }

    // 唤醒所有队列退出循环
    this->condition_.notify_all();

    // 等待所有线程执行完工作
    for (std::thread& worker : this->workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

// void KrakenPool::enqueue(std::function<void()> task) {
//
//     {
//         // 加锁添加任务到任务队列中
//         std::unique_lock<std::mutex> lock(this->queue_mutex_);
//
//         // 检查线程池是否已经关闭，如果关闭则不将新的任务添加到任务队列中
//         if (this->stop_) {
//             throw std::runtime_error("KrakenPool is stopped, cannot enqueue new tasks!");
//         }
//
//         // 将任务推入队列，使用std::move提升性能
//         this->tasks_.emplace(std::move(task));
//     }
//
//     // 唤醒工作线程处理任务
//     this->condition_.notify_one();
// }
