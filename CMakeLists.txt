cmake_minimum_required(VERSION 3.10)
project(Mnsx_KrakenPool LANGUAGES CXX)

# ==========================================
# 全局设置
# ==========================================
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 引入线程
find_package(Threads REQUIRED)

# ==========================================
# 引入外部基建 (Mnsx-ArgusLog)
# ==========================================
# 执行argus的cmake
add_subdirectory(third_party/argus_log)

# ==========================================
# 定义核心库 (KrakenPool)
# ==========================================
add_library(KrakenPool STATIC
        src/KrakenPool.cpp
)

target_include_directories(KrakenPool PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 限定编译选项，防止污染
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(KrakenPool PRIVATE -fexec-charset=UTF-8)
endif()

# 绑定多线程和argus
target_link_libraries(KrakenPool PUBLIC
        Threads::Threads
        ArgusLog
)

# ==========================================
# 定义实例程序 (Demo)
# ==========================================
add_executable(KrakenDemo examples/BasicUsage.cpp)

target_link_libraries(KrakenDemo PRIVATE KrakenPool)