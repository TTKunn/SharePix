/**
 * @file comment_handler.cpp
 * @brief 评论API处理器实现
 * @author Knot Team
 * @date 2025-10-19
 */

#include "api/comment_handler.h"
#include "utils/logger.h"
#include <json/json.h>
#include <chrono>
#include <unordered_set>

// 构造函数
CommentHandler::CommentHandler() {
    commentService_ = std::make_unique<CommentService>();
    userService_ = std::make_unique<UserService>();
    Logger::info("CommentHandler initialized");
}

// 注册所有路由
void CommentHandler::registerRoutes(httplib::Server& server) {
    Logger::info("CommentHandler::registerRoutes - START");

    // 创建评论
    Logger::info("Registering: POST /api/v1/posts/:post_id/comments");
    server.Post("/api/v1/posts/:post_id/comments", [this](const httplib::Request& req, httplib::Response& res) {
        Logger::info("CommentHandler::handleCreateComment called");
        handleCreateComment(req, res);
    });

    // 获取评论列表
    Logger::info("Registering: GET /api/v1/posts/:post_id/comments");
    server.Get("/api/v1/posts/:post_id/comments", [this](const httplib::Request& req, httplib::Response& res) {
        Logger::info("CommentHandler::handleGetComments called");
        handleGetComments(req, res);
    });

    // 删除评论
    Logger::info("Registering: DELETE /api/v1/posts/:post_id/comments/:comment_id");
    server.Delete("/api/v1/posts/:post_id/comments/:comment_id", [this](const httplib::Request& req, httplib::Response& res) {
        Logger::info("CommentHandler::handleDeleteComment called");
        handleDeleteComment(req, res);
    });

    Logger::info("CommentHandler routes registered - COMPLETE");
}

// 处理创建评论请求
void CommentHandler::handleCreateComment(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并获取用户ID
        int userId = 0;
        if (!authenticateRequest(req, userId)) {
            sendJsonResponse(res, 401, false, "未提供认证令牌或令牌无效");
            return;
        }

        // 2. 获取路径参数post_id
        std::string postId = req.path_params.at("post_id");

        // 3. 解析请求体
        Json::Value requestBody;
        Json::CharReaderBuilder reader;
        std::string errs;
        std::istringstream requestStream(req.body);

        if (!Json::parseFromStream(reader, requestStream, &requestBody, &errs)) {
            Logger::warning("Failed to parse JSON: " + errs);
            sendJsonResponse(res, 400, false, "JSON格式错误");
            return;
        }

        // 4. 获取评论内容
        if (!requestBody.isMember("content") || !requestBody["content"].isString()) {
            sendJsonResponse(res, 400, false, "缺少必需参数：content");
            return;
        }

        std::string content = requestBody["content"].asString();

        // 5. 调用Service层创建评论
        CommentResult result = commentService_->createComment(userId, postId, content);

        // 6. 构建响应数据
        if (result.success && result.comment.has_value()) {
            Json::Value data;
            data["comment_id"] = result.comment->getCommentId();
            data["post_id"] = postId;
            data["content"] = result.comment->getContent();
            data["create_time"] = static_cast<Json::Int64>(result.comment->getCreateTime());
            data["comment_count"] = result.commentCount;

            // 查询用户信息
            std::vector<int> userIds = {userId};
            auto usersMap = userService_->batchGetUsers(userIds);
            if (usersMap.find(userId) != usersMap.end()) {
                const User& user = usersMap[userId];
                Json::Value author;
                author["user_id"] = user.getUserId();
                author["username"] = user.getUsername();
                author["avatar_url"] = user.getAvatarUrl();
                data["author"] = author;
            }

            sendJsonResponse(res, result.statusCode, result.success, result.message, data);
        } else {
            sendJsonResponse(res, result.statusCode, result.success, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleCreateComment: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理获取评论列表请求
void CommentHandler::handleGetComments(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 获取路径参数post_id
        std::string postId = req.path_params.at("post_id");

        // 2. 获取查询参数（分页）
        int page = 1;
        int pageSize = 20;

        if (req.has_param("page")) {
            try {
                page = std::stoi(req.get_param_value("page"));
            } catch (...) {
                page = 1;
            }
        }

        if (req.has_param("page_size")) {
            try {
                pageSize = std::stoi(req.get_param_value("page_size"));
            } catch (...) {
                pageSize = 20;
            }
        }

        // 3. 调用Service层获取评论列表
        CommentListResult result = commentService_->getCommentsByPost(postId, page, pageSize);

        // 4. 构建响应数据
        if (result.success) {
            // 批量查询用户信息
            std::vector<int> userIds;
            std::unordered_set<int> userIdSet;  // 去重

            for (const auto& comment : result.comments) {
                if (userIdSet.find(comment.getUserId()) == userIdSet.end()) {
                    userIds.push_back(comment.getUserId());
                    userIdSet.insert(comment.getUserId());
                }
            }

            auto usersMap = userService_->batchGetUsers(userIds);

            // 构建评论JSON数组
            Json::Value commentsArray;
            for (const auto& comment : result.comments) {
                Json::Value commentJson;
                commentJson["comment_id"] = comment.getCommentId();
                commentJson["content"] = comment.getContent();
                commentJson["create_time"] = static_cast<Json::Int64>(comment.getCreateTime());

                // 添加作者信息
                int userId = comment.getUserId();
                if (usersMap.find(userId) != usersMap.end()) {
                    const User& user = usersMap[userId];
                    Json::Value author;
                    author["user_id"] = user.getUserId();
                    author["username"] = user.getUsername();
                    author["avatar_url"] = user.getAvatarUrl();
                    commentJson["author"] = author;
                }

                commentsArray.append(commentJson);
            }

            Json::Value data;
            data["comments"] = commentsArray;
            data["total"] = result.total;
            data["page"] = page;
            data["page_size"] = pageSize;
            data["has_more"] = result.hasMore;

            sendJsonResponse(res, result.statusCode, result.success, result.message, data);
        } else {
            sendJsonResponse(res, result.statusCode, result.success, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetComments: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理删除评论请求
void CommentHandler::handleDeleteComment(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并获取用户ID
        int userId = 0;
        if (!authenticateRequest(req, userId)) {
            sendJsonResponse(res, 401, false, "未提供认证令牌或令牌无效");
            return;
        }

        // 2. 获取路径参数
        std::string postId = req.path_params.at("post_id");
        std::string commentId = req.path_params.at("comment_id");

        // 3. 调用Service层删除评论
        CommentResult result = commentService_->deleteComment(userId, commentId);

        // 4. 构建响应数据
        if (result.success) {
            Json::Value data;
            data["comment_id"] = commentId;
            data["comment_count"] = result.commentCount;

            sendJsonResponse(res, result.statusCode, result.success, result.message, data);
        } else {
            sendJsonResponse(res, result.statusCode, result.success, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleDeleteComment: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}
