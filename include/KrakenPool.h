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
#include <mutex>
#include <queue>
#include <thread>
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
     * @brief 提交一个无参数、无返回值的任务到线程池 TODO 第一阶段基础版
     * @param task 一个可调用的对象，包含在std::function中
     */
    void enqueue(std::function<void()> task);

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

#endif //MNSX_KRAKENPOOL_KRAKENPOOL_H