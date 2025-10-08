/**
 * @file logger.h
 * @brief Logging utility for the authentication service
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#pragma once

#include <string>
#include <memory>
#include <mutex>
#include <fstream>

/**
 * @brief Log levels enumeration
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
};

/**
 * @brief Static logger class for application-wide logging
 */
class Logger {
public:
    /**
     * @brief Initialize logger with configuration
     * @param logFile Path to log file (empty for console only)
     * @param level Minimum log level
     * @param consoleOutput Enable console output
     * @return true if successful, false otherwise
     */
    static bool initialize(const std::string& logFile = "", 
                          LogLevel level = LogLevel::INFO,
                          bool consoleOutput = true);
    
    /**
     * @brief Log debug message
     * @param message Log message
     */
    static void debug(const std::string& message);
    
    /**
     * @brief Log info message
     * @param message Log message
     */
    static void info(const std::string& message);
    
    /**
     * @brief Log warning message
     * @param message Log message
     */
    static void warning(const std::string& message);
    
    /**
     * @brief Log error message
     * @param message Log message
     */
    static void error(const std::string& message);
    
    /**
     * @brief Log fatal message
     * @param message Log message
     */
    static void fatal(const std::string& message);
    
    /**
     * @brief Set log level
     * @param level New log level
     */
    static void setLevel(LogLevel level);
    
    /**
     * @brief Get current log level
     * @return Current log level
     */
    static LogLevel getLevel();

private:
    static std::mutex logMutex_;
    static LogLevel currentLevel_;
    static std::unique_ptr<std::ofstream> logFile_;
    static bool consoleOutput_;
    static bool initialized_;
    
    /**
     * @brief Internal log function
     * @param level Log level
     * @param message Log message
     */
    static void log(LogLevel level, const std::string& message);
    
    /**
     * @brief Get current timestamp string
     * @return Formatted timestamp
     */
    static std::string getCurrentTimestamp();
    
    /**
     * @brief Convert log level to string
     * @param level Log level
     * @return String representation
     */
    static std::string levelToString(LogLevel level);
};
