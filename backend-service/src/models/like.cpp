/**
 * @file like.cpp
 * @brief 点赞模型实现
 * @author Knot Team
 * @date 2025-10-10
 */

#include "like.h"
#include "utils/logger.h"

Like::Like()
    : id_(0),
      userId_(0),
      postId_(0),
      createTime_(0) {
}

Like::Like(int id, int userId, int postId)
    : id_(id),
      userId_(userId),
      postId_(postId),
      createTime_(std::time(nullptr)) {
}

Json::Value Like::toJson() const {
    Json::Value json;

    json["id"] = id_;
    json["user_id"] = userId_;
    json["post_id"] = postId_;
    json["create_time"] = static_cast<Json::Int64>(createTime_);

    return json;
}

Like Like::fromJson(const Json::Value& json) {
    Like like;

    if (json.isMember("id") && json["id"].isInt()) {
        like.setId(json["id"].asInt());
    }

    if (json.isMember("user_id") && json["user_id"].isInt()) {
        like.setUserId(json["user_id"].asInt());
    }

    if (json.isMember("post_id") && json["post_id"].isInt()) {
        like.setPostId(json["post_id"].asInt());
    }

    if (json.isMember("create_time") && json["create_time"].isInt64()) {
        like.setCreateTime(static_cast<std::time_t>(json["create_time"].asInt64()));
    }

    return like;
}
