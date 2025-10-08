/**
 * @file logger.cpp
 * @brief Logging utility implementation using spdlog
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#include "utils/logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <memory>
#include <vector>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <sys/types.h>
#include <libgen.h>
#include <cstring>

// Static member initialization
std::mutex Logger::logMutex_;
LogLevel Logger::currentLevel_ = LogLevel::INFO;
std::unique_ptr<std::ofstream> Logger::logFile_;
bool Logger::consoleOutput_ = true;
bool Logger::initialized_ = false;

// Initialize logger with configuration
bool Logger::initialize(const std::string& logFile,
                       LogLevel level,
                       bool consoleOutput) {
    std::lock_guard<std::mutex> lock(logMutex_);

    try {
        std::vector<spdlog::sink_ptr> sinks;

        // Console sink
        if (consoleOutput) {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::trace);
            sinks.push_back(console_sink);
        }

        // File sink
        if (!logFile.empty()) {
            // Create log directory if it doesn't exist
            std::string logFileCopy = logFile;
            char* logFilePathCopy = strdup(logFileCopy.c_str());
            char* logDir = dirname(logFilePathCopy);

            // Create directory recursively
            struct stat st;
            if (stat(logDir, &st) != 0) {
                // Directory doesn't exist, create it
                std::string mkdirCmd = "mkdir -p " + std::string(logDir);
                if (system(mkdirCmd.c_str()) != 0) {
                    free(logFilePathCopy);
                    return false;
                }
            }
            free(logFilePathCopy);

            // Create rotating file sink (10MB max size, 5 rotated files)
            auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                logFile, 1024 * 1024 * 10, 5);
            file_sink->set_level(spdlog::level::trace);
            sinks.push_back(file_sink);
        }
        
        // Create logger
        auto logger = std::make_shared<spdlog::logger>("shared_parking", 
                                                       sinks.begin(), 
                                                       sinks.end());
        
        // Set log level
        switch (level) {
            case LogLevel::DEBUG:
                logger->set_level(spdlog::level::debug);
                break;
            case LogLevel::INFO:
                logger->set_level(spdlog::level::info);
                break;
            case LogLevel::WARNING:
                logger->set_level(spdlog::level::warn);
                break;
            case LogLevel::ERROR:
                logger->set_level(spdlog::level::err);
                break;
            case LogLevel::FATAL:
                logger->set_level(spdlog::level::critical);
                break;
        }
        
        // Set pattern
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [thread %t] %v");
        
        // Register as default logger
        spdlog::set_default_logger(logger);
        spdlog::flush_on(spdlog::level::err);
        
        currentLevel_ = level;
        consoleOutput_ = consoleOutput;
        initialized_ = true;
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// Log debug message
void Logger::debug(const std::string& message) {
    if (initialized_ && currentLevel_ <= LogLevel::DEBUG) {
        spdlog::debug(message);
    }
}

// Log info message
void Logger::info(const std::string& message) {
    if (initialized_ && currentLevel_ <= LogLevel::INFO) {
        spdlog::info(message);
    }
}

// Log warning message
void Logger::warning(const std::string& message) {
    if (initialized_ && currentLevel_ <= LogLevel::WARNING) {
        spdlog::warn(message);
    }
}

// Log error message
void Logger::error(const std::string& message) {
    if (initialized_ && currentLevel_ <= LogLevel::ERROR) {
        spdlog::error(message);
    }
}

// Log fatal message
void Logger::fatal(const std::string& message) {
    if (initialized_) {
        spdlog::critical(message);
        spdlog::shutdown();
    }
}

// Set log level
void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex_);
    currentLevel_ = level;
    
    if (initialized_) {
        switch (level) {
            case LogLevel::DEBUG:
                spdlog::set_level(spdlog::level::debug);
                break;
            case LogLevel::INFO:
                spdlog::set_level(spdlog::level::info);
                break;
            case LogLevel::WARNING:
                spdlog::set_level(spdlog::level::warn);
                break;
            case LogLevel::ERROR:
                spdlog::set_level(spdlog::level::err);
                break;
            case LogLevel::FATAL:
                spdlog::set_level(spdlog::level::critical);
                break;
        }
    }
}

// Get current log level
LogLevel Logger::getLevel() {
    std::lock_guard<std::mutex> lock(logMutex_);
    return currentLevel_;
}

// Internal log function (not used with spdlog, kept for compatibility)
void Logger::log(LogLevel level, const std::string& message) {
    switch (level) {
        case LogLevel::DEBUG:
            debug(message);
            break;
        case LogLevel::INFO:
            info(message);
            break;
        case LogLevel::WARNING:
            warning(message);
            break;
        case LogLevel::ERROR:
            error(message);
            break;
        case LogLevel::FATAL:
            fatal(message);
            break;
    }
}

// Get current timestamp string (not used with spdlog, kept for compatibility)
std::string Logger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// Convert log level to string (not used with spdlog, kept for compatibility)
std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:                return "UNKNOWN";
    }
}

