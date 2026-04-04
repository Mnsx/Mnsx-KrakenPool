cmake_minimum_required(VERSION 4.3)
project(Mnsx_KrakenPool)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexec-charset=UTF-8")

include_directories(include)

add_library(KrakenPool STATIC
        src/KrakenPool.cpp
        examples/BasicUsage.cpp)

# 示例代码运行
add_executable(KrakenDemo examples/BasicUsage.cpp
        include/KrakenPool.h)
target_link_libraries(KrakenDemo KrakenPool)