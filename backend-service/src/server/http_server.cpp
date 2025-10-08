/**
 * @file http_server.cpp
 * @brief HTTP server implementation using cpp-httplib
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#include "server/http_server.h"
#include "api/auth_handler.h"
#include "api/image_handler.h"
#include "utils/config_manager.h"
#include "utils/logger.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include <json/json.h>
#include <chrono>

HttpServer::HttpServer()
    : server_(std::make_unique<httplib::Server>()),
      authHandler_(std::make_unique<AuthHandler>()),
      imageHandler_(std::make_unique<ImageHandler>()),
      host_("0.0.0.0"),
      port_(8080),
      running_(false) {
}

HttpServer::~HttpServer() {
    if (running_) {
        stop();
    }
}

bool HttpServer::initialize() {
    try {
        // Load configuration
        auto& config = ConfigManager::getInstance();
        host_ = config.get<std::string>("server.host", "0.0.0.0");
        port_ = config.get<int>("server.port", 8080);
        
        Logger::info("Initializing HTTP server on " + host_ + ":" + std::to_string(port_));
        
        // Setup middleware
        setupMiddleware();
        
        // Setup CORS
        setupCORS();
        
        // Setup routes
        setupRoutes();
        
        // Setup error handlers
        setupErrorHandlers();
        
        Logger::info("HTTP server initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to initialize HTTP server: " + std::string(e.what()));
        return false;
    }
}

bool HttpServer::start() {
    if (running_) {
        Logger::warning("HTTP server is already running");
        return true;
    }
    
    Logger::info("Starting HTTP server...");
    
    // Start server in a separate thread
    running_ = true;
    
    // This is a blocking call
    bool result = server_->listen(host_.c_str(), port_);
    
    if (!result) {
        Logger::error("Failed to start HTTP server");
        running_ = false;
        return false;
    }
    
    return true;
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }
    
    Logger::info("Stopping HTTP server...");
    
    server_->stop();
    running_ = false;
    
    Logger::info("HTTP server stopped");
}

void HttpServer::setupMiddleware() {
    // Request logging middleware
    server_->set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        Logger::info("Request: " + req.method + " " + req.path);
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // Response logging middleware + CORS headers (合并处理,避免重复设置导致覆盖)
    server_->set_post_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        // 1. 记录响应日志
        Logger::info("Response: " + std::to_string(res.status) + " for " + req.method + " " + req.path);

        // 2. 设置CORS头
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        res.set_header("Access-Control-Max-Age", "3600");
    });
}

void HttpServer::setupRoutes() {
    // 注册认证相关路由
    authHandler_->registerRoutes(*server_);

    // 注册图片相关路由
    imageHandler_->registerRoutes(*server_);

    // Health check endpoint
    server_->Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealthCheck(req, res);
    });

    // Metrics endpoint
    server_->Get("/metrics", [this](const httplib::Request& req, httplib::Response& res) {
        handleMetrics(req, res);
    });

    // API version endpoint
    server_->Get("/api/v1/version", [](const httplib::Request& req, httplib::Response& res) {
        Json::Value response;
        response["version"] = "1.0.0";
        response["service"] = "Knot - Image Sharing Service";
        response["timestamp"] = static_cast<Json::Int64>(std::time(nullptr));

        Json::StreamWriterBuilder writer;
        std::string jsonStr = Json::writeString(writer, response);

        res.set_content(jsonStr, "application/json");
        res.status = 200;
    });
}

void HttpServer::setupCORS() {
    // CORS headers - 已在 setupMiddleware() 中的 post_routing_handler 里统一处理
    // 避免重复设置 post_routing_handler 导致覆盖,这是导致 BUG #1 的根因

    // Handle OPTIONS requests
    server_->Options(".*", [](const httplib::Request& req, httplib::Response& res) {
        res.status = 204;
    });
}

void HttpServer::setupErrorHandlers() {
    // 404 Not Found - 只处理真正的404错误,不拦截其他4xx错误(如400)
    server_->set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        // 只有当状态码是404时才覆盖响应
        if (res.status == 404) {
            Json::Value error;
            error["success"] = false;
            error["error"] = "Not Found";
            error["message"] = "The requested endpoint does not exist";
            error["path"] = req.path;
            error["timestamp"] = static_cast<Json::Int64>(std::time(nullptr));

            Json::StreamWriterBuilder writer;
            std::string jsonStr = Json::writeString(writer, error);

            res.set_content(jsonStr, "application/json");
        }
        // 其他状态码(如400, 401, 403等)保持原始响应不变
    });

    // Exception handler
    server_->set_exception_handler([](const httplib::Request& req, httplib::Response& res, std::exception_ptr ep) {
        try {
            std::rethrow_exception(ep);
        } catch (const std::exception& e) {
            Logger::error("Exception in request handler: " + std::string(e.what()));

            Json::Value error;
            error["success"] = false;
            error["error"] = "Internal Server Error";
            error["message"] = "An unexpected error occurred";
            error["timestamp"] = static_cast<Json::Int64>(std::time(nullptr));

            Json::StreamWriterBuilder writer;
            std::string jsonStr = Json::writeString(writer, error);

            res.set_content(jsonStr, "application/json");
            res.status = 500;
        }
    });
}

void HttpServer::handleHealthCheck(const httplib::Request& req, httplib::Response& res) {
    Json::Value response;
    response["status"] = "healthy";
    response["service"] = "Knot - Image Sharing Service";
    response["timestamp"] = static_cast<Json::Int64>(std::time(nullptr));

    // Check database connection
    // 使用ConnectionGuard自动管理连接，确保连接正确归还
    ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());

    if (connGuard.isValid()) {
        response["database"] = "connected";
    } else {
        response["database"] = "disconnected";
        response["status"] = "unhealthy";
    }
    // connGuard析构时自动归还连接

    Json::StreamWriterBuilder writer;
    std::string jsonStr = Json::writeString(writer, response);

    res.set_content(jsonStr, "application/json");
    res.status = (response["status"].asString() == "healthy") ? 200 : 503;
}

void HttpServer::handleMetrics(const httplib::Request& req, httplib::Response& res) {
    Json::Value response;

    // Server metrics
    Json::Value serverMetrics;
    serverMetrics["running"] = running_;
    serverMetrics["host"] = host_;
    serverMetrics["port"] = port_;
    response["server"] = serverMetrics;

    // Database metrics
    auto& dbPool = DatabaseConnectionPool::getInstance();
    response["database"] = dbPool.getStats();

    // Timestamp
    response["timestamp"] = static_cast<Json::Int64>(std::time(nullptr));

    Json::StreamWriterBuilder writer;
    std::string jsonStr = Json::writeString(writer, response);

    res.set_content(jsonStr, "application/json");
    res.status = 200;
}

