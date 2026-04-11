/** 
 * @file BasicUsage.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/4
 * @description KrakenPool基本使用
 */

#include <iostream>
#include <chrono>
#include <string>
#include <vector>

#include "KrakenPool.h"
#include "Logger.h"
#include "LogLevel.h"

using namespace mnsx::kraken;

// 防止多线程调用std::cout导致输出错乱
std::mutex cout_mutex;
void safePrint(const std::string& msg) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << msg << std::endl;
}

// 模拟一个耗时的待参数和返回值的计算任务
int heavyComputation(int a, int b) {

    std::string msg = "[" + std::to_string(
                 std::hash<std::thread::id>{}(std::this_thread::get_id()) % 10000) +
                     "] 正在计算 " + std::to_string(a) + " + " + std::to_string(b);

    safePrint(msg);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    return a * b;
}

int main() {

    std::string curThreadId =std::to_string(
                 std::hash<std::thread::id>{}(std::this_thread::get_id()) % 10000);

    safePrint("[主线程 " + curThreadId + "] 启动 KrakenPool...");

    {
        // 初始化线程池设置3个线程
        // KrakenPool pool(3);
        // 测试拒绝策略
        KrakenPool pool(1, 2, RejectPolicies::callerRuns);
        // 用一个vector手机任务的future
        std::vector<std::future<int>> results;

        for (int i = 1; i <= 8; ++i) {
            results.emplace_back(pool.enqueue(heavyComputation, i, i * 10));
        }

        auto string_future = pool.enqueue_after(std::chrono::milliseconds(5000), [](const std::string& name) {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            return "Hello, " + name + "!";
        }, "Mnsx_x");

        safePrint("[主线程] 获取任务的异步结果...");
        LOG_DEBUG << "hhh";

        for (size_t i = 0; i < results.size(); ++i) {
            int res = results[i].get();
            safePrint("\t-> 第 " + std::to_string(i + 1) + " 个计算任务的结果是 " + std::to_string(res));
        }

        safePrint("\n\t-> 其他类型任务执行结果: " + string_future.get());

        safePrint("\n[主线程] 线程池所有结果接收完毕");
    } // 线程池声明周期结束，执行析构函数

    safePrint("[主线程] KrakenPool已经安全停止，程序正常退出");
    return 0;
}