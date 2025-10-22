/**
 * @file share_handler.cpp
 * @brief 分享API处理器实现
 * @author Claude Code Assistant
 * @date 2025-10-22
 * @version v2.10.0
 */

#include "api/share_handler.h"
#include "core/share_service.h"
#include "security/jwt_manager.h"
#include "utils/logger.h"
#include <json/json.h>
#include <sstream>

// 构造函数
ShareHandler::ShareHandler()
    : shareService_(std::make_unique<ShareService>())
    , jwtManager_(std::make_unique<JWTManager>()) {
}

// 析构函数
ShareHandler::~ShareHandler() = default;

// 注册路由
void ShareHandler::registerRoutes(httplib::Server& server) {
    // POST /api/v1/shares/posts - 创建分享记录
    server.Post("/api/v1/shares/posts", [this](const httplib::Request& req, httplib::Response& res) {
        handleCreateShare(req, res);
    });

    // GET /api/v1/shares/received - 获取收到的分享列表
    server.Get("/api/v1/shares/received", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetReceivedShares(req, res);
    });

    // GET /api/v1/shares/sent - 获取发出的分享列表
    server.Get("/api/v1/shares/sent", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetSentShares(req, res);
    });

    // DELETE /api/v1/shares/:id - 删除分享记录
    server.Delete("/api/v1/shares/:id", [this](const httplib::Request& req, httplib::Response& res) {
        handleDeleteShare(req, res);
    });

    Logger::info("ShareHandler routes registered");
}

// 从JWT令牌中提取用户ID
int ShareHandler::extractUserIdFromToken(const httplib::Request& req) {
    // 提取Authorization header
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ") {
        return 0;
    }

    std::string token = authHeader.substr(7);
    auto jwtManager = std::make_unique<JWTManager>();

    if (!jwtManager->validateToken(token)) {
        return 0;
    }

    Json::Value tokenData = jwtManager->decodeToken(token);
    int64_t userId = std::stoll(tokenData["subject"].asString());

    return static_cast<int>(userId);
}

// 构建JSON响应
std::string ShareHandler::buildJsonResponse(bool success, const std::string& message, const Json::Value& data) {
    Json::Value response;
    response["success"] = success;
    response["message"] = message;

    if (!data.isNull()) {
        response["data"] = data;
    }

    response["timestamp"] = static_cast<Json::Int64>(std::time(nullptr));

    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, response);
}

// 发送错误响应
void ShareHandler::sendErrorResponse(httplib::Response& res, int statusCode, const std::string& message) {
    res.status = statusCode;
    res.set_content(buildJsonResponse(false, message), "application/json");
}

// 创建分享记录
void ShareHandler::handleCreateShare(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并提取用户ID
        int senderId = extractUserIdFromToken(req);
        if (senderId == 0) {
            sendErrorResponse(res, 401, "未授权：请先登录");
            return;
        }

        // 2. 解析请求体
        Json::Value requestJson;
        Json::CharReaderBuilder readerBuilder;
        std::string errs;
        std::istringstream iss(req.body);

        if (!Json::parseFromStream(readerBuilder, iss, &requestJson, &errs)) {
            sendErrorResponse(res, 400, "请求体格式错误");
            return;
        }

        // 3. 提取参数
        if (!requestJson.isMember("post_id") || !requestJson.isMember("receiver_id")) {
            sendErrorResponse(res, 400, "缺少必需参数: post_id 或 receiver_id");
            return;
        }

        // 提取业务ID参数（字符串格式）
        std::string postId = requestJson["post_id"].asString();
        std::string receiverId = requestJson["receiver_id"].asString();
        std::string shareMessage = requestJson.get("share_message", "").asString();

        // 验证业务ID格式
        if (postId.empty() || receiverId.empty()) {
            sendErrorResponse(res, 400, "参数不能为空: post_id 和 receiver_id");
            return;
        }

        // 4. 调用Service创建分享（传入业务ID）
        auto result = shareService_->createShare(senderId, postId, receiverId, shareMessage);

        // 5. 构建响应
        if (result.success) {
            Json::Value data;
            data["share_id"] = result.shareId;
            data["create_time"] = result.createTime;

            res.status = result.statusCode;
            res.set_content(buildJsonResponse(true, result.message, data), "application/json");
        } else {
            sendErrorResponse(res, result.statusCode, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleCreateShare: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// 获取收到的分享列表
void ShareHandler::handleGetReceivedShares(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并提取用户ID
        int receiverId = extractUserIdFromToken(req);
        if (receiverId == 0) {
            sendErrorResponse(res, 401, "未授权：请先登录");
            return;
        }

        // 2. 提取分页参数
        int page = 1;
        int pageSize = 20;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }

        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
        }

        // 3. 调用Service查询分享列表
        auto result = shareService_->getReceivedShares(receiverId, page, pageSize);

        // 4. 构建响应
        if (result.success) {
            Json::Value data;
            Json::Value sharesArray(Json::arrayValue);

            for (const auto& item : result.shares) {
                Json::Value shareJson;
                shareJson["share_id"] = item.shareId;
                shareJson["share_message"] = item.shareMessage;
                shareJson["create_time"] = item.createTime;

                // 帖子信息
                Json::Value postJson;
                postJson["id"] = item.post.id;
                postJson["post_id"] = item.post.postId;
                postJson["title"] = item.post.title;
                postJson["description"] = item.post.description;
                postJson["cover_image"] = item.post.coverImage;
                postJson["like_count"] = item.post.likeCount;
                postJson["favorite_count"] = item.post.favoriteCount;
                shareJson["post"] = postJson;

                // 发送者信息
                Json::Value senderJson;
                senderJson["id"] = item.sender.id;
                senderJson["user_id"] = item.sender.userId;
                senderJson["username"] = item.sender.username;
                senderJson["avatar_url"] = item.sender.avatarUrl;
                senderJson["bio"] = item.sender.bio;
                shareJson["sender"] = senderJson;

                sharesArray.append(shareJson);
            }

            data["shares"] = sharesArray;
            data["total"] = result.total;
            data["page"] = result.page;
            data["page_size"] = result.pageSize;
            data["has_more"] = result.hasMore;

            res.status = result.statusCode;
            res.set_content(buildJsonResponse(true, result.message, data), "application/json");
        } else {
            sendErrorResponse(res, result.statusCode, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetReceivedShares: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// 获取发出的分享列表
void ShareHandler::handleGetSentShares(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并提取用户ID
        int senderId = extractUserIdFromToken(req);
        if (senderId == 0) {
            sendErrorResponse(res, 401, "未授权：请先登录");
            return;
        }

        // 2. 提取分页参数
        int page = 1;
        int pageSize = 20;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }

        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
        }

        // 3. 调用Service查询分享列表
        auto result = shareService_->getSentShares(senderId, page, pageSize);

        // 4. 构建响应（与handleGetReceivedShares相同的逻辑）
        if (result.success) {
            Json::Value data;
            Json::Value sharesArray(Json::arrayValue);

            for (const auto& item : result.shares) {
                Json::Value shareJson;
                shareJson["share_id"] = item.shareId;
                shareJson["share_message"] = item.shareMessage;
                shareJson["create_time"] = item.createTime;

                // 帖子信息
                Json::Value postJson;
                postJson["id"] = item.post.id;
                postJson["post_id"] = item.post.postId;
                postJson["title"] = item.post.title;
                postJson["description"] = item.post.description;
                postJson["cover_image"] = item.post.coverImage;
                postJson["like_count"] = item.post.likeCount;
                postJson["favorite_count"] = item.post.favoriteCount;
                shareJson["post"] = postJson;

                // 接收者信息（在sent列表中，sender字段实际存的是receiver信息）
                Json::Value receiverJson;
                receiverJson["id"] = item.sender.id;
                receiverJson["user_id"] = item.sender.userId;
                receiverJson["username"] = item.sender.username;
                receiverJson["avatar_url"] = item.sender.avatarUrl;
                receiverJson["bio"] = item.sender.bio;
                shareJson["receiver"] = receiverJson;  // 注意：这里改为receiver

                sharesArray.append(shareJson);
            }

            data["shares"] = sharesArray;
            data["total"] = result.total;
            data["page"] = result.page;
            data["page_size"] = result.pageSize;
            data["has_more"] = result.hasMore;

            res.status = result.statusCode;
            res.set_content(buildJsonResponse(true, result.message, data), "application/json");
        } else {
            sendErrorResponse(res, result.statusCode, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetSentShares: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// 删除分享记录
void ShareHandler::handleDeleteShare(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌并提取用户ID
        int operatorId = extractUserIdFromToken(req);
        if (operatorId == 0) {
            sendErrorResponse(res, 401, "未授权：请先登录");
            return;
        }

        // 2. 提取路径参数
        auto it = req.path_params.find("id");
        if (it == req.path_params.end()) {
            sendErrorResponse(res, 400, "缺少参数: id");
            return;
        }

        int shareId = std::stoi(it->second);

        // 3. 调用Service删除分享
        auto result = shareService_->deleteShare(shareId, operatorId);

        // 4. 构建响应
        if (result.success) {
            res.status = result.statusCode;
            res.set_content(buildJsonResponse(true, result.message), "application/json");
        } else {
            sendErrorResponse(res, result.statusCode, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleDeleteShare: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}
