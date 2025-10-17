/**
 * @file user_stats.h
 * @brief 用户统计信息模型定义
 * @author Knot Team
 * @date 2025-10-16
 */

#pragma once

#include <string>
#include <json/json.h>

/**
 * @brief 用户统计信息模型类
 *
 * 用于返回用户的统计数据，包括关注数、粉丝数、帖子数等
 * 主要用于用户主页展示和API响应
 */
class UserStats {
public:
    /**
     * @brief 默认构造函数
     */
    UserStats();

    /**
     * @brief 带参数的构造函数
     * @param userId 用户业务ID
     * @param followingCount 关注数（我关注的人数）
     * @param followerCount 粉丝数（关注我的人数）
     * @param postCount 帖子总数
     * @param totalLikes 获赞总数
     */
    UserStats(const std::string& userId, int followingCount, int followerCount, 
              int postCount, int totalLikes);

    /**
     * @brief 析构函数
     */
    ~UserStats() = default;

    // Getters
    std::string getUserId() const { return userId_; }
    int getFollowingCount() const { return followingCount_; }
    int getFollowerCount() const { return followerCount_; }
    int getPostCount() const { return postCount_; }
    int getTotalLikes() const { return totalLikes_; }

    // Setters
    void setUserId(const std::string& userId) { userId_ = userId; }
    void setFollowingCount(int count) { followingCount_ = count; }
    void setFollowerCount(int count) { followerCount_ = count; }
    void setPostCount(int count) { postCount_ = count; }
    void setTotalLikes(int likes) { totalLikes_ = likes; }

    /**
     * @brief 将统计信息转换为JSON
     * @return JSON表示
     */
    Json::Value toJson() const;

private:
    std::string userId_;        // 用户业务ID
    int followingCount_;        // 关注数（我关注的人数）
    int followerCount_;         // 粉丝数（关注我的人数）
    int postCount_;             // 帖子总数
    int totalLikes_;            // 获赞总数（所有帖子的点赞数总和）
};



