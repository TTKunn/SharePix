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

    // 查询收藏状态
    server.Get("/api/v1/posts/:post_id/favorite/status", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetFavoriteStatus(req, res);
    });

    // 获取用户收藏列表
    server.Get("/api/v1/my/favorites", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetUserFavorites(req, res);
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
        std::string postId = req.path_params.at("post_id");

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
        std::string postId = req.path_params.at("post_id");

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

// 处理查询收藏状态请求
void FavoriteHandler::handleGetFavoriteStatus(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并获取用户ID
        int userId = 0;
        if (!authenticateRequest(req, userId)) {
            sendJsonResponse(res, 401, false, "未提供认证令牌或令牌无效");
            return;
        }

        // 2. 获取路径参数post_id
        std::string postId = req.path_params.at("post_id");

        // 3. 调用Service层查询收藏状态
        FavoriteStatusResult result = favoriteService_->getFavoriteStatus(userId, postId);

        // 4. 构建响应数据
        Json::Value data;
        data["post_id"] = postId;
        data["has_favorited"] = result.hasFavorited;
        data["favorite_count"] = result.favoriteCount;

        // 5. 发送响应
        sendJsonResponse(res, result.statusCode, result.success, result.message, data);

    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetFavoriteStatus: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理获取用户收藏列表请求
void FavoriteHandler::handleGetUserFavorites(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并获取用户ID
        int userId = 0;
        if (!authenticateRequest(req, userId)) {
            sendJsonResponse(res, 401, false, "未提供认证令牌或令牌无效");
            return;
        }

        // 2. 获取分页参数
        int page = 1;
        int pageSize = 20;
        
        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }
        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
        }

        // 验证分页参数
        if (page < 1) page = 1;
        if (pageSize < 1 || pageSize > 100) pageSize = 20;

        Logger::info("Getting favorites for user " + std::to_string(userId) + 
                     ", page=" + std::to_string(page) + ", pageSize=" + std::to_string(pageSize));

        // 3. 调用Service层获取收藏列表
        FavoriteListResult result = favoriteService_->getUserFavorites(userId, page, pageSize);

        // 4. 构建响应数据
        Json::Value data;
        data["posts"] = Json::Value(Json::arrayValue);
        
        for (const auto& post : result.posts) {
            data["posts"].append(post.toJson());
        }
        
        data["total"] = result.total;
        data["page"] = page;
        data["page_size"] = pageSize;
        data["total_pages"] = (result.total + pageSize - 1) / pageSize;

        // 5. 发送响应
        sendJsonResponse(res, result.statusCode, result.success, result.message, data);

    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetUserFavorites: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}
