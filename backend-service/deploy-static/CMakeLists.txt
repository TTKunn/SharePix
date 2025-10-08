cmake_minimum_required(VERSION 3.16)
project(knot_image_sharing VERSION 1.0.0 LANGUAGES CXX)

# ============================================
# 静态编译配置 - 仅用于生产部署
# 开发请使用上层目录的CMakeLists.txt
# ============================================

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Force Release build
set(CMAKE_BUILD_TYPE Release)

# ============================================
# 强制使用静态库
# ============================================
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)

# 静态链接C++标准库和GCC运行时
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

# Release优化标志
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -s")  # -s用于strip符号表

# ============================================
# 源文件路径（指向上层目录）
# ============================================
set(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Include directories
include_directories(${PROJECT_ROOT}/include)
include_directories(${PROJECT_ROOT}/third_party)
include_directories(${PROJECT_ROOT}/src)

# ============================================
# 强制使用系统库路径（避免Anaconda）
# ============================================
set(SYSTEM_LIB_PATH "/usr/lib/x86_64-linux-gnu")
set(SYSTEM_INCLUDE_PATH "/usr/include")

# Find required packages
find_package(PkgConfig REQUIRED)

# ============================================
# MySQL静态库（强制系统路径）
# ============================================
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
    PATHS ${SYSTEM_INCLUDE_PATH}
    PATH_SUFFIXES mysql
    NO_DEFAULT_PATH)

find_library(MYSQL_LIBRARY
    NAMES libmysqlclient.a
    PATHS ${SYSTEM_LIB_PATH}
    PATH_SUFFIXES mysql
    NO_DEFAULT_PATH)

if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    message(STATUS "✓ Found MySQL static library: ${MYSQL_LIBRARY}")
else()
    message(FATAL_ERROR "MySQL static library not found in ${SYSTEM_LIB_PATH}")
endif()

# ============================================
# JsonCpp（动态库，Ubuntu不提供静态库）
# ============================================
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

if(JSONCPP_FOUND)
    message(STATUS "✓ Found JsonCpp library: ${JSONCPP_LIBRARIES}")
    message(STATUS "  (Note: JsonCpp uses dynamic linking)")
else()
    message(FATAL_ERROR "JsonCpp library not found")
endif()

# ============================================
# OpenSSL静态库（强制系统路径）
# ============================================
find_path(OPENSSL_INCLUDE_DIR openssl/ssl.h
    PATHS ${SYSTEM_INCLUDE_PATH}
    NO_DEFAULT_PATH)

find_library(OPENSSL_SSL_LIBRARY
    NAMES libssl.a
    PATHS ${SYSTEM_LIB_PATH}
    NO_DEFAULT_PATH)

find_library(OPENSSL_CRYPTO_LIBRARY
    NAMES libcrypto.a
    PATHS ${SYSTEM_LIB_PATH}
    NO_DEFAULT_PATH)

if(OPENSSL_INCLUDE_DIR AND OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)
    message(STATUS "✓ Found OpenSSL static libraries:")
    message(STATUS "  SSL: ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "  Crypto: ${OPENSSL_CRYPTO_LIBRARY}")
    set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
else()
    message(FATAL_ERROR "OpenSSL static libraries not found in ${SYSTEM_LIB_PATH}")
endif()

# ============================================
# spdlog（header-only或静态库）
# ============================================
find_path(SPDLOG_INCLUDE_DIR spdlog/spdlog.h
    PATHS ${SYSTEM_INCLUDE_PATH} /usr/local/include
    NO_DEFAULT_PATH)

find_library(SPDLOG_LIBRARY
    NAMES libspdlog.a
    PATHS ${SYSTEM_LIB_PATH} /usr/local/lib
    NO_DEFAULT_PATH)

if(SPDLOG_INCLUDE_DIR)
    message(STATUS "✓ Found spdlog include: ${SPDLOG_INCLUDE_DIR}")
    if(SPDLOG_LIBRARY)
        message(STATUS "✓ Found spdlog static library: ${SPDLOG_LIBRARY}")
    else()
        message(STATUS "ℹ Using spdlog header-only mode")
    endif()
else()
    message(FATAL_ERROR "spdlog not found")
endif()

# Find Threads
find_package(Threads REQUIRED)

# ============================================
# 源文件（从上层目录）
# ============================================
file(GLOB_RECURSE SOURCES
    "${PROJECT_ROOT}/src/*.cpp"
    "${PROJECT_ROOT}/src/*.h"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# ============================================
# 静态链接所有库
# ============================================
target_link_libraries(${PROJECT_NAME}
    ${MYSQL_LIBRARY}
    ${JSONCPP_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
    # 必需的系统库
    dl          # 动态加载
    z           # zlib（MySQL需要）
    zstd        # zstd（MySQL需要）
    rt          # 实时扩展
    resolv      # DNS解析（MySQL需要）
    fmt         # fmt库（spdlog需要）
    # 静态链接pthread
    -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
    # 静态链接C++库
    -static-libgcc
    -static-libstdc++
)

# spdlog是header-only，但依赖fmt库

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${MYSQL_INCLUDE_DIR}
    ${JSONCPP_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR}
    ${PROJECT_ROOT}/third_party/jwt-cpp/include
)

# Compiler definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
    CPPHTTPLIB_LISTEN_BACKLOG=128
    CPPHTTPLIB_THREAD_POOL_COUNT=32
)

# ============================================
# 构建信息
# ============================================
message(STATUS "========================================")
message(STATUS "  Static Build Configuration")
message(STATUS "========================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Project root: ${PROJECT_ROOT}")
message(STATUS "System lib path: ${SYSTEM_LIB_PATH}")
message(STATUS "MySQL: ${MYSQL_LIBRARY}")
message(STATUS "JsonCpp: ${JSONCPP_LIBRARIES}")
message(STATUS "OpenSSL: ${OPENSSL_LIBRARIES}")
message(STATUS "spdlog: header-only mode")
message(STATUS "========================================")

