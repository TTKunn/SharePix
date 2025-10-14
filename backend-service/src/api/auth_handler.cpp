/**
 * @file auth_handler.cpp
 * @brief 认证API处理器实现
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#include "auth_handler.h"
#include "../core/auth_service.h"
#include "../database/user_repository.h"
#include "../security/jwt_manager.h"
#include "../utils/logger.h"
#include "../utils/url_helper.h"
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
    
    // 修改密码（API文档路径：PUT /api/v1/auth/password）
    server.Put("/api/v1/auth/password", [this](const httplib::Request& req, httplib::Response& res) {
        handleChangePassword(req, res);
    });

    // 获取当前用户信息
    server.Get("/api/v1/users/profile", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetProfile(req, res);
    });

    // 更新用户信息
    server.Put("/api/v1/users/profile", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpdateProfile(req, res);
    });

    // 检查用户名可用性 (必须在正则匹配路由之前注册)
    server.Get("/api/v1/users/check-username", [this](const httplib::Request& req, httplib::Response& res) {
        handleCheckUsername(req, res);
    });

    Logger::info("Auth routes registered");
}

// 注册通配符路由（必须最后注册）
void AuthHandler::registerWildcardRoutes(httplib::Server& server) {
    // 获取用户公开信息 (使用正则表达式，必须放在最后)
    server.Get(R"(/api/v1/users/([^/]+))", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetUserPublicInfo(req, res);
    });

    Logger::info("Auth wildcard routes registered");
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
        responseData["valid"] = true;  // 保留valid字段以兼容旧API
        responseData["user_id"] = result.userId;
        responseData["username"] = result.username;
        
        sendJsonResponse(res, 200, true, result.message, responseData);
    } else {
        Json::Value responseData;
        responseData["valid"] = false;  // 保留valid字段以兼容旧API
        
        sendJsonResponse(res, 401, false, result.message, responseData);
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
    
    // 1. 提取并验证JWT令牌
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
        sendJsonResponse(res, 401, false, "未提供认证令牌");
        return;
    }

    std::string token = authHeader.substr(7);

    // 2. 验证令牌
    TokenValidationResult validation = authService_->validateToken(token);
    if (!validation.valid) {
        sendJsonResponse(res, 401, false, validation.message);
        return;
    }
    
    // 3. 解析JSON请求体
    Json::Value requestJson;
    if (!parseJsonBody(req.body, requestJson)) {
        sendJsonResponse(res, 400, false, "无效的JSON格式");
        return;
    }
    
    // 4. 提取参数（从JWT令牌获取userId，从body获取密码）
    std::string oldPassword = requestJson.get("old_password", "").asString();
    std::string newPassword = requestJson.get("new_password", "").asString();
    
    // 5. 验证参数
    if (oldPassword.empty() || newPassword.empty()) {
        sendJsonResponse(res, 400, false, "缺少必要参数");
        return;
    }
    
    // 6. 调用认证服务
    bool result = authService_->changePassword(validation.userId, oldPassword, newPassword);
    
    if (result) {
        sendJsonResponse(res, 200, true, "密码修改成功");
        Logger::info("密码修改成功: userId=" + std::to_string(validation.userId));
    } else {
        sendJsonResponse(res, 400, false, "密码修改失败，请检查旧密码是否正确");
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

// 处理获取当前用户信息请求
void AuthHandler::handleGetProfile(const httplib::Request& req, httplib::Response& res) {
    Logger::info("处理获取当前用户信息请求");

    // 1. 提取并验证JWT令牌
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
        sendJsonResponse(res, 401, false, "未提供认证令牌");
        return;
    }

    std::string token = authHeader.substr(7);

    // 2. 验证令牌
    TokenValidationResult validation = authService_->validateToken(token);

    if (!validation.valid) {
        sendJsonResponse(res, 401, false, validation.message);
        return;
    }

    // 3. 查询用户信息（通过Repository）
    auto userRepo = std::make_unique<UserRepository>();
    auto user = userRepo->findById(validation.userId);

    if (!user.has_value()) {
        sendJsonResponse(res, 404, false, "用户不存在");
        return;
    }

    // 4. 返回用户信息（包含所有字段）
    Json::Value userData = user->toJson();
    // 兼容旧路径字段可能直接读取model的原始avatar_url，确保添加URL前缀
    if (userData.isMember("avatar_url") && userData["avatar_url"].isString()) {
        userData["avatar_url"] = UrlHelper::toFullUrl(userData["avatar_url"].asString());
    }
    // 移除敏感字段
    userData.removeMember("password");
    userData.removeMember("salt");

    sendJsonResponse(res, 200, true, "查询成功", userData);
    Logger::info("用户信息查询成功: userId=" + std::to_string(validation.userId));
}

// 处理更新用户信息请求
void AuthHandler::handleUpdateProfile(const httplib::Request& req, httplib::Response& res) {
    Logger::info("处理更新用户信息请求");

    // 1. 提取并验证JWT令牌
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
        sendJsonResponse(res, 401, false, "未提供认证令牌");
        return;
    }

    std::string token = authHeader.substr(7);

    // 2. 验证令牌
    TokenValidationResult validation = authService_->validateToken(token);

    if (!validation.valid) {
        sendJsonResponse(res, 401, false, validation.message);
        return;
    }

    // 3. 解析JSON请求体
    Json::Value requestJson;
    if (!parseJsonBody(req.body, requestJson)) {
        sendJsonResponse(res, 400, false, "无效的JSON格式");
        return;
    }

    // 4. 提取参数
    std::string realName = requestJson.get("real_name", "").asString();
    std::string email = requestJson.get("email", "").asString();
    std::string avatarUrl = requestJson.get("avatar_url", "").asString();
    std::string phone = requestJson.get("phone", "").asString();
    std::string bio = requestJson.get("bio", "").asString();
    std::string gender = requestJson.get("gender", "").asString();
    std::string location = requestJson.get("location", "").asString();

    // 5. 调用Service层更新
    UpdateProfileResult result = authService_->updateUserProfile(
        validation.userId, realName, email, avatarUrl, phone, bio, gender, location
    );

    if (!result.success) {
        sendJsonResponse(res, 400, false, result.message);
        return;
    }

    // 6. 返回更新后的用户信息
    Json::Value userData = result.user.toJson();
    if (userData.isMember("avatar_url") && userData["avatar_url"].isString()) {
        userData["avatar_url"] = UrlHelper::toFullUrl(userData["avatar_url"].asString());
    }
    // 移除敏感字段
    userData.removeMember("password");
    userData.removeMember("salt");

    sendJsonResponse(res, 200, true, result.message, userData);
    Logger::info("用户信息更新成功: userId=" + std::to_string(validation.userId));
}

// 处理获取用户公开信息请求
void AuthHandler::handleGetUserPublicInfo(const httplib::Request& req, httplib::Response& res) {
    Logger::info("处理获取用户公开信息请求");

    // 1. 提取用户ID（从路径参数）
    std::string userId;
    if (req.matches.size() > 1) {
        userId = req.matches[1];
    }

    if (userId.empty()) {
        sendJsonResponse(res, 400, false, "缺少用户ID参数");
        return;
    }

    Logger::info("查询用户公开信息: userId=" + userId);

    // 2. 调用Service层查询
    auto user = authService_->getUserPublicInfo(userId);

    if (!user.has_value()) {
        sendJsonResponse(res, 404, false, "用户不存在");
        return;
    }

    // 3. 返回公开信息（只包含公开字段）
    Json::Value publicData;
    publicData["user_id"] = user->getUserId();
    publicData["username"] = user->getUsername();
    publicData["real_name"] = user->getRealName();
    publicData["avatar_url"] = UrlHelper::toFullUrl(user->getAvatarUrl());
    publicData["bio"] = user->getBio();
    publicData["gender"] = user->getGender();
    publicData["location"] = user->getLocation();
    publicData["create_time"] = static_cast<Json::Int64>(user->getCreateTime());

    sendJsonResponse(res, 200, true, "查询成功", publicData);
    Logger::info("用户公开信息查询成功: userId=" + userId);
}

// 处理用户名可用性检查请求
void AuthHandler::handleCheckUsername(const httplib::Request& req, httplib::Response& res) {
    Logger::info("处理用户名可用性检查请求");

    // 1. 提取用户名参数
    std::string username;
    if (req.has_param("username")) {
        username = req.get_param_value("username");
    }

    if (username.empty()) {
        sendJsonResponse(res, 400, false, "缺少username参数");
        return;
    }

    Logger::info("检查用户名可用性: username=" + username);

    // 2. 调用Service层检查
    UsernameCheckResult result = authService_->checkUsernameAvailability(username);

    // 3. 返回检查结果
    Json::Value data;
    data["valid"] = result.valid;
    data["available"] = result.available;

    int statusCode = result.valid ? 200 : 400;
    sendJsonResponse(res, statusCode, result.valid, result.message, data);

    Logger::info("用户名可用性检查完成: username=" + username +
                 ", valid=" + (result.valid ? "true" : "false") +
                 ", available=" + (result.available ? "true" : "false"));
}

