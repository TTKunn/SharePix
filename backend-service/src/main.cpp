/**
 * @file main.cpp
 * @brief Main entry point for Knot - Image Sharing Service
 * @author Knot Team
 * @date 2024-01-01
 * @version 1.0.0
 */

#include <iostream>
#include <memory>
#include <signal.h>

// Third party includes
#include "httplib.h"
#include <json/json.h>

// Project includes
#include "utils/config_manager.h"
#include "utils/logger.h"
#include "server/http_server.h"
#include "database/connection_pool.h"
// Note: auth_service.h will be included when implementing authentication in Phase B

using Json::Value;

// Global server instance for signal handling
std::unique_ptr<HttpServer> g_server;

/**
 * @brief Signal handler for graceful shutdown
 * @param signal Signal number
 */
void signalHandler(int signal) {
    Logger::info("Received signal " + std::to_string(signal) + ", shutting down gracefully...");
    
    if (g_server) {
        g_server->stop();
    }
    
    Logger::info("Server stopped successfully");
    exit(0);
}

/**
 * @brief Initialize signal handlers
 */
void setupSignalHandlers() {
    signal(SIGINT, signalHandler);   // Ctrl+C
    signal(SIGTERM, signalHandler);  // Termination request
    signal(SIGQUIT, signalHandler);  // Quit signal
}

/**
 * @brief Print application banner
 */
void printBanner() {
    std::cout << R"(
    __ __           __
   / //_/___  ____ / /_
  / ,<  / _ \/ __ \/ __/
 / /| |/  __/ /_/ / /_
/_/ |_|\___/\____/\__/

Image Sharing Service v1.0.0
)" << std::endl;
}

/**
 * @brief Main application entry point
 * @param argc Argument count
 * @param argv Argument values
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    try {
        // Print banner
        printBanner();
        
        // Setup signal handlers
        setupSignalHandlers();
        
        // Load configuration
        std::string configPath = "config/config.json";
        if (argc > 1) {
            configPath = argv[1];
        }

        std::cout << "Loading configuration from: " << configPath << std::endl;
        if (!ConfigManager::getInstance().loadConfig(configPath)) {
            std::cerr << "Failed to load configuration file: " << configPath << std::endl;
            return 1;
        }

        // Initialize logger
        auto& config = ConfigManager::getInstance();
        std::string logFile = config.get<std::string>("logging.file", "logs/app.log");
        std::string logLevelStr = config.get<std::string>("logging.level", "info");
        bool consoleOutput = config.get<bool>("logging.console", true);

        // Convert log level string to enum
        LogLevel logLevel = LogLevel::INFO;
        if (logLevelStr == "debug") logLevel = LogLevel::DEBUG;
        else if (logLevelStr == "info") logLevel = LogLevel::INFO;
        else if (logLevelStr == "warning") logLevel = LogLevel::WARNING;
        else if (logLevelStr == "error") logLevel = LogLevel::ERROR;
        else if (logLevelStr == "fatal") logLevel = LogLevel::FATAL;

        if (!Logger::initialize(logFile, logLevel, consoleOutput)) {
            std::cerr << "Failed to initialize logger" << std::endl;
            return 1;
        }

        Logger::info("Configuration loaded successfully");
        Logger::info("Initializing Knot image sharing service...");
        
        // Initialize database connection pool
        Logger::info("Initializing database connection pool...");
        auto& dbPool = DatabaseConnectionPool::getInstance();
        if (!dbPool.initialize()) {
            Logger::error("Failed to initialize database connection pool");
            return 1;
        }
        
        // Create HTTP server
        Logger::info("Creating HTTP server...");
        g_server = std::make_unique<HttpServer>();
        
        // Initialize and start server
        if (!g_server->initialize()) {
            Logger::error("Failed to initialize HTTP server");
            return 1;
        }
        
        Logger::info("Starting HTTP server...");
        if (!g_server->start()) {
            Logger::error("Failed to start HTTP server");
            return 1;
        }

        Logger::info("Knot service started successfully");
        Logger::info("Server is running and ready to accept connections");
        
        // Keep the main thread alive
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        Logger::error("Fatal error: " + std::string(e.what()));
        return 1;
    } catch (...) {
        Logger::error("Unknown fatal error occurred");
        return 1;
    }
    
    return 0;
}
