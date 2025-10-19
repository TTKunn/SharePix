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
    userService_ = std::make_unique<UserService>();         // 初始化用户服务
    likeService_ = std::make_unique<LikeService>();         // 初始化点赞服务
    Logger::info("FavoriteHandler initialized with all services");
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
        Logger::info("=== [GET FAVORITES] Request received ===");

        // ========================================
        // 第1步: JWT认证（必需）
        // ========================================
        int currentUserId = 0;
        if (!authenticateRequest(req, currentUserId)) {
            Logger::warning("[GET FAVORITES] ✗ Authentication failed");
            sendJsonResponse(res, 401, false, "未提供认证令牌或令牌无效");
            return;
        }

        Logger::info("[GET FAVORITES] ✓ User authenticated - UserID: " + std::to_string(currentUserId));

        // ========================================
        // 第2步: 获取分页参数
        // ========================================
        int page = 1;
        int pageSize = 20;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }
        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
        }

        // 验证参数
        if (page < 1) page = 1;
        if (pageSize < 1 || pageSize > 100) pageSize = 20;

        Logger::info("[GET FAVORITES] 📄 Pagination: page=" + std::to_string(page) +
                     ", page_size=" + std::to_string(pageSize));

        // ========================================
        // 第3步: 查询收藏的帖子列表（基础数据）
        // ========================================
        auto startTime = std::chrono::steady_clock::now();

        FavoriteListResult result = favoriteService_->getUserFavorites(currentUserId, page, pageSize);

        if (!result.success) {
            Logger::error("[GET FAVORITES] ✗ Failed to query favorites: " + result.message);
            sendJsonResponse(res, result.statusCode, false, result.message);
            return;
        }

        Logger::info("[GET FAVORITES] ✓ Found " + std::to_string(result.posts.size()) + " posts");

        // ========================================
        // 第4步: 收集需要批量查询的ID
        // ========================================
        std::vector<int> postIds;
        std::vector<int> authorIds;

        for (const auto& post : result.posts) {
            postIds.push_back(post.getId());
            authorIds.push_back(post.getUserId());
        }

        Logger::info("[GET FAVORITES] 📦 Collected " + std::to_string(postIds.size()) +
                     " post IDs, " + std::to_string(authorIds.size()) + " author IDs");

        // ========================================
        // 第5步: 批量查询作者信息
        // ========================================
        std::unordered_map<int, User> authorMap;

        if (!authorIds.empty()) {
            authorMap = userService_->batchGetUsers(authorIds);
            Logger::info("[GET FAVORITES] ✓ Batch fetched " + std::to_string(authorMap.size()) + " authors");
        }

        // ========================================
        // 第6步: 批量查询点赞状态
        // ========================================
        std::unordered_map<int, bool> likeStatusMap;

        if (!postIds.empty()) {
            likeStatusMap = likeService_->batchCheckLikedStatus(currentUserId, postIds);
            Logger::info("[GET FAVORITES] ✓ Batch checked like status for " + std::to_string(likeStatusMap.size()) + " posts");
        }

        // ========================================
        // 第7步: 组装JSON响应
        // ========================================
        Json::Value data;
        Json::Value postsArray(Json::arrayValue);

        for (const auto& post : result.posts) {
            Json::Value postJson = post.toJson();

            // 添加作者信息
            auto authorIt = authorMap.find(post.getUserId());
            if (authorIt != authorMap.end()) {
                Json::Value authorInfo;
                authorInfo["user_id"] = authorIt->second.getUserId();  // 逻辑ID
                authorInfo["username"] = authorIt->second.getUsername();
                authorInfo["avatar_url"] = authorIt->second.getAvatarUrl();
                postJson["author"] = authorInfo;
            } else {
                // 作者信息缺失时的降级处理
                Json::Value authorInfo;
                authorInfo["user_id"] = "";
                authorInfo["username"] = "Unknown";
                authorInfo["avatar_url"] = "";
                postJson["author"] = authorInfo;
                Logger::warning("[GET FAVORITES] ⚠ Author not found for user_id=" + std::to_string(post.getUserId()));
            }

            // 添加互动状态（字段必须存在）
            postJson["has_liked"] = likeStatusMap[post.getId()];       // 查询结果
            postJson["has_favorited"] = true;                          // 固定为true（收藏列表）

            postsArray.append(postJson);
        }

        data["posts"] = postsArray;
        data["total"] = result.total;
        data["page"] = page;
        data["page_size"] = pageSize;
        data["total_pages"] = (result.total + pageSize - 1) / pageSize;

        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

        Logger::info("[GET FAVORITES] ✓ Response assembled successfully in " + std::to_string(duration) + "ms");
        Logger::info("[GET FAVORITES] 📊 Performance: " + std::to_string(result.posts.size()) +
                     " posts, 3 queries (1 posts + 1 authors + 1 likes)");

        // ========================================
        // 第8步: 发送响应
        // ========================================
        sendJsonResponse(res, 200, true, "查询成功", data);

    } catch (const std::exception& e) {
        Logger::error("[GET FAVORITES] ✗ Exception: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}
