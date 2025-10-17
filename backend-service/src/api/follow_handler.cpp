/**
 * @file follow_handler.cpp
 * @brief 关注API处理器实现
 * @author Knot Team
 * @date 2025-10-16
 */

#include "api/follow_handler.h"
#include "security/jwt_manager.h"
#include "utils/logger.h"
#include <json/json.h>

// 构造函数
FollowHandler::FollowHandler()
    : followService_(std::make_unique<FollowService>()) {
}

// 注册所有路由
void FollowHandler::registerRoutes(httplib::Server& server) {
    Logger::info("FollowHandler: Registering POST /api/v1/users/:user_id/follow");

    // POST /api/v1/users/:user_id/follow - 关注用户
    server.Post("/api/v1/users/:user_id/follow", [this](const httplib::Request& req, httplib::Response& res) {
        Logger::info("POST /api/v1/users/:user_id/follow lambda called - path: " + req.path);
        handleFollow(req, res);
    });

    // DELETE /api/v1/users/:user_id/follow - 取消关注
    server.Delete("/api/v1/users/:user_id/follow", [this](const httplib::Request& req, httplib::Response& res) {
        handleUnfollow(req, res);
    });

    // GET /api/v1/users/:user_id/follow/status - 检查关注关系
    server.Get("/api/v1/users/:user_id/follow/status", [this](const httplib::Request& req, httplib::Response& res) {
        handleCheckFollowStatus(req, res);
    });

    // GET /api/v1/users/:user_id/following - 获取关注列表
    server.Get("/api/v1/users/:user_id/following", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetFollowingList(req, res);
    });

    // GET /api/v1/users/:user_id/followers - 获取粉丝列表
    server.Get("/api/v1/users/:user_id/followers", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetFollowerList(req, res);
    });

    // GET /api/v1/users/:user_id/stats - 获取用户统计信息
    server.Get("/api/v1/users/:user_id/stats", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetUserStats(req, res);
    });

    // POST /api/v1/users/follow/batch-status - 批量检查关注关系
    server.Post("/api/v1/users/follow/batch-status", [this](const httplib::Request& req, httplib::Response& res) {
        handleBatchCheckFollowStatus(req, res);
    });

    Logger::info("Follow routes registered");
}

// POST /api/v1/users/:user_id/follow - 关注用户
void FollowHandler::handleFollow(const httplib::Request& req, httplib::Response& res) {
    Logger::info("FollowHandler::handleFollow called");
    
    try {
        // 1. 提取路径参数
        std::string followeeUserId = req.path_params.at("user_id");
        Logger::info("Path param user_id: " + followeeUserId);
        
        // 2. JWT验证
        std::string authHeader = req.get_header_value("Authorization");
        Logger::info("Authorization header: " + std::string(authHeader.empty() ? "[empty]" : "[present]"));
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }
        
        std::string token = authHeader.substr(7);
        auto jwtManager = std::make_unique<JWTManager>();
        
        if (!jwtManager->validateToken(token)) {
            sendErrorResponse(res, 401, "令牌无效或已过期");
            return;
        }
        
        Json::Value tokenData = jwtManager->decodeToken(token);
        int64_t followerId = std::stoll(tokenData["subject"].asString());
        
        // 3. 调用Service
        FollowResult result = followService_->followUser(followerId, followeeUserId);
        
        // 4. 构建响应
        Json::Value responseData;
        responseData["followee_user_id"] = followeeUserId;
        responseData["is_following"] = result.isFollowing;
        responseData["follower_count"] = result.followerCount;
        
        if (result.success) {
            sendJsonResponse(res, result.statusCode, true, result.message, responseData);
        } else {
            // 对于409冲突（已经关注过），也返回数据
            if (result.statusCode == 409) {
                sendJsonResponse(res, result.statusCode, false, result.message, responseData);
            } else {
                sendErrorResponse(res, result.statusCode, result.message);
            }
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleFollow: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// DELETE /api/v1/users/:user_id/follow - 取消关注
void FollowHandler::handleUnfollow(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 提取路径参数
        std::string followeeUserId = req.path_params.at("user_id");

        // 2. JWT验证
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }
        
        std::string token = authHeader.substr(7);
        auto jwtManager = std::make_unique<JWTManager>();
        
        if (!jwtManager->validateToken(token)) {
            sendErrorResponse(res, 401, "令牌无效或已过期");
            return;
        }
        
        Json::Value tokenData = jwtManager->decodeToken(token);
        int64_t followerId = std::stoll(tokenData["subject"].asString());
        
        // 3. 调用Service
        FollowResult result = followService_->unfollowUser(followerId, followeeUserId);
        
        // 4. 构建响应
        Json::Value responseData;
        responseData["followee_user_id"] = followeeUserId;
        responseData["is_following"] = result.isFollowing;
        responseData["follower_count"] = result.followerCount;
        
        if (result.success) {
            sendJsonResponse(res, result.statusCode, true, result.message, responseData);
        } else {
            // 对于404（未关注），也返回数据
            if (result.statusCode == 404) {
                sendJsonResponse(res, result.statusCode, false, result.message, responseData);
            } else {
                sendErrorResponse(res, result.statusCode, result.message);
            }
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleUnfollow: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// GET /api/v1/users/:user_id/follow/status - 检查关注关系
void FollowHandler::handleCheckFollowStatus(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 提取路径参数
        std::string followeeUserId = req.path_params.at("user_id");

        // 2. JWT验证
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }
        
        std::string token = authHeader.substr(7);
        auto jwtManager = std::make_unique<JWTManager>();
        
        if (!jwtManager->validateToken(token)) {
            sendErrorResponse(res, 401, "令牌无效或已过期");
            return;
        }
        
        Json::Value tokenData = jwtManager->decodeToken(token);
        int64_t followerId = std::stoll(tokenData["subject"].asString());
        
        // 3. 调用Service
        FollowStatusResult result = followService_->checkFollowStatus(followerId, followeeUserId);
        
        // 4. 构建响应
        Json::Value responseData;
        responseData["user_id"] = followeeUserId;
        responseData["is_following"] = result.isFollowing;
        responseData["is_followed_by"] = result.isFollowedBy;
        
        if (result.success) {
            sendJsonResponse(res, result.statusCode, true, result.message, responseData);
        } else {
            sendErrorResponse(res, result.statusCode, result.message);
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleCheckFollowStatus: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// GET /api/v1/users/:user_id/following - 获取关注列表
void FollowHandler::handleGetFollowingList(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 提取路径参数
        std::string userId = req.path_params.at("user_id");

        // 2. 提取查询参数
        int page = 1;
        int pageSize = 20;
        
        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
            if (page < 1) page = 1;
        }
        
        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
            if (pageSize < 1) pageSize = 20;
            if (pageSize > 100) pageSize = 100;  // 限制最大值
        }
        
        // 3. 可选JWT验证（用于标记is_following）
        int64_t currentUserId = 0;
        std::string authHeader = req.get_header_value("Authorization");
        
        if (!authHeader.empty() && authHeader.substr(0, 7) == "Bearer ") {
            std::string token = authHeader.substr(7);
            auto jwtManager = std::make_unique<JWTManager>();
            
            if (jwtManager->validateToken(token)) {
                Json::Value tokenData = jwtManager->decodeToken(token);
                currentUserId = std::stoll(tokenData["subject"].asString());
            }
        }
        
        // 4. 调用Service
        int total = 0;
        std::vector<UserListInfo> userList = followService_->getFollowingList(userId, currentUserId, page, pageSize, total);
        
        // 5. 构建响应
        Json::Value responseData;
        responseData["total"] = total;
        responseData["page"] = page;
        responseData["page_size"] = pageSize;
        
        Json::Value usersArray(Json::arrayValue);
        for (const auto& user : userList) {
            Json::Value userJson;
            userJson["user_id"] = user.userId;
            userJson["username"] = user.username;
            userJson["real_name"] = user.realName;
            userJson["avatar_url"] = user.avatarUrl;
            userJson["bio"] = user.bio;
            userJson["follower_count"] = user.followerCount;
            userJson["is_following"] = user.isFollowing;
            userJson["followed_at"] = static_cast<Json::Int64>(user.followedAt);
            usersArray.append(userJson);
        }
        responseData["users"] = usersArray;
        
        sendJsonResponse(res, 200, true, "查询成功", responseData);
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetFollowingList: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// GET /api/v1/users/:user_id/followers - 获取粉丝列表
void FollowHandler::handleGetFollowerList(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 提取路径参数
        std::string userId = req.path_params.at("user_id");

        // 2. 提取查询参数
        int page = 1;
        int pageSize = 20;
        
        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
            if (page < 1) page = 1;
        }
        
        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
            if (pageSize < 1) pageSize = 20;
            if (pageSize > 100) pageSize = 100;  // 限制最大值
        }
        
        // 3. 可选JWT验证（用于标记is_following）
        int64_t currentUserId = 0;
        std::string authHeader = req.get_header_value("Authorization");
        
        if (!authHeader.empty() && authHeader.substr(0, 7) == "Bearer ") {
            std::string token = authHeader.substr(7);
            auto jwtManager = std::make_unique<JWTManager>();
            
            if (jwtManager->validateToken(token)) {
                Json::Value tokenData = jwtManager->decodeToken(token);
                currentUserId = std::stoll(tokenData["subject"].asString());
            }
        }
        
        // 4. 调用Service
        int total = 0;
        std::vector<UserListInfo> userList = followService_->getFollowerList(userId, currentUserId, page, pageSize, total);
        
        // 5. 构建响应
        Json::Value responseData;
        responseData["total"] = total;
        responseData["page"] = page;
        responseData["page_size"] = pageSize;
        
        Json::Value usersArray(Json::arrayValue);
        for (const auto& user : userList) {
            Json::Value userJson;
            userJson["user_id"] = user.userId;
            userJson["username"] = user.username;
            userJson["real_name"] = user.realName;
            userJson["avatar_url"] = user.avatarUrl;
            userJson["bio"] = user.bio;
            userJson["follower_count"] = user.followerCount;
            userJson["is_following"] = user.isFollowing;
            userJson["followed_at"] = static_cast<Json::Int64>(user.followedAt);
            usersArray.append(userJson);
        }
        responseData["users"] = usersArray;
        
        sendJsonResponse(res, 200, true, "查询成功", responseData);
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetFollowerList: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// GET /api/v1/users/:user_id/stats - 获取用户统计信息
void FollowHandler::handleGetUserStats(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 提取路径参数
        std::string userId = req.path_params.at("user_id");

        // 2. 调用Service
        auto statsOpt = followService_->getUserStats(userId);
        
        if (!statsOpt.has_value()) {
            sendErrorResponse(res, 404, "用户不存在");
            return;
        }
        
        // 3. 构建响应
        Json::Value responseData = statsOpt->toJson();
        sendJsonResponse(res, 200, true, "查询成功", responseData);
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetUserStats: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// POST /api/v1/users/follow/batch-status - 批量检查关注关系
void FollowHandler::handleBatchCheckFollowStatus(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. JWT验证
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }
        
        std::string token = authHeader.substr(7);
        auto jwtManager = std::make_unique<JWTManager>();
        
        if (!jwtManager->validateToken(token)) {
            sendErrorResponse(res, 401, "令牌无效或已过期");
            return;
        }
        
        Json::Value tokenData = jwtManager->decodeToken(token);
        int64_t followerId = std::stoll(tokenData["subject"].asString());
        
        // 2. 解析请求体
        Json::Value requestJson;
        Json::Reader reader;
        
        if (!reader.parse(req.body, requestJson)) {
            sendErrorResponse(res, 400, "请求体格式错误");
            return;
        }
        
        if (!requestJson.isMember("user_ids") || !requestJson["user_ids"].isArray()) {
            sendErrorResponse(res, 400, "user_ids字段缺失或格式错误");
            return;
        }
        
        // 3. 提取用户ID列表
        std::vector<std::string> userIds;
        for (const auto& userId : requestJson["user_ids"]) {
            if (userId.isString()) {
                userIds.push_back(userId.asString());
            }
        }
        
        if (userIds.empty()) {
            sendErrorResponse(res, 400, "user_ids不能为空");
            return;
        }
        
        if (userIds.size() > 100) {
            sendErrorResponse(res, 400, "user_ids不能超过100个");
            return;
        }
        
        // 4. 调用Service
        std::map<std::string, bool> followMap = followService_->batchCheckFollowStatus(followerId, userIds);
        
        // 5. 构建响应
        Json::Value responseData;
        for (const auto& pair : followMap) {
            responseData[pair.first] = pair.second;
        }
        
        sendJsonResponse(res, 200, true, "查询成功", responseData);
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleBatchCheckFollowStatus: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}
