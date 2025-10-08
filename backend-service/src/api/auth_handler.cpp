/**
 * @file auth_handler.cpp
 * @brief 认证API处理器实现
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#include "auth_handler.h"
#include "../core/auth_service.h"
#include "../utils/logger.h"
#include <sstream>

// 构造函数
AuthHandler::AuthHandler() {
    authService_ = std::make_unique<AuthService>();
    Logger::info("AuthHandler initialized");
}

// 析构函数
AuthHandler::~AuthHandler() {
    Logger::info("AuthHandler destroyed");
}

// 注册路由
void AuthHandler::registerRoutes(httplib::Server& server) {
    // 用户注册
    server.Post("/api/v1/auth/register", [this](const httplib::Request& req, httplib::Response& res) {
        handleRegister(req, res);
    });
    
    // 用户登录
    server.Post("/api/v1/auth/login", [this](const httplib::Request& req, httplib::Response& res) {
        handleLogin(req, res);
    });
    
    // 令牌验证
    server.Post("/api/v1/auth/validate", [this](const httplib::Request& req, httplib::Response& res) {
        handleValidate(req, res);
    });
    
    // 令牌刷新
    server.Post("/api/v1/auth/refresh", [this](const httplib::Request& req, httplib::Response& res) {
        handleRefresh(req, res);
    });
    
    // 用户登出
    server.Post("/api/v1/auth/logout", [this](const httplib::Request& req, httplib::Response& res) {
        handleLogout(req, res);
    });
    
    // 修改密码
    server.Post("/api/v1/auth/change-password", [this](const httplib::Request& req, httplib::Response& res) {
        handleChangePassword(req, res);
    });
    
    Logger::info("Auth routes registered");
}

// 处理用户注册
void AuthHandler::handleRegister(const httplib::Request& req, httplib::Response& res) {
    Logger::info("Handling register request");
    
    // 解析JSON请求体
    Json::Value requestJson;
    if (!parseJsonBody(req.body, requestJson)) {
        sendJsonResponse(res, 400, false, "无效的JSON格式");
        return;
    }
    
    // 提取参数
    std::string username = requestJson.get("username", "").asString();
    std::string password = requestJson.get("password", "").asString();
    std::string realName = requestJson.get("real_name", "").asString();
    std::string phone = requestJson.get("phone", "").asString();
    std::string email = requestJson.get("email", "").asString();
    std::string roleStr = requestJson.get("role", "user").asString();

    // 转换角色
    UserRole role = UserRole::USER;
    if (roleStr == "admin") {
        role = UserRole::ADMIN;
    }
    
    // 调用认证服务
    RegisterResult result = authService_->registerUser(username, password, realName, phone, email, role);
    
    if (result.success) {
        // 构建响应数据
        Json::Value userData = result.user.toJson(false); // 不包含敏感信息
        sendJsonResponse(res, 200, true, result.message, userData);
    } else {
        sendJsonResponse(res, 400, false, result.message);
    }
}

// 处理用户登录
void AuthHandler::handleLogin(const httplib::Request& req, httplib::Response& res) {
    Logger::info("Handling login request");
    
    // 解析JSON请求体
    Json::Value requestJson;
    if (!parseJsonBody(req.body, requestJson)) {
        sendJsonResponse(res, 400, false, "无效的JSON格式");
        return;
    }
    
    // 提取参数
    std::string username = requestJson.get("username", "").asString();
    std::string password = requestJson.get("password", "").asString();
    
    // 调用认证服务
    AuthResult result = authService_->loginUser(username, password);
    
    if (result.success) {
        // 构建响应数据
        Json::Value responseData;
        responseData["access_token"] = result.accessToken;
        responseData["refresh_token"] = result.refreshToken;
        responseData["user"] = result.user.toJson(false); // 不包含敏感信息
        
        sendJsonResponse(res, 200, true, result.message, responseData);
    } else {
        sendJsonResponse(res, 401, false, result.message);
    }
}

// 处理令牌验证
void AuthHandler::handleValidate(const httplib::Request& req, httplib::Response& res) {
    Logger::info("Handling validate request");
    
    // 解析JSON请求体
    Json::Value requestJson;
    if (!parseJsonBody(req.body, requestJson)) {
        sendJsonResponse(res, 400, false, "无效的JSON格式");
        return;
    }
    
    // 提取令牌
    std::string token = requestJson.get("token", "").asString();
    
    // 调用认证服务
    TokenValidationResult result = authService_->validateToken(token);
    
    if (result.valid) {
        // 构建响应数据
        Json::Value responseData;
        responseData["user_id"] = result.userId;
        responseData["username"] = result.username;
        
        Json::Value response;
        response["valid"] = true;
        response["message"] = result.message;
        response["data"] = responseData;
        
        Json::StreamWriterBuilder writer;
        std::string jsonStr = Json::writeString(writer, response);
        res.set_content(jsonStr, "application/json");
        res.status = 200;
    } else {
        Json::Value response;
        response["valid"] = false;
        response["message"] = result.message;
        response["data"] = Json::Value::null;
        
        Json::StreamWriterBuilder writer;
        std::string jsonStr = Json::writeString(writer, response);
        res.set_content(jsonStr, "application/json");
        res.status = 401;
    }
}

// 处理令牌刷新
void AuthHandler::handleRefresh(const httplib::Request& req, httplib::Response& res) {
    Logger::info("Handling refresh request");
    
    // 解析JSON请求体
    Json::Value requestJson;
    if (!parseJsonBody(req.body, requestJson)) {
        sendJsonResponse(res, 400, false, "无效的JSON格式");
        return;
    }
    
    // 提取刷新令牌
    std::string refreshToken = requestJson.get("refresh_token", "").asString();
    
    // 调用认证服务
    AuthResult result = authService_->refreshTokens(refreshToken);
    
    if (result.success) {
        // 构建响应数据
        Json::Value responseData;
        responseData["access_token"] = result.accessToken;
        responseData["refresh_token"] = result.refreshToken;
        
        sendJsonResponse(res, 200, true, result.message, responseData);
    } else {
        sendJsonResponse(res, 401, false, result.message);
    }
}

// 处理用户登出
void AuthHandler::handleLogout(const httplib::Request& req, httplib::Response& res) {
    Logger::info("Handling logout request");
    
    // 解析JSON请求体
    Json::Value requestJson;
    if (!parseJsonBody(req.body, requestJson)) {
        sendJsonResponse(res, 400, false, "无效的JSON格式");
        return;
    }
    
    // 提取访问令牌
    std::string accessToken = requestJson.get("access_token", "").asString();
    
    // 调用认证服务
    bool result = authService_->logoutUser(accessToken);
    
    if (result) {
        sendJsonResponse(res, 200, true, "登出成功");
    } else {
        sendJsonResponse(res, 500, false, "登出失败");
    }
}

// 处理修改密码
void AuthHandler::handleChangePassword(const httplib::Request& req, httplib::Response& res) {
    Logger::info("Handling change password request");
    
    // 解析JSON请求体
    Json::Value requestJson;
    if (!parseJsonBody(req.body, requestJson)) {
        sendJsonResponse(res, 400, false, "无效的JSON格式");
        return;
    }
    
    // 提取参数
    int userId = requestJson.get("user_id", 0).asInt();
    std::string oldPassword = requestJson.get("old_password", "").asString();
    std::string newPassword = requestJson.get("new_password", "").asString();
    
    // 调用认证服务
    bool result = authService_->changePassword(userId, oldPassword, newPassword);
    
    if (result) {
        sendJsonResponse(res, 200, true, "密码修改成功");
    } else {
        sendJsonResponse(res, 400, false, "密码修改失败");
    }
}

// 解析JSON请求体
bool AuthHandler::parseJsonBody(const std::string& body, Json::Value& jsonOut) {
    Json::CharReaderBuilder reader;
    std::istringstream stream(body);
    std::string errors;
    
    bool success = Json::parseFromStream(reader, stream, &jsonOut, &errors);
    if (!success) {
        Logger::warning("Failed to parse JSON: " + errors);
    }
    
    return success;
}

// 发送JSON响应
void AuthHandler::sendJsonResponse(httplib::Response& res,
                                   int statusCode,
                                   bool success,
                                   const std::string& message,
                                   const Json::Value& data) {
    Json::Value response;
    response["success"] = success;
    response["message"] = message;
    response["data"] = data;
    
    Json::StreamWriterBuilder writer;
    std::string jsonStr = Json::writeString(writer, response);
    
    res.set_content(jsonStr, "application/json");
    res.status = statusCode;
}

