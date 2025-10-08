cmake_minimum_required(VERSION 3.16)
project(knot_image_sharing VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party)
include_directories(${CMAKE_SOURCE_DIR}/src)

# Find required packages
find_package(PkgConfig REQUIRED)

# Find MySQL
find_path(MYSQL_INCLUDE_DIR mysql/mysql.h
    PATHS /usr/include /usr/local/include /opt/local/include
    PATH_SUFFIXES mysql)

find_library(MYSQL_LIBRARY
    NAMES mysqlclient mysql
    PATHS /usr/lib /usr/local/lib /opt/local/lib
    PATH_SUFFIXES mysql)

if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    message(STATUS "Found MySQL: ${MYSQL_LIBRARY}")
    set(MYSQL_FOUND TRUE)
else()
    message(FATAL_ERROR "MySQL not found")
endif()

# Find JsonCpp
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
if(NOT JSONCPP_FOUND)
    find_path(JSONCPP_INCLUDE_DIR json/json.h
        PATHS /usr/include /usr/local/include
        PATH_SUFFIXES jsoncpp)
    find_library(JSONCPP_LIBRARY
        NAMES jsoncpp
        PATHS /usr/lib /usr/local/lib)
    if(JSONCPP_INCLUDE_DIR AND JSONCPP_LIBRARY)
        set(JSONCPP_FOUND TRUE)
        set(JSONCPP_INCLUDE_DIRS ${JSONCPP_INCLUDE_DIR})
        set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARY})
    endif()
endif()

if(NOT JSONCPP_FOUND)
    message(FATAL_ERROR "JsonCpp not found. Please install libjsoncpp-dev")
endif()

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find spdlog
find_package(spdlog REQUIRED)

# Find Threads
find_package(Threads REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${MYSQL_LIBRARY}
    ${JSONCPP_LIBRARIES}
    OpenSSL::SSL
    OpenSSL::Crypto
    spdlog::spdlog
    Threads::Threads
    dl
)

# Include directories for target
target_include_directories(${PROJECT_NAME} PRIVATE
    ${MYSQL_INCLUDE_DIR}
    ${JSONCPP_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/jwt-cpp/include
)

# Compiler definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    CPPHTTPLIB_OPENSSL_SUPPORT
    CPPHTTPLIB_LISTEN_BACKLOG=128      # 增加socket监听队列大小，支持更多并发连接
    CPPHTTPLIB_THREAD_POOL_COUNT=32    # 增加线程池大小，支持更多并发请求处理
)

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Install config files
install(DIRECTORY config/
    DESTINATION etc/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.json"
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "MySQL include: ${MYSQL_INCLUDE_DIR}")
message(STATUS "MySQL library: ${MYSQL_LIBRARY}")
