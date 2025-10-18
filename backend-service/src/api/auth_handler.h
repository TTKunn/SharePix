/**
 * @file auth_handler.h
 * @brief 认证API处理器
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#pragma once

#include "httplib.h"
#include <json/json.h>
#include <memory>

// 前向声明
class AuthService;

/**
 * @brief 认证API处理器类
 * 
 * 处理所有认证相关的HTTP请求，包括注册、登录、令牌验证等
 */
class AuthHandler {
public:
    /**
     * @brief 构造函数
     */
    AuthHandler();
    
    /**
     * @brief 析构函数
     */
    ~AuthHandler();
    
    /**
     * @brief 注册路由到HTTP服务器
     * 
     * @param server HTTP服务器实例
     */
    void registerRoutes(httplib::Server& server);
    
    /**
     * @brief 注册通配符路由（必须最后注册以避免覆盖其他路由）
     * 
     * @param server HTTP服务器实例
     */
    void registerWildcardRoutes(httplib::Server& server);
    
private:
    /**
     * @brief 处理用户注册请求
     * 
     * POST /api/v1/auth/register
     */
    void handleRegister(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理用户登录请求
     * 
     * POST /api/v1/auth/login
     */
    void handleLogin(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理令牌验证请求
     * 
     * POST /api/v1/auth/validate
     */
    void handleValidate(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理令牌刷新请求
     * 
     * POST /api/v1/auth/refresh
     */
    void handleRefresh(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理用户登出请求
     * 
     * POST /api/v1/auth/logout
     */
    void handleLogout(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理修改密码请求
     *
     * POST /api/v1/auth/change-password
     */
    void handleChangePassword(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理获取当前用户信息请求
     *
     * GET /api/v1/users/profile
     */
    void handleGetProfile(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理更新用户信息请求
     *
     * PUT /api/v1/users/profile
     */
    void handleUpdateProfile(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理获取用户公开信息请求
     *
     * GET /api/v1/users/:user_id
     */
    void handleGetUserPublicInfo(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理用户名可用性检查请求
     *
     * GET /api/v1/users/check-username
     */
    void handleCheckUsername(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 处理头像上传请求
     *
     * POST /api/v1/users/avatar
     */
    void handleUploadAvatar(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 解析JSON请求体
     * 
     * @param body 请求体字符串
     * @param jsonOut 输出的JSON对象
     * @return true 解析成功，false 解析失败
     */
    bool parseJsonBody(const std::string& body, Json::Value& jsonOut);
    
    /**
     * @brief 发送JSON响应
     * 
     * @param res HTTP响应对象
     * @param statusCode HTTP状态码
     * @param success 是否成功
     * @param message 消息内容
     * @param data 数据对象（可选）
     */
    void sendJsonResponse(httplib::Response& res, 
                         int statusCode,
                         bool success, 
                         const std::string& message,
                         const Json::Value& data = Json::Value::null);
    
    std::unique_ptr<AuthService> authService_;
};

