/**
 * @file http_server.h
 * @brief HTTP server implementation using cpp-httplib
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#pragma once

#include <memory>
#include <string>
#include <functional>
#include "httplib.h"
#include <json/json.h>

// 前向声明
class AuthHandler;
class ImageHandler;

/**
 * @brief HTTP server wrapper class
 */
class HttpServer {
public:
    /**
     * @brief Constructor
     */
    HttpServer();
    
    /**
     * @brief Destructor
     */
    ~HttpServer();
    
    /**
     * @brief Initialize server with configuration
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Start the HTTP server
     * @return true if successful, false otherwise
     */
    bool start();
    
    /**
     * @brief Stop the HTTP server
     */
    void stop();
    
    /**
     * @brief Check if server is running
     * @return true if running, false otherwise
     */
    bool isRunning() const { return running_; }

private:
    std::unique_ptr<httplib::Server> server_;
    std::unique_ptr<AuthHandler> authHandler_;
    std::unique_ptr<ImageHandler> imageHandler_;
    std::string host_;
    int port_;
    bool running_;
    
    /**
     * @brief Setup middleware
     */
    void setupMiddleware();
    
    /**
     * @brief Setup routes
     */
    void setupRoutes();
    
    /**
     * @brief Setup CORS middleware
     */
    void setupCORS();
    
    /**
     * @brief Setup error handlers
     */
    void setupErrorHandlers();
    
    /**
     * @brief Health check endpoint handler
     */
    void handleHealthCheck(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief Metrics endpoint handler
     */
    void handleMetrics(const httplib::Request& req, httplib::Response& res);
};
