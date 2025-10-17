/**
 * @file user_stats.cpp
 * @brief 用户统计信息模型实现
 * @author Knot Team
 * @date 2025-10-16
 */

#include "user_stats.h"

// 默认构造函数
UserStats::UserStats()
    : userId_("")
    , followingCount_(0)
    , followerCount_(0)
    , postCount_(0)
    , totalLikes_(0) {
}

// 带参数的构造函数
UserStats::UserStats(const std::string& userId, int followingCount, int followerCount,
                     int postCount, int totalLikes)
    : userId_(userId)
    , followingCount_(followingCount)
    , followerCount_(followerCount)
    , postCount_(postCount)
    , totalLikes_(totalLikes) {
}

// 将统计信息转换为JSON
Json::Value UserStats::toJson() const {
    Json::Value json;
    json["user_id"] = userId_;
    json["following_count"] = followingCount_;
    json["follower_count"] = followerCount_;
    json["post_count"] = postCount_;
    json["total_likes"] = totalLikes_;
    return json;
}



