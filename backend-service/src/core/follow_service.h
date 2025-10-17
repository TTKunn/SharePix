/**
 * @file follow_service.h
 * @brief 关注服务 - 业务逻辑层
 * @author Knot Team
 * @date 2025-10-16
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include "models/user_stats.h"

// 前向声明
class FollowRepository;
class UserRepository;

/**
 * @brief 关注结果结构体
 */
struct FollowResult {
    bool success;           // 操作是否成功
    int statusCode;         // HTTP状态码
    std::string message;    // 消息
    int followerCount;      // 被关注者的粉丝数
    bool isFollowing;       // 是否正在关注

    FollowResult()
        : success(false), statusCode(500), message(""), followerCount(0), isFollowing(false) {}
};

/**
 * @brief 关注状态查询结果结构体
 */
struct FollowStatusResult {
    bool success;           // 查询是否成功
    int statusCode;         // HTTP状态码
    std::string message;    // 消息
    bool isFollowing;       // 我是否关注该用户
    bool isFollowedBy;      // 该用户是否关注我

    FollowStatusResult()
        : success(false), statusCode(500), message(""), isFollowing(false), isFollowedBy(false) {}
};

/**
 * @brief 用户信息结构体（用于列表展示）
 */
struct UserListInfo {
    std::string userId;         // 用户业务ID
    std::string username;       // 用户名
    std::string realName;       // 真实姓名
    std::string avatarUrl;      // 头像URL
    std::string bio;            // 个人简介
    int followerCount;          // 粉丝数
    bool isFollowing;           // 当前登录用户是否关注该用户
    int64_t followedAt;         // 关注时间戳
};

/**
 * @brief 关注服务类
 *
 * 负责关注相关的业务逻辑：
 * - 关注用户
 * - 取消关注
 * - 检查关注关系
 * - 查询关注列表
 * - 查询粉丝列表
 * - 获取用户统计信息
 * - 批量检查关注关系
 */
class FollowService {
public:
    /**
     * @brief 构造函数
     */
    FollowService();

    /**
     * @brief 析构函数
     */
    ~FollowService();

    /**
     * @brief 关注用户
     *
     * 业务流程：
     * 1. 查询目标用户是否存在
     * 2. 检查是否尝试关注自己
     * 3. 检查是否已关注（幂等性）
     * 4. 开启事务
     * 5. 创建关注记录
     * 6. 更新关注者的following_count
     * 7. 更新被关注者的follower_count
     * 8. 提交事务
     *
     * @param followerId 关注者ID（物理ID）
     * @param followeeUserId 被关注者业务ID（USR_XXXXXX）
     * @return FollowResult 关注结果
     */
    FollowResult followUser(int64_t followerId, const std::string& followeeUserId);

    /**
     * @brief 取消关注
     *
     * @param followerId 关注者ID（物理ID）
     * @param followeeUserId 被关注者业务ID
     * @return FollowResult 取消关注结果
     */
    FollowResult unfollowUser(int64_t followerId, const std::string& followeeUserId);

    /**
     * @brief 检查关注关系
     *
     * @param followerId 关注者ID（物理ID）
     * @param followeeUserId 被关注者业务ID
     * @return FollowStatusResult 关注状态
     */
    FollowStatusResult checkFollowStatus(int64_t followerId, const std::string& followeeUserId);

    /**
     * @brief 获取关注列表（我关注的人）
     *
     * @param userId 用户业务ID
     * @param currentUserId 当前登录用户ID（物理ID，可选，用于标记is_following）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @param total 返回总数（输出参数）
     * @return 用户列表
     */
    std::vector<UserListInfo> getFollowingList(const std::string& userId, int64_t currentUserId,
                                                 int page, int pageSize, int& total);

    /**
     * @brief 获取粉丝列表（关注我的人）
     *
     * @param userId 用户业务ID
     * @param currentUserId 当前登录用户ID（物理ID，可选，用于标记is_following）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @param total 返回总数（输出参数）
     * @return 用户列表
     */
    std::vector<UserListInfo> getFollowerList(const std::string& userId, int64_t currentUserId,
                                               int page, int pageSize, int& total);

    /**
     * @brief 获取用户统计信息
     *
     * @param userId 用户业务ID
     * @return UserStats对象
     */
    std::optional<UserStats> getUserStats(const std::string& userId);

    /**
     * @brief 批量检查关注关系
     *
     * @param followerId 关注者ID（物理ID）
     * @param followeeUserIds 被关注者业务ID列表
     * @return Map<userId, isFollowing>
     */
    std::map<std::string, bool> batchCheckFollowStatus(int64_t followerId,
                                                         const std::vector<std::string>& followeeUserIds);

private:
    std::unique_ptr<FollowRepository> followRepo_;
    std::unique_ptr<UserRepository> userRepo_;
};

