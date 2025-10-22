#include "share.h"
#include "../utils/logger.h"
#include <sstream>

// 默认构造函数
Share::Share()
    : id_(0)
    , shareId_("")
    , postId_(0)
    , senderId_(0)
    , receiverId_(0)
    , shareMessage_("")
    , createTime_(0) {
}

// 参数化构造函数
Share::Share(int id, const std::string& shareId, int postId, int senderId, int receiverId)
    : id_(id)
    , shareId_(shareId)
    , postId_(postId)
    , senderId_(senderId)
    , receiverId_(receiverId)
    , shareMessage_("")
    , createTime_(std::time(nullptr)) {
}

// 转换为JSON对象
Json::Value Share::toJson() const {
    Json::Value json;
    json["id"] = id_;
    json["share_id"] = shareId_;
    json["post_id"] = postId_;
    json["sender_id"] = senderId_;
    json["receiver_id"] = receiverId_;
    json["share_message"] = shareMessage_;
    json["create_time"] = static_cast<Json::Int64>(createTime_);
    return json;
}

// 从JSON对象创建Share实例
Share Share::fromJson(const Json::Value& json) {
    Share share;

    if (json.isMember("id") && json["id"].isInt()) {
        share.setId(json["id"].asInt());
    }

    if (json.isMember("share_id") && json["share_id"].isString()) {
        share.setShareId(json["share_id"].asString());
    }

    if (json.isMember("post_id") && json["post_id"].isInt()) {
        share.setPostId(json["post_id"].asInt());
    }

    if (json.isMember("sender_id") && json["sender_id"].isInt()) {
        share.setSenderId(json["sender_id"].asInt());
    }

    if (json.isMember("receiver_id") && json["receiver_id"].isInt()) {
        share.setReceiverId(json["receiver_id"].asInt());
    }

    if (json.isMember("share_message") && json["share_message"].isString()) {
        share.setShareMessage(json["share_message"].asString());
    }

    if (json.isMember("create_time") && json["create_time"].isInt64()) {
        share.setCreateTime(static_cast<std::time_t>(json["create_time"].asInt64()));
    }

    return share;
}

// 验证分享数据的有效性
std::string Share::validate() const {
    // 验证帖子ID
    if (postId_ <= 0) {
        return "帖子ID无效";
    }

    // 验证发送者ID
    if (senderId_ <= 0) {
        return "发送者ID无效";
    }

    // 验证接收者ID
    if (receiverId_ <= 0) {
        return "接收者ID无效";
    }

    // 验证不能分享给自己
    if (senderId_ == receiverId_) {
        return "不能分享给自己";
    }

    // 验证分享附言长度（最多500字符）
    if (shareMessage_.length() > 500) {
        return "分享附言过长（最多500字符）";
    }

    return "";  // 验证通过
}
