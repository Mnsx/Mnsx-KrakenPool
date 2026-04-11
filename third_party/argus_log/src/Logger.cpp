/** 
 * @file Logger.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/11
 */
#include "Logger.h"
#include "LogEvent.h"
#include "LogHandlerManager.h"

#include <cstring>

using namespace mnsx::argus;

Logger::Logger(LogLevel level, const char* file_path, int line) :
    level_(level), line_(line) {
    // 判断是否为项目根目录下的文件，如果是就是用相对路径
#ifdef PROJECT_ROOT_DIR
    const char *root = PROJECT_ROOT_DIR;
    size_t root_len = std::strlen(root);
    if (std::strncmp(file_path, root, root_len) == 0) {
        file_path_ = file_path + root_len;
    } else {
        file_path_ = file_path;
    }
#else
    file_path_ = file_path;
#endif
}

Logger::~Logger() {
    // 文本内容
    std::string content = stream_.str();
    // 包装日志
    LogEvent event(level_, file_path_, line_, std::move(content));
    // 通知manager进行处理
    LogHandlerManager::getInstance().pushEvent(event);
}
