cmake_minimum_required(VERSION 3.10)
project(Mnsx-ArgusLog LANGUAGES CXX)

# ==========================================
# 全局设置
# ==========================================
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 添加thread
find_package(Threads REQUIRED)

# ==========================================
# 定义 ArgusLog 核心静态库
# ==========================================
add_library(ArgusLog STATIC
        src/Logger.cpp
        src/handler/ILogHandler.cpp
        src/handler/FileLogHandler.cpp
        src/handler/ConsoleLogHandler.cpp
        src/LogHandlerManager.cpp
)

target_include_directories(ArgusLog PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(ArgusLog PRIVATE -fexec-charset=UTF-8)
endif()

target_compile_definitions(ArgusLog PRIVATE PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}/")

# 连接多线程库
target_link_libraries(ArgusLog PRIVATE Threads::Threads)