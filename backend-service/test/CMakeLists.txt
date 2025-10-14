# 测试程序CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(KnotDiagnosticTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖库
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# 查找OpenSSL (cpp-httplib需要)
find_package(OpenSSL REQUIRED)

# MySQL查找
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
    PATHS /usr/include/mysql /usr/local/include/mysql)
find_library(MYSQL_LIBRARY NAMES mysqlclient
    PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)

# 检查是否找到所有库
if(NOT MYSQL_LIBRARY)
    message(FATAL_ERROR "Could not find MySQL library")
endif()

message(STATUS "MySQL library: ${MYSQL_LIBRARY}")
message(STATUS "JsonCpp libraries: ${JSONCPP_LIBRARIES}")
message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")

# 包含目录
include_directories(
    ${JSONCPP_INCLUDE_DIRS}
    ${MYSQL_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
    ../third_party  # 包含cpp-httplib
)

# 编译测试程序
add_executable(comprehensive_diagnostic_test comprehensive_diagnostic_test.cpp)

# 链接库
target_link_libraries(comprehensive_diagnostic_test
    ${JSONCPP_LIBRARIES}
    ${MYSQL_LIBRARY}
    ${OPENSSL_LIBRARIES}
    pthread  # cpp-httplib需要
)

