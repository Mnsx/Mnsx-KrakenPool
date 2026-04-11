/** 
 * @file ILogHandler.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/11
 */
#include "handler/FileLogHandler.h"

using namespace mnsx::argus;

std::string ILogHandler::formatEvent(const LogEvent &event) {
    // 处理事件数据
    auto timestamp_t = std::chrono::system_clock::to_time_t(event.timestamp);
    struct tm local_time{};
    safe_localtime(&timestamp_t, &local_time);
    char format_time[100];
    std::strftime(format_time, sizeof(format_time), "%Y-%m-%d %H:%M:%S", &local_time);

    // 处理线程编号
    std::string cur_thread_id = std::to_string(std::hash<std::thread::id>{}(event.thread_id) % 10000000);

    // 组装数据，填充对齐
    std::string res = "[ " + static_cast<std::string>(format_time) + " ] ";
    res.append("[ " + addRightSpace(levelToString(event.level), 5) + " ] ");
    res.append("[ " + addRightSpace(cur_thread_id, 8) + " ] ");
    res.append("[ " + addRightSpace(event.filename + ":" + std::to_string(event.line_number), 30) + " ] ");
    res.append(event.content);
    return res;
}
