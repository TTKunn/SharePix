/**
 * @file follow.cpp
 * @brief 关注模型实现
 * @author Knot Team
 * @date 2025-10-16
 */

#include "follow.h"
#include <chrono>

// 默认构造函数
Follow::Follow() 
    : id_(0)
    , followerId_(0)
    , followeeId_(0)
    , createTime_(std::time(nullptr)) {
}

// 带参数的构造函数
Follow::Follow(int64_t id, int64_t followerId, int64_t followeeId)
    : id_(id)
    , followerId_(followerId)
    , followeeId_(followeeId)
    , createTime_(std::time(nullptr)) {
}

// 将关注对象转换为JSON
Json::Value Follow::toJson() const {
    Json::Value json;
    json["id"] = static_cast<Json::Int64>(id_);
    json["follower_id"] = static_cast<Json::Int64>(followerId_);
    json["followee_id"] = static_cast<Json::Int64>(followeeId_);
    json["create_time"] = static_cast<Json::Int64>(createTime_);
    return json;
}

// 从JSON创建关注对象
Follow Follow::fromJson(const Json::Value& json) {
    Follow follow;
    
    if (json.isMember("id")) {
        follow.setId(json["id"].asInt64());
    }
    
    if (json.isMember("follower_id")) {
        follow.setFollowerId(json["follower_id"].asInt64());
    }
    
    if (json.isMember("followee_id")) {
        follow.setFolloweeId(json["followee_id"].asInt64());
    }
    
    if (json.isMember("create_time")) {
        follow.setCreateTime(static_cast<std::time_t>(json["create_time"].asInt64()));
    }
    
    return follow;
}



