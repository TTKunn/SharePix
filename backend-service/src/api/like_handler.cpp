/**
 * @file like_handler.cpp
 * @brief 点赞API处理器实现
 * @author Knot Team
 * @date 2025-10-10
 */

#include "api/like_handler.h"
#include "utils/logger.h"
#include <json/json.h>
#include <chrono>

// 构造函数
LikeHandler::LikeHandler() {
    likeService_ = std::make_unique<LikeService>();
    Logger::info("LikeHandler initialized");
}

// 注册所有路由
void LikeHandler::registerRoutes(httplib::Server& server) {
    // 点赞帖子
    server.Post("/api/v1/posts/:post_id/like", [this](const httplib::Request& req, httplib::Response& res) {
        handleLike(req, res);
    });

    // 取消点赞
    server.Delete("/api/v1/posts/:post_id/like", [this](const httplib::Request& req, httplib::Response& res) {
        handleUnlike(req, res);
    });

    Logger::info("LikeHandler routes registered");
}

// 处理点赞请求
void LikeHandler::handleLike(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并获取用户ID
        int userId = 0;
        if (!authenticateRequest(req, userId)) {
            sendJsonResponse(res, 401, false, "未提供认证令牌或令牌无效");
            return;
        }

        // 2. 获取路径参数post_id
        std::string postId;
        if (req.path_params.count("post_id") > 0) {
            postId = req.path_params.at("post_id");
        } else {
            sendJsonResponse(res, 400, false, "缺少帖子ID");
            return;
        }

        // 3. 调用Service层进行点赞
        LikeResult result = likeService_->likePost(userId, postId);

        // 4. 构建响应数据
        Json::Value data;
        data["post_id"] = postId;
        data["like_count"] = result.likeCount;
        data["has_liked"] = result.hasLiked;

        // 5. 发送响应
        sendJsonResponse(res, result.statusCode, result.success, result.message, data);

    } catch (const std::exception& e) {
        Logger::error("Exception in handleLike: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理取消点赞请求
void LikeHandler::handleUnlike(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并获取用户ID
        int userId = 0;
        if (!authenticateRequest(req, userId)) {
            sendJsonResponse(res, 401, false, "未提供认证令牌或令牌无效");
            return;
        }

        // 2. 获取路径参数post_id
        std::string postId;
        if (req.path_params.count("post_id") > 0) {
            postId = req.path_params.at("post_id");
        } else {
            sendJsonResponse(res, 400, false, "缺少帖子ID");
            return;
        }

        // 3. 调用Service层进行取消点赞
        LikeResult result = likeService_->unlikePost(userId, postId);

        // 4. 构建响应数据
        Json::Value data;
        data["post_id"] = postId;
        data["like_count"] = result.likeCount;
        data["has_liked"] = result.hasLiked;

        // 5. 发送响应
        sendJsonResponse(res, result.statusCode, result.success, result.message, data);

    } catch (const std::exception& e) {
        Logger::error("Exception in handleUnlike: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}
