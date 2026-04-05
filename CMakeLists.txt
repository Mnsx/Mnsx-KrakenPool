cmake_minimum_required(VERSION 3.10)
project(Mnsx_KrakenPool LANGUAGES CXX)

# 全局设置
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexec-charset=UTF-8")

# 定义核心库
add_library(KrakenPool STATIC
        src/KrakenPool.cpp
)

# 配置库的头文件路径
include_directories(KrakenPool PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 定义实例程序
add_executable(KrakenDemo examples/BasicUsage.cpp)

# 建立连接关系
target_link_libraries(KrakenDemo PRIVATE KrakenPool)