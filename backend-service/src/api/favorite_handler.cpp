/**
 * @file favorite_handler.cpp
 * @brief 收藏API处理器实现
 * @author Knot Team
 * @date 2025-10-10
 */

#include "api/favorite_handler.h"
#include "utils/logger.h"
#include <json/json.h>
#include <chrono>

// 构造函数
FavoriteHandler::FavoriteHandler() {
    favoriteService_ = std::make_unique<FavoriteService>();
    Logger::info("FavoriteHandler initialized");
}

// 注册所有路由
void FavoriteHandler::registerRoutes(httplib::Server& server) {
    // 收藏帖子
    server.Post("/api/v1/posts/:post_id/favorite", [this](const httplib::Request& req, httplib::Response& res) {
        handleFavorite(req, res);
    });

    // 取消收藏
    server.Delete("/api/v1/posts/:post_id/favorite", [this](const httplib::Request& req, httplib::Response& res) {
        handleUnfavorite(req, res);
    });

    Logger::info("FavoriteHandler routes registered");
}

// 处理收藏请求
void FavoriteHandler::handleFavorite(const httplib::Request& req, httplib::Response& res) {
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

        Logger::info("User " + std::to_string(userId) + " attempting to favorite post: " + postId);

        // 3. 调用Service层进行收藏
        FavoriteResult result = favoriteService_->favoritePost(userId, postId);

        // 4. 构建响应数据
        Json::Value data;
        data["post_id"] = postId;
        data["favorite_count"] = result.favoriteCount;
        data["has_favorited"] = result.hasFavorited;

        // 5. 发送响应
        sendJsonResponse(res, result.statusCode, result.success, result.message, data);

    } catch (const std::exception& e) {
        Logger::error("Exception in handleFavorite: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理取消收藏请求
void FavoriteHandler::handleUnfavorite(const httplib::Request& req, httplib::Response& res) {
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

        Logger::info("User " + std::to_string(userId) + " attempting to unfavorite post: " + postId);

        // 3. 调用Service层进行取消收藏
        FavoriteResult result = favoriteService_->unfavoritePost(userId, postId);

        // 4. 构建响应数据
        Json::Value data;
        data["post_id"] = postId;
        data["favorite_count"] = result.favoriteCount;
        data["has_favorited"] = result.hasFavorited;

        // 5. 发送响应
        sendJsonResponse(res, result.statusCode, result.success, result.message, data);

    } catch (const std::exception& e) {
        Logger::error("Exception in handleUnfavorite: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}
