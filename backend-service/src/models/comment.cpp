/**
 * @file comment.cpp
 * @brief 评论模型实现
 * @author Knot Team
 * @date 2025-10-19
 */

#include "comment.h"
#include "utils/logger.h"
#include <algorithm>
#include <cctype>

Comment::Comment()
    : id_(0),
      commentId_(""),
      postId_(0),
      userId_(0),
      content_(""),
      createTime_(0) {
}

Comment::Comment(int id, const std::string& commentId, int postId, int userId, const std::string& content)
    : id_(id),
      commentId_(commentId),
      postId_(postId),
      userId_(userId),
      content_(content),
      createTime_(std::time(nullptr)) {
}

Json::Value Comment::toJson() const {
    Json::Value json;

    json["id"] = id_;
    json["comment_id"] = commentId_;
    json["post_id"] = postId_;
    json["user_id"] = userId_;
    json["content"] = content_;
    json["create_time"] = static_cast<Json::Int64>(createTime_);

    return json;
}

Comment Comment::fromJson(const Json::Value& json) {
    Comment comment;

    if (json.isMember("id") && json["id"].isInt()) {
        comment.setId(json["id"].asInt());
    }

    if (json.isMember("comment_id") && json["comment_id"].isString()) {
        comment.setCommentId(json["comment_id"].asString());
    }

    if (json.isMember("post_id") && json["post_id"].isInt()) {
        comment.setPostId(json["post_id"].asInt());
    }

    if (json.isMember("user_id") && json["user_id"].isInt()) {
        comment.setUserId(json["user_id"].asInt());
    }

    if (json.isMember("content") && json["content"].isString()) {
        comment.setContent(json["content"].asString());
    }

    if (json.isMember("create_time") && json["create_time"].isInt64()) {
        comment.setCreateTime(static_cast<std::time_t>(json["create_time"].asInt64()));
    }

    return comment;
}

std::string Comment::validate() const {
    // 验证评论内容
    if (content_.empty()) {
        return "评论内容不能为空";
    }

    // 检查是否为纯空格
    bool allSpaces = std::all_of(content_.begin(), content_.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    if (allSpaces) {
        return "评论内容不能为纯空格";
    }

    // 检查长度（UTF-8字符计数，这里简化处理，按字节计数）
    if (content_.length() > 1000) {
        return "评论内容不能超过1000字符";
    }

    // 检查特殊控制字符
    for (char c : content_) {
        if (c < 0x20 && c != '\n' && c != '\r' && c != '\t') {
            return "评论内容包含非法字符";
        }
    }

    return "";  // 验证通过
}
