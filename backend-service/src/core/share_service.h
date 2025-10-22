/**
 * @file share_service.h
 * @brief 分享服务 - 业务逻辑层
 * @author Claude Code Assistant
 * @date 2025-10-22
 * @version v2.10.0
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "models/share.h"

// 前向声明
class ShareRepository;
class FollowRepository;
class PostRepository;
class UserRepository;

/**
 * @brief 创建分享结果结构体
 */
struct CreateShareResult {
    bool success;           // 操作是否成功
    int statusCode;         // HTTP状态码
    std::string message;    // 消息
    std::string shareId;    // 分享业务ID (成功时返回)
    int64_t createTime;     // 创建时间戳 (成功时返回)

    CreateShareResult()
        : success(false), statusCode(500), message(""), shareId(""), createTime(0) {}
};

/**
 * @brief 分享列表项结构体（包含完整的帖子和用户信息）
 */
struct ShareListItem {
    // 分享基础信息
    std::string shareId;        // 分享业务ID
    std::string shareMessage;   // 分享附言
    int64_t createTime;         // 分享时间

    // 帖子信息（通过批量查询获取）
    struct PostInfo {
        int id;                 // 帖子物理ID
        std::string postId;     // 帖子业务ID
        std::string title;      // 标题
        std::string description;// 描述
        std::string coverImage; // 封面图
        int likeCount;          // 点赞数
        int favoriteCount;      // 收藏数
    } post;

    // 发送者信息（通过批量查询获取）
    struct SenderInfo {
        int id;                 // 用户物理ID
        std::string userId;     // 用户业务ID
        std::string username;   // 用户名
        std::string avatarUrl;  // 头像
        std::string bio;        // 简介
    } sender;
};

/**
 * @brief 分享列表查询结果结构体
 */
struct ShareListResult {
    bool success;                       // 查询是否成功
    int statusCode;                     // HTTP状态码
    std::string message;                // 消息
    std::vector<ShareListItem> shares;  // 分享列表
    int total;                          // 总数
    int page;                           // 当前页码
    int pageSize;                       // 每页数量
    bool hasMore;                       // 是否有更多

    ShareListResult()
        : success(false), statusCode(500), message(""), total(0), page(1), pageSize(20), hasMore(false) {}
};

/**
 * @brief 删除分享结果结构体
 */
struct DeleteShareResult {
    bool success;           // 操作是否成功
    int statusCode;         // HTTP状态码
    std::string message;    // 消息

    DeleteShareResult()
        : success(false), statusCode(500), message("") {}
};

/**
 * @brief 分享服务类
 *
 * 负责分享相关的业务逻辑：
 * - 创建分享记录（验证互关关系）
 * - 查询收到的分享列表（批量获取帖子和用户信息）
 * - 查询发出的分享列表
 * - 删除分享记录（权限验证）
 */
class ShareService {
public:
    /**
     * @brief 构造函数
     */
    ShareService();

    /**
     * @brief 析构函数
     */
    ~ShareService();

    /**
     * @brief 创建分享记录
     * @param senderId 发送者ID（物理ID，从JWT中获取）
     * @param postId 帖子业务ID（如"POST_2025Q4_ABC123"）
     * @param receiverId 接收者业务ID（如"USR_2025Q4_XYZ789"）
     * @param shareMessage 分享附言（可选，最多500字符）
     * @return 创建结果
     *
     * 业务规则：
     * 1. 将业务ID转换为物理ID
     * 2. 验证发送者和接收者是否互相关注
     * 3. 验证帖子是否存在
     * 4. 验证不能分享给自己
     * 5. 验证是否已分享过（防重复）
     * 6. 生成业务ID（SHR_格式）
     * 7. 创建分享记录
     */
    CreateShareResult createShare(int senderId, const std::string& postId, const std::string& receiverId, const std::string& shareMessage);

    /**
     * @brief 获取收到的分享列表
     * @param receiverId 接收者ID（物理ID，从JWT中获取）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量（默认20，最大50）
     * @return 分享列表结果（包含完整的帖子和用户信息）
     *
     * 实现策略（批量查询优化）：
     * 1. ShareRepository查询基础分享记录
     * 2. 提取所有post_id，批量查询帖子信息
     * 3. 提取所有sender_id，批量查询用户信息
     * 4. 组装完整响应数据
     */
    ShareListResult getReceivedShares(int receiverId, int page, int pageSize);

    /**
     * @brief 获取发出的分享列表
     * @param senderId 发送者ID（物理ID，从JWT中获取）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量（默认20，最大50）
     * @return 分享列表结果（包含完整的帖子和用户信息）
     */
    ShareListResult getSentShares(int senderId, int page, int pageSize);

    /**
     * @brief 删除分享记录
     * @param shareId 分享物理ID
     * @param operatorId 操作者ID（物理ID，从JWT中获取）
     * @return 删除结果
     *
     * 业务规则：
     * 1. 验证分享记录是否存在
     * 2. 验证操作者是否为分享发送者（仅发送者可删除）
     * 3. 删除分享记录
     */
    DeleteShareResult deleteShare(int shareId, int operatorId);

private:
    std::unique_ptr<ShareRepository> shareRepo_;
    std::unique_ptr<FollowRepository> followRepo_;
    std::unique_ptr<PostRepository> postRepo_;
    std::unique_ptr<UserRepository> userRepo_;

    /**
     * @brief 生成分享业务ID
     * @return 业务ID（格式：SHR_2025Q4_ABC123）
     */
    std::string generateShareId();

    /**
     * @brief 检查两个用户是否互相关注
     * @param userId1 用户1的物理ID
     * @param userId2 用户2的物理ID
     * @return 互关返回true，否则返回false
     */
    bool checkMutualFollow(int userId1, int userId2);

    /**
     * @brief 批量获取帖子信息（复用PostRepository）
     * @param postIds 帖子物理ID列表
     * @return 帖子ID到帖子信息的映射
     */
    std::map<int, ShareListItem::PostInfo> batchGetPostInfo(const std::vector<int>& postIds);

    /**
     * @brief 批量获取用户信息（复用UserRepository）
     * @param userIds 用户物理ID列表
     * @return 用户ID到用户信息的映射
     */
    std::map<int, ShareListItem::SenderInfo> batchGetUserInfo(const std::vector<int>& userIds);

    /**
     * @brief 组装分享列表项（合并基础Share数据和批量查询结果）
     * @param shares 基础Share对象列表
     * @param postInfoMap 帖子信息映射
     * @param userInfoMap 用户信息映射
     * @return 完整的分享列表项
     */
    std::vector<ShareListItem> assembleShareListItems(
        const std::vector<Share>& shares,
        const std::map<int, ShareListItem::PostInfo>& postInfoMap,
        const std::map<int, ShareListItem::SenderInfo>& userInfoMap
    );
};
