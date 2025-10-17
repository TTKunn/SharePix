/**
 * @file follow.h
 * @brief 关注模型定义
 * @author Knot Team
 * @date 2025-10-16
 */

#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

/**
 * @brief 关注模型类
 *
 * 一条Follow记录代表一个用户对另一个用户的关注行为
 * follower_id关注followee_id（单向关注）
 * 通过唯一约束(follower_id, followee_id)保证幂等性
 */
class Follow {
public:
    /**
     * @brief 默认构造函数
     */
    Follow();

    /**
     * @brief 带参数的构造函数
     * @param id 物理ID（自增主键）
     * @param followerId 关注者ID（主动方）
     * @param followeeId 被关注者ID（被动方）
     */
    Follow(int64_t id, int64_t followerId, int64_t followeeId);

    /**
     * @brief 析构函数
     */
    ~Follow() = default;

    // Getters
    int64_t getId() const { return id_; }
    int64_t getFollowerId() const { return followerId_; }
    int64_t getFolloweeId() const { return followeeId_; }
    std::time_t getCreateTime() const { return createTime_; }

    // Setters
    void setId(int64_t id) { id_ = id; }
    void setFollowerId(int64_t followerId) { followerId_ = followerId; }
    void setFolloweeId(int64_t followeeId) { followeeId_ = followeeId; }
    void setCreateTime(std::time_t createTime) { createTime_ = createTime; }

    /**
     * @brief 将关注对象转换为JSON
     * @return JSON表示
     */
    Json::Value toJson() const;

    /**
     * @brief 从JSON创建关注对象
     * @param json JSON对象
     * @return Follow对象
     */
    static Follow fromJson(const Json::Value& json);

private:
    int64_t id_;                    // 物理ID（自增主键）
    int64_t followerId_;            // 关注者ID（A关注B，A是follower）
    int64_t followeeId_;            // 被关注者ID（A关注B，B是followee）
    std::time_t createTime_;        // 关注时间
};



