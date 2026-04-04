/** 
 * @file KrakenPool.h
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/4
 * @description KrakenPool核心类接口声明
 */
#ifndef MNSX_KRAKENPOOL_KRAKENPOOL_H
#define MNSX_KRAKENPOOL_KRAKENPOOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

class KrakenPool {
public:
    /**
     * @brief 构造并启动线程池
     * @param num_threads 初始化的线程数量，默认值为当前系统支持的CPU核心数量
     */
    explicit KrakenPool(size_t num_threads = std::thread::hardware_concurrency());

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

    /**
     * @brief 提交一个无参数、无返回值的任务到线程池（第一阶段基础版）
     * @param task 一个可调用的对象，包含在std::function中
     */
    // void enqueue(std::function<void()> task);

    // 禁止拷贝构造和赋值操作
    KrakenPool(const KrakenPool&) = delete;
    KrakenPool& operator=(const KrakenPool&) = delete;

private:
    std::vector<std::thread> workers_;               // 工作线程组
    std::queue<std::function<void()>> tasks_;        // 任务队列

    std::mutex queue_mutex_;                         // 互斥锁
    std::condition_variable condition_;              // 条件变量
    bool stop_;                                      // 停止标志位
};

template<typename F, typename... Args>
auto KrakenPool::enqueue(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
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

    {
        std::unique_lock<std::mutex> lock(this->queue_mutex_);

        // 线程池关闭
        if (this->stop_) {
            throw std::runtime_error("KrakenPool has been stopped");
        }

        // 容器存放的是std::function<void>，task通过std::bind擦除参数列表，通过Lambda包装隐藏返回值
        this->tasks_.push([task]() {
            (*task)();
        });
    }

    // 唤醒工作线程
    this->condition_.notify_one();

    return res;
}

#endif //MNSX_KRAKENPOOL_KRAKENPOOL_H