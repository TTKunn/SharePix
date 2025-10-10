/**
 * @file favorite.cpp
 * @brief 收藏模型实现
 * @author Knot Team
 * @date 2025-10-10
 */

#include "favorite.h"
#include "utils/logger.h"

Favorite::Favorite()
    : id_(0),
      userId_(0),
      postId_(0),
      createTime_(0) {
}

Favorite::Favorite(int id, int userId, int postId)
    : id_(id),
      userId_(userId),
      postId_(postId),
      createTime_(std::time(nullptr)) {
}

Json::Value Favorite::toJson() const {
    Json::Value json;

    json["id"] = id_;
    json["user_id"] = userId_;
    json["post_id"] = postId_;
    json["create_time"] = static_cast<Json::Int64>(createTime_);

    return json;
}

Favorite Favorite::fromJson(const Json::Value& json) {
    Favorite favorite;

    if (json.isMember("id") && json["id"].isInt()) {
        favorite.setId(json["id"].asInt());
    }

    if (json.isMember("user_id") && json["user_id"].isInt()) {
        favorite.setUserId(json["user_id"].asInt());
    }

    if (json.isMember("post_id") && json["post_id"].isInt()) {
        favorite.setPostId(json["post_id"].asInt());
    }

    if (json.isMember("create_time") && json["create_time"].isInt64()) {
        favorite.setCreateTime(static_cast<std::time_t>(json["create_time"].asInt64()));
    }

    return favorite;
}
