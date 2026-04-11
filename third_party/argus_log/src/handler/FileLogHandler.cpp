/** 
 * @file FileLogHandler.cpp
 * @author Mnsx_x <xx1527030652@gmail.com>
 * @date 2026/4/11
 */
#include "handler/FileLogHandler.h"

using namespace mnsx::argus;

FileLogHandler::FileLogHandler(const std::string& file_path) {
    // 打开日志文件输出流
    file_stream_.open(file_path, std::ios::out | std::ios::app);
    if (file_stream_.is_open() == false) {
        throw std::runtime_error("[ Mnsx-ArgusLog ] Failed to open file: " + file_path);
    }
}

FileLogHandler::~FileLogHandler() {
    if (file_stream_.is_open() == true) {
        file_stream_.close();
    }
}

void FileLogHandler::handle(const LogEvent &event) {
    std::string log_line = formatEvent(event);

    {
        std::lock_guard<std::mutex> lock(file_mutex_);
        if (file_stream_.is_open() == true) {
            // 不能使用std::endl，因为endl会刷新磁盘，每次都提交的话，会增大开销
            file_stream_ << log_line << "\n";
        }
    }
}
