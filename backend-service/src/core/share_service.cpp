/**
 * @file share_service.cpp
 * @brief 分享服务 - 业务逻辑层实现
 * @author Claude Code Assistant
 * @date 2025-10-22
 * @version v2.10.0
 */

#include "core/share_service.h"
#include "database/share_repository.h"
#include "database/follow_repository.h"
#include "database/post_repository.h"
#include "database/user_repository.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include "utils/logger.h"
#include "models/post.h"
#include "models/user.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>

// 构造函数
ShareService::ShareService()
    : shareRepo_(std::make_unique<ShareRepository>())
    , followRepo_(std::make_unique<FollowRepository>())
    , postRepo_(std::make_unique<PostRepository>())
    , userRepo_(std::make_unique<UserRepository>()) {
}

// 析构函数
ShareService::~ShareService() = default;

// 生成分享业务ID
std::string ShareService::generateShareId() {
    // 格式：SHR_2025Q4_ABC123
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);

    // 计算季度
    int quarter = (tm->tm_mon / 3) + 1;

    // 生成随机字符串（6位）
    static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

    std::string randomStr;
    for (int i = 0; i < 6; ++i) {
        randomStr += alphanum[dis(gen)];
    }

    std::ostringstream oss;
    oss << "SHR_" << (1900 + tm->tm_year) << "Q" << quarter << "_" << randomStr;

    return oss.str();
}

// 检查两个用户是否互相关注
bool ShareService::checkMutualFollow(int userId1, int userId2) {
    try {
        ConnectionGuard guard(DatabaseConnectionPool::getInstance());
        if (!guard.isValid()) {
            Logger::error("Failed to get database connection in checkMutualFollow");
            return false;
        }

        MYSQL* conn = guard.get();

        // 检查 userId1 是否关注 userId2
        bool user1FollowsUser2 = followRepo_->exists(conn, userId1, userId2);
        if (!user1FollowsUser2) {
            return false;
        }

        // 检查 userId2 是否关注 userId1
        bool user2FollowsUser1 = followRepo_->exists(conn, userId2, userId1);
        if (!user2FollowsUser1) {
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in checkMutualFollow: " + std::string(e.what()));
        return false;
    }
}

// 批量获取帖子信息
std::map<int, ShareListItem::PostInfo> ShareService::batchGetPostInfo(const std::vector<int>& postIds) {
    std::map<int, ShareListItem::PostInfo> postInfoMap;

    if (postIds.empty()) {
        return postInfoMap;
    }

    try {
        ConnectionGuard guard(DatabaseConnectionPool::getInstance());
        if (!guard.isValid()) {
            Logger::error("Failed to get database connection in batchGetPostInfo");
            return postInfoMap;
        }

        MYSQL* conn = guard.get();

        // 构建IN查询（使用物理ID）
        std::string idsStr;
        for (size_t i = 0; i < postIds.size(); ++i) {
            if (i > 0) idsStr += ",";
            idsStr += std::to_string(postIds[i]);
        }

        std::string query = "SELECT p.id, p.post_id, p.title, p.description, p.like_count, p.favorite_count, "
                           "i.thumbnail_url FROM posts p "
                           "LEFT JOIN (SELECT post_id, thumbnail_url FROM images WHERE display_order = 0) i "
                           "ON p.id = i.post_id WHERE p.id IN (" + idsStr + ")";

        if (mysql_query(conn, query.c_str()) != 0) {
            Logger::error("Failed to execute batch query: " + std::string(mysql_error(conn)));
            return postInfoMap;
        }

        MYSQL_RES* result = mysql_store_result(conn);
        if (!result) {
            Logger::error("Failed to store result: " + std::string(mysql_error(conn)));
            return postInfoMap;
        }

        MYSQL_ROW row;
        while ((row = mysql_fetch_row(result))) {
            ShareListItem::PostInfo info;
            info.id = std::stoi(row[0]);
            info.postId = row[1] ? row[1] : "";
            info.title = row[2] ? row[2] : "";
            info.description = row[3] ? row[3] : "";
            info.likeCount = row[4] ? std::stoi(row[4]) : 0;
            info.favoriteCount = row[5] ? std::stoi(row[5]) : 0;
            info.coverImage = row[6] ? row[6] : "";

            postInfoMap[info.id] = info;
        }

        mysql_free_result(result);
        Logger::debug("Batch loaded " + std::to_string(postInfoMap.size()) + " posts");

    } catch (const std::exception& e) {
        Logger::error("Exception in batchGetPostInfo: " + std::string(e.what()));
    }

    return postInfoMap;
}

// 批量获取用户信息
std::map<int, ShareListItem::SenderInfo> ShareService::batchGetUserInfo(const std::vector<int>& userIds) {
    std::map<int, ShareListItem::SenderInfo> userInfoMap;

    if (userIds.empty()) {
        return userInfoMap;
    }

    try {
        ConnectionGuard guard(DatabaseConnectionPool::getInstance());
        if (!guard.isValid()) {
            Logger::error("Failed to get database connection in batchGetUserInfo");
            return userInfoMap;
        }

        MYSQL* conn = guard.get();

        // 批量查询用户（使用UserRepository的查询方法）
        for (int userId : userIds) {
            auto userOpt = userRepo_->findById(userId);
            if (userOpt.has_value()) {
                User user = userOpt.value();
                ShareListItem::SenderInfo info;
                info.id = user.getId();
                info.userId = user.getUserId();
                info.username = user.getUsername();
                info.avatarUrl = user.getAvatarUrl();
                info.bio = user.getBio();

                userInfoMap[userId] = info;
            }
        }

        Logger::debug("Batch loaded " + std::to_string(userInfoMap.size()) + " users");

    } catch (const std::exception& e) {
        Logger::error("Exception in batchGetUserInfo: " + std::string(e.what()));
    }

    return userInfoMap;
}

// 组装分享列表项
std::vector<ShareListItem> ShareService::assembleShareListItems(
    const std::vector<Share>& shares,
    const std::map<int, ShareListItem::PostInfo>& postInfoMap,
    const std::map<int, ShareListItem::SenderInfo>& userInfoMap
) {
    std::vector<ShareListItem> items;

    for (const auto& share : shares) {
        ShareListItem item;

        // 设置分享基础信息
        item.shareId = share.getShareId();
        item.shareMessage = share.getShareMessage();
        item.createTime = static_cast<int64_t>(share.getCreateTime());

        // 查找帖子信息
        auto postIt = postInfoMap.find(share.getPostId());
        if (postIt != postInfoMap.end()) {
            item.post = postIt->second;
        } else {
            // 帖子不存在（已被删除），跳过此分享记录
            Logger::warning("Post not found for share_id=" + share.getShareId() +
                          ", post_id=" + std::to_string(share.getPostId()));
            continue;
        }

        // 查找发送者信息
        auto userIt = userInfoMap.find(share.getSenderId());
        if (userIt != userInfoMap.end()) {
            item.sender = userIt->second;
        } else {
            // 用户不存在（已被删除），跳过此分享记录
            Logger::warning("User not found for share_id=" + share.getShareId() +
                          ", sender_id=" + std::to_string(share.getSenderId()));
            continue;
        }

        items.push_back(item);
    }

    return items;
}

// 创建分享记录
CreateShareResult ShareService::createShare(int senderId, const std::string& postId, const std::string& receiverId, const std::string& shareMessage) {
    CreateShareResult result;

    try {
        // 1. 验证基本参数
        if (senderId <= 0 || postId.empty() || receiverId.empty()) {
            result.statusCode = 400;
            result.message = "参数无效";
            return result;
        }

        // 2. 验证分享附言长度
        if (shareMessage.length() > 500) {
            result.statusCode = 400;
            result.message = "分享附言过长（最多500字符）";
            return result;
        }

        // 3. 通过业务ID查询帖子
        auto postOpt = postRepo_->findByPostId(postId);
        if (!postOpt.has_value()) {
            result.statusCode = 404;
            result.message = "帖子不存在";
            return result;
        }
        int postPhysicalId = postOpt.value().getId();

        // 4. 通过业务ID查询接收者用户
        auto receiverOpt = userRepo_->findByUserId(receiverId);
        if (!receiverOpt.has_value()) {
            result.statusCode = 404;
            result.message = "接收者不存在";
            return result;
        }
        int receiverPhysicalId = receiverOpt.value().getId();

        // 5. 验证不能分享给自己
        if (senderId == receiverPhysicalId) {
            result.statusCode = 400;
            result.message = "不能分享给自己";
            return result;
        }

        ConnectionGuard guard(DatabaseConnectionPool::getInstance());
        if (!guard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = guard.get();

        // 6. 验证是否互相关注（使用物理ID）
        if (!checkMutualFollow(senderId, receiverPhysicalId)) {
            result.statusCode = 403;
            result.message = "只能分享给互相关注的用户";
            return result;
        }

        // 7. 验证是否已分享过（防重复，使用物理ID）
        if (shareRepo_->exists(conn, senderId, receiverPhysicalId, postPhysicalId)) {
            result.statusCode = 409;
            result.message = "已分享过此帖子给该用户";
            return result;
        }

        // 8. 生成业务ID
        std::string shareId = generateShareId();

        // 9. 创建分享记录（使用物理ID）
        Share share;
        share.setShareId(shareId);
        share.setPostId(postPhysicalId);
        share.setSenderId(senderId);
        share.setReceiverId(receiverPhysicalId);
        share.setShareMessage(shareMessage);

        int insertedId = shareRepo_->create(conn, share);
        if (insertedId <= 0) {
            result.statusCode = 500;
            result.message = "创建分享记录失败";
            return result;
        }

        // 10. 返回成功结果
        result.success = true;
        result.statusCode = 201;
        result.message = "分享成功";
        result.shareId = shareId;
        result.createTime = static_cast<int64_t>(std::time(nullptr));

        Logger::info("Share created: shareId=" + shareId +
                    ", sender=" + std::to_string(senderId) +
                    ", receiver=" + std::to_string(receiverPhysicalId) +
                    ", post=" + std::to_string(postPhysicalId));

    } catch (const std::exception& e) {
        Logger::error("Exception in createShare: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
    }

    return result;
}

// 获取收到的分享列表
ShareListResult ShareService::getReceivedShares(int receiverId, int page, int pageSize) {
    ShareListResult result;

    try {
        // 1. 验证参数
        if (receiverId <= 0) {
            result.statusCode = 400;
            result.message = "用户ID无效";
            return result;
        }

        if (page < 1) {
            page = 1;
        }

        if (pageSize < 1 || pageSize > 50) {
            pageSize = 20;  // 默认20，最大50
        }

        ConnectionGuard guard(DatabaseConnectionPool::getInstance());
        if (!guard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = guard.get();

        // 2. 查询总数
        int total = shareRepo_->countReceivedShares(conn, receiverId);

        // 3. 查询分享列表（基础数据）
        int offset = (page - 1) * pageSize;
        std::vector<Share> shares = shareRepo_->findReceivedShares(conn, receiverId, pageSize, offset);

        if (shares.empty()) {
            result.success = true;
            result.statusCode = 200;
            result.message = "查询成功";
            result.total = total;
            result.page = page;
            result.pageSize = pageSize;
            result.hasMore = false;
            return result;
        }

        // 4. 提取post_id和sender_id列表
        std::vector<int> postIds;
        std::vector<int> senderIds;
        for (const auto& share : shares) {
            postIds.push_back(share.getPostId());
            senderIds.push_back(share.getSenderId());
        }

        // 5. 批量查询帖子信息
        auto postInfoMap = batchGetPostInfo(postIds);

        // 6. 批量查询用户信息
        auto userInfoMap = batchGetUserInfo(senderIds);

        // 7. 组装完整的分享列表项
        result.shares = assembleShareListItems(shares, postInfoMap, userInfoMap);

        // 8. 设置返回结果
        result.success = true;
        result.statusCode = 200;
        result.message = "查询成功";
        result.total = total;
        result.page = page;
        result.pageSize = pageSize;
        result.hasMore = (offset + pageSize) < total;

        Logger::info("Get received shares: receiverId=" + std::to_string(receiverId) +
                    ", page=" + std::to_string(page) +
                    ", total=" + std::to_string(total) +
                    ", returned=" + std::to_string(result.shares.size()));

    } catch (const std::exception& e) {
        Logger::error("Exception in getReceivedShares: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
    }

    return result;
}

// 获取发出的分享列表
ShareListResult ShareService::getSentShares(int senderId, int page, int pageSize) {
    ShareListResult result;

    try {
        // 1. 验证参数
        if (senderId <= 0) {
            result.statusCode = 400;
            result.message = "用户ID无效";
            return result;
        }

        if (page < 1) {
            page = 1;
        }

        if (pageSize < 1 || pageSize > 50) {
            pageSize = 20;  // 默认20，最大50
        }

        ConnectionGuard guard(DatabaseConnectionPool::getInstance());
        if (!guard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = guard.get();

        // 2. 查询总数
        int total = shareRepo_->countSentShares(conn, senderId);

        // 3. 查询分享列表（基础数据）
        int offset = (page - 1) * pageSize;
        std::vector<Share> shares = shareRepo_->findSentShares(conn, senderId, pageSize, offset);

        if (shares.empty()) {
            result.success = true;
            result.statusCode = 200;
            result.message = "查询成功";
            result.total = total;
            result.page = page;
            result.pageSize = pageSize;
            result.hasMore = false;
            return result;
        }

        // 4. 提取post_id和receiver_id列表（注意：这里是receiver，不是sender）
        std::vector<int> postIds;
        std::vector<int> receiverIds;  // 发出的分享，需要查询接收者信息
        for (const auto& share : shares) {
            postIds.push_back(share.getPostId());
            receiverIds.push_back(share.getReceiverId());
        }

        // 5. 批量查询帖子信息
        auto postInfoMap = batchGetPostInfo(postIds);

        // 6. 批量查询用户信息（这里查询的是接收者）
        auto userInfoMap = batchGetUserInfo(receiverIds);

        // 7. 组装完整的分享列表项（但需要调整：sender字段实际存的是receiver信息）
        // 注意：对于发出的分享，我们需要显示接收者信息，所以这里有些特殊处理
        std::vector<ShareListItem> items;
        for (const auto& share : shares) {
            ShareListItem item;
            item.shareId = share.getShareId();
            item.shareMessage = share.getShareMessage();
            item.createTime = static_cast<int64_t>(share.getCreateTime());

            // 查找帖子信息
            auto postIt = postInfoMap.find(share.getPostId());
            if (postIt != postInfoMap.end()) {
                item.post = postIt->second;
            } else {
                continue;
            }

            // 查找接收者信息（注意：在sender字段中存储）
            auto userIt = userInfoMap.find(share.getReceiverId());
            if (userIt != userInfoMap.end()) {
                item.sender = userIt->second;  // 复用sender字段显示接收者
            } else {
                continue;
            }

            items.push_back(item);
        }

        result.shares = items;

        // 8. 设置返回结果
        result.success = true;
        result.statusCode = 200;
        result.message = "查询成功";
        result.total = total;
        result.page = page;
        result.pageSize = pageSize;
        result.hasMore = (offset + pageSize) < total;

        Logger::info("Get sent shares: senderId=" + std::to_string(senderId) +
                    ", page=" + std::to_string(page) +
                    ", total=" + std::to_string(total) +
                    ", returned=" + std::to_string(result.shares.size()));

    } catch (const std::exception& e) {
        Logger::error("Exception in getSentShares: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
    }

    return result;
}

// 删除分享记录
DeleteShareResult ShareService::deleteShare(int shareId, int operatorId) {
    DeleteShareResult result;

    try {
        // 1. 验证参数
        if (shareId <= 0 || operatorId <= 0) {
            result.statusCode = 400;
            result.message = "参数无效";
            return result;
        }

        ConnectionGuard guard(DatabaseConnectionPool::getInstance());
        if (!guard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = guard.get();

        // 2. 查询分享记录是否存在
        auto shareOpt = shareRepo_->findById(conn, shareId);
        if (!shareOpt.has_value()) {
            result.statusCode = 404;
            result.message = "分享记录不存在";
            return result;
        }

        Share share = shareOpt.value();

        // 3. 验证操作者是否为分享发送者（仅发送者可删除）
        if (share.getSenderId() != operatorId) {
            result.statusCode = 403;
            result.message = "无权删除此分享记录";
            return result;
        }

        // 4. 删除分享记录
        bool deleted = shareRepo_->deleteById(conn, shareId);
        if (!deleted) {
            result.statusCode = 500;
            result.message = "删除失败";
            return result;
        }

        // 5. 返回成功结果
        result.success = true;
        result.statusCode = 200;
        result.message = "删除成功";

        Logger::info("Share deleted: shareId=" + std::to_string(shareId) +
                    ", operator=" + std::to_string(operatorId));

    } catch (const std::exception& e) {
        Logger::error("Exception in deleteShare: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
    }

    return result;
}
