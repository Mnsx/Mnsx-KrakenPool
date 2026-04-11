/** 
 * @file KrakenPool.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/4
 * @description KrakenPool核心类接口声明
 */
#ifndef MNSX_KRAKENPOOL_KRAKENPOOL_H
#define MNSX_KRAKENPOOL_KRAKENPOOL_H

#include "SafeQueue.h"

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

#include "Logger.h"

namespace mnsx {
    namespace kraken {

        using Task = std::function<void()>;
        using DelayedTime = std::chrono::steady_clock::time_point;
        struct DelayedTask {
            // 任务计划执行的绝对时间点
            DelayedTime execute_time_;
            // 类型擦除后的任务实体
            Task task_;
            // 构造函数
            DelayedTask(DelayedTime time, Task&& task) : execute_time_(time), task_(std::move(task)) {}
            // 重载比较符号，给priority_queue提供排序规则
            bool operator<(const DelayedTask& other) const {
                return this->execute_time_ > other.execute_time_;
            }
        };

        using RejectHandler = std::function<void(SafeQueue<DelayedTask>&, Task&)>;
        namespace RejectPolicies {
            // 默认，直接抛出异常
            inline void abort(SafeQueue<DelayedTask>& safe_queue,Task& task) {
                LOG_ERROR << "Task queue is full. Aborting and throwing exception.";
                throw std::runtime_error("The task queue of KrakenPool is full");
            }

            // 由调用入队操作的线程自己执行任务
            inline void callerRuns(SafeQueue<DelayedTask>& safe_queue,Task& task) {
                if (task != nullptr) {
                    LOG_WARN << "Task queue is full. Executing task in caller thread (CallerRunsPolicy).";
                    task();
                }
            }

            // 静默丢弃，如果调用future.get()会抛出Broken promise
            inline void discard(SafeQueue<DelayedTask>& safe_queue,Task& task) {
                LOG_WARN << "Task queue is full. Task discarded silently (DiscardPolicy).";
                // 不错任何处理
            }

            // TODO 阻塞等待
            inline void blockedWait(SafeQueue<DelayedTask>& safe_queue,Task& task) {
            }
        }

        class KrakenPool {
        public:
            /**
             * @brief 构造并启动线程池
             * @param num_threads 初始化的线程数量，默认值为当前系统支持的CPU核心数量
             * @param max_queue_size 任务队列最大长度，默认为DEFAULT_MAX_QUEUE_SIZE
             * @param reject_handler 线程池拒绝处理器，默认为RejectPolicies::Abort，用户可以自定义
             */
            explicit KrakenPool(size_t num_threads = std::thread::hardware_concurrency(),
                size_t max_queue_size = DEFAULT_MAX_QUEUE_SIZE, RejectHandler reject_handler = RejectPolicies::abort);

            /**
             * @brief 析构函数，安全关闭线程池
             */
            ~KrakenPool();

            /**
             * @brief 提交任何带有参数和返回值的异步任务
             * @tparam F 可调用对象的类型
             * @tparam Args 参数类型包
             * @param f 要执行的函数
             * @param args 传递给函数的参数
             * @return std::future通过这个类可以异步获取执行的结果
             */
            template <typename F, typename... Args>
            auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

            template <typename Req, typename Period, typename F, typename... Args>
            auto enqueue_after(const std::chrono::duration<Req, Period>& delay, F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

            // 禁止拷贝构造和赋值操作
            KrakenPool(const KrakenPool&) = delete;
            KrakenPool& operator=(const KrakenPool&) = delete;

        private:
            std::vector<std::thread> workers_; // 工作线程组
            SafeQueue<DelayedTask> tasks_; // 线程安全任务队列

            std::mutex pool_mutex_; // 互斥锁
            std::condition_variable condition_; // 条件变量
            std::atomic<bool> stop_{false}; // 停止标志位

            RejectHandler reject_handler_; // 拒绝策略

            size_t max_queue_size_; // 任务队列最大长度
            constexpr const static size_t DEFAULT_MAX_QUEUE_SIZE = 128; // 任务队列默认最大长度，以IO密集型设置

            template <typename F, typename... Args>
            auto enqueue_at(DelayedTime execute_time, F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;
        };

        template<typename F, typename... Args>
        auto KrakenPool::enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {

            return this->enqueue_at(
                std::chrono::steady_clock::now(),
                std::forward<F>(f),
                std::forward<Args>(args)...
            );
        }

        template<typename Req, typename Period, typename F, typename... Args>
        auto KrakenPool::enqueue_after(const std::chrono::duration<Req, Period> &delay, F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type> {

            return this->enqueue_at(
                std::chrono::steady_clock::now() + delay,
                std::forward<F>(f),
                std::forward<Args>(args)...
            );
        }

        template<typename F, typename... Args>
        auto KrakenPool::enqueue_at(DelayedTime execute_time, F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {

            // 通过std::result_of推导返回值的类型
            using return_type = typename std::result_of<F(Args...)>::type;

            // 将可调用对象包装成packaged_task，用于返回future，异步获取结果
            // 使用std::share_ptr是因为需要将所有任务装进vector中，而容器中的类型是std::function<void()>
            // 而C++11强制要求能够装进std::function<void()>的可调对象必须是可拷贝的
            // 而std::packaged_task因为其中复杂的实现，拷贝构造被delete了，所以需要使用智能指针包装
            auto task = std::make_shared<std::packaged_task<return_type()>>(
                            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                        );

            // 获取future，提供异步获取任务结果
            std::future<return_type> res = task->get_future();

            // 线程池关闭
            if (this->stop_.load()) {
                LOG_ERROR << "Attempted to enqueue task to a stopped pool.";
                throw std::runtime_error("KrakenPool has been stopped");
            }

            // 容器存放的是std::function<void>，task通过std::bind擦除参数列表，通过Lambda包装隐藏返回值
            Task wrappered_task = [task]() {
                (*task)();
            };

            bool is_pushed = false;
            {
                // 加锁，防止信号丢失
                std::unique_lock<std::mutex> lock(pool_mutex_);

                // 必须保证，队列中有空位才能使用move
                if (this->tasks_.size() < this->max_queue_size_) {

                    is_pushed = this->tasks_.emplace(execute_time, std::move(wrappered_task));
                }
            }

            // 如果没有加入成功代表队列已经满了
            if (is_pushed == false) {
                LOG_WARN << "Task enqueue failed. Queue reached max capacity: " << this->max_queue_size_;
                // 执行拒绝策略
                this->reject_handler_(this->tasks_, wrappered_task);
                return res;
            }

            // 唤醒工作线程
            this->condition_.notify_one();

            LOG_DEBUG << "Task successfully scheduled. Current queue size: " << this->tasks_.size();

            return res;
        }

    }
}

#endif //MNSX_KRAKENPOOL_KRAKENPOOL_H