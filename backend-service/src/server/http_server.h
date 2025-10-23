/**
 * @file http_server.h
 * @brief 使用 cpp-httplib 实现的 HTTP 服务器
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
class PostHandler;
class LikeHandler;
class FavoriteHandler;
class FollowHandler;
class CommentHandler;
class ShareHandler;

/**
 * @brief HTTP 服务器封装类
 */
class HttpServer {
public:
    /**
     * @brief 构造函数
     */
    HttpServer();
    
    /**
     * @brief 析构函数
     */
    ~HttpServer();
    
    /**
     * @brief 使用配置初始化服务器
     * @return 成功返回 true，否则返回 false
     */
    bool initialize();
    
    /**
     * @brief 启动 HTTP 服务器
     * @return 成功返回 true，否则返回 false
     */
    bool start();
    
    /**
     * @brief 停止 HTTP 服务器
     */
    void stop();
    
    /**
     * @brief 检查服务器是否正在运行
     * @return 运行中返回 true，否则返回 false
     */
    bool isRunning() const { return running_; }

private:
    std::unique_ptr<httplib::Server> server_;
    std::unique_ptr<AuthHandler> authHandler_;
    std::unique_ptr<ImageHandler> imageHandler_;
    std::unique_ptr<PostHandler> postHandler_;
    std::unique_ptr<LikeHandler> likeHandler_;
    std::unique_ptr<FavoriteHandler> favoriteHandler_;
    std::unique_ptr<FollowHandler> followHandler_;
    std::unique_ptr<CommentHandler> commentHandler_;
    std::unique_ptr<ShareHandler> shareHandler_;
    std::string host_;
    int port_;
    bool running_;
    
    /**
     * @brief 设置中间件
     */
    void setupMiddleware();
    
    /**
     * @brief 设置路由
     */
    void setupRoutes();
    
    /**
     * @brief 设置 CORS 中间件
     */
    void setupCORS();
    
    /**
     * @brief 设置错误处理器
     */
    void setupErrorHandlers();
    
    /**
     * @brief 设置静态文件服务
     */
    void setupStaticFiles();
    
    /**
     * @brief 健康检查端点处理器
     */
    void handleHealthCheck(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 指标端点处理器
     */
    void handleMetrics(const httplib::Request& req, httplib::Response& res);
};
