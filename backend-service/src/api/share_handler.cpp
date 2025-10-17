/**
 * @file share_handler.cpp
 * @brief 分享链接API处理器实现
 * @author Knot Development Team
 * @date 2025-10-16
 */

#include "api/share_handler.h"
#include "core/share_service.h"
#include "core/auth_service.h"
#include "security/jwt_manager.h"
#include "utils/logger.h"
#include <json/json.h>
#include <sstream>
#include <ctime>

// 注册路由
void ShareHandler::registerRoutes(httplib::Server& server) {
    // POST /api/v1/posts/:post_id/share - 创建分享链接
    server.Post("/api/v1/posts/(\\d+)/share", [this](const httplib::Request& req, httplib::Response& res) {
        handleCreateShareLink(req, res);
    });
    
    // GET /api/v1/share/:code - 解析分享链接
    server.Get("/api/v1/share/([^/]+)", [this](const httplib::Request& req, httplib::Response& res) {
        handleResolveShareLink(req, res);
    });
    
    Logger::info("ShareHandler routes registered");
}

// 创建帖子分享链接
void ShareHandler::handleCreateShareLink(const httplib::Request& req, httplib::Response& res) {
    res.set_header("Content-Type", "application/json");
    
    // 1. JWT认证
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
        res.status = 401;
        res.set_content(buildResponse(false, "未提供认证令牌"), "application/json");
        return;
    }
    
    std::string token = authHeader.substr(7);
    auto jwtManager = std::make_unique<JWTManager>();
    
    if (!jwtManager->validateToken(token)) {
        res.status = 401;
        res.set_content(buildResponse(false, "认证令牌无效或已过期"), "application/json");
        return;
    }
    
    Json::Value tokenData = jwtManager->decodeToken(token);
    int64_t userId = tokenData["user_id"].asInt64();
    
    // 2. 获取post_id
    if (req.matches.size() < 2) {
        res.status = 400;
        res.set_content(buildResponse(false, "无效的帖子ID"), "application/json");
        return;
    }
    
    int64_t postId = 0;
    try {
        postId = std::stoll(req.matches[1].str());
    } catch (const std::exception& e) {
        res.status = 400;
        res.set_content(buildResponse(false, "无效的帖子ID格式"), "application/json");
        return;
    }
    
    // 3. 解析请求体（可选参数）
    int expireDays = 0;
    if (!req.body.empty()) {
        Json::Reader reader;
        Json::Value requestJson;
        
        if (reader.parse(req.body, requestJson)) {
            if (requestJson.isMember("expire_days") && requestJson["expire_days"].isInt()) {
                expireDays = requestJson["expire_days"].asInt();
                
                // 验证过期天数范围
                if (expireDays < 0 || expireDays > 365) {
                    res.status = 400;
                    res.set_content(buildResponse(false, "过期天数必须在0-365之间"), "application/json");
                    return;
                }
            }
        }
    }
    
    // 4. 调用服务创建分享链接
    auto result = ShareService::createShareLink(postId, userId, expireDays);
    
    if (!result.success) {
        res.status = result.message == "帖子不存在" ? 404 : 500;
        res.set_content(buildResponse(false, result.message), "application/json");
        return;
    }
    
    // 5. 构造响应数据
    Json::Value data;
    data["short_code"] = result.shortCode;
    data["short_url"] = result.shortUrl;
    data["deep_links"] = result.deepLinks;
    data["is_reused"] = result.isReused;
    
    res.status = 200;
    res.set_content(buildResponse(true, result.message, data), "application/json");
    
    Logger::info("User " + std::to_string(userId) + " created share link for post " + 
                std::to_string(postId) + ": " + result.shortCode);
}

// 解析分享链接
void ShareHandler::handleResolveShareLink(const httplib::Request& req, httplib::Response& res) {
    res.set_header("Content-Type", "application/json");
    
    // 1. 获取短码
    if (req.matches.size() < 2) {
        res.status = 400;
        res.set_content(buildResponse(false, "无效的分享码"), "application/json");
        return;
    }
    
    std::string shortCode = req.matches[1].str();
    
    // 2. 调用服务解析
    auto result = ShareService::resolveShareLink(shortCode);
    
    if (!result.success) {
        if (result.expired) {
            res.status = 410;  // Gone
            res.set_content(buildResponse(false, result.message), "application/json");
        } else {
            res.status = 404;
            res.set_content(buildResponse(false, result.message), "application/json");
        }
        return;
    }
    
    // 3. 构造响应数据
    Json::Value data;
    
    if (result.post.has_value()) {
        data["post"] = result.post->toJson();
    }
    
    data["expired"] = result.expired;
    
    res.status = 200;
    res.set_content(buildResponse(true, result.message, data), "application/json");
    
    Logger::info("Resolved share link: " + shortCode);
}

// 构造统一JSON响应
std::string ShareHandler::buildResponse(bool success, const std::string& message, 
                                       const Json::Value& data) {
    Json::Value response;
    response["success"] = success;
    response["message"] = message;
    
    if (!data.isNull()) {
        response["data"] = data;
    }
    
    response["timestamp"] = static_cast<Json::Int64>(std::time(nullptr));
    
    Json::StreamWriterBuilder writer;
    writer["indentation"] = "";
    return Json::writeString(writer, response);
}

