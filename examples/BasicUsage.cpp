/** 
 * @file basic_usage.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/4
 * @description KrakenPool基本使用
 */

#include <iostream>
#include <mutex>
#include <string>

#include "KrakenPool.h"

// 防止多线程调用std::cout导致输出错乱
std::mutex cout_mutex;
void safePrint(const std::string& msg) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << msg << std::endl;
}

int main() {
    safePrint("[主线程] 启动 KrakenPool...");

    {
        // 初始化线程池设置4个线程
        KrakenPool pool(4);

        // 模拟放入8个耗时任务
        for (int i = 1; i <= 8; ++i) {
            pool.enqueue([i]() {
                std::string msg = "\t-> 工作线程[" +
                    std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()) % 10000) +
                        "]正在执行任务" + std::to_string(i) + "...";
                safePrint(msg);

                // 模拟耗时任务 200ms
                std::this_thread::sleep_for(std::chrono::microseconds(200));
            });
        }

        safePrint("[主线程] 线程任务已经加入任务队列，线程池停止...");
    } // 线程池声明周期结束，执行析构函数

    safePrint("[主线程] KrakenPool已经安全停止，程序正常退出");
    return 0;
}