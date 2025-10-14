/**
 * @file image.cpp
 * @brief 图片模型实现
 * @author Knot Team
 * @date 2025-10-07
 */

#include "models/image.h"
#include "utils/url_helper.h"

// 默认构造函数
Image::Image()
    : id_(0), postId_(0), displayOrder_(0), userId_(0), fileSize_(0), width_(0), height_(0),
      createTime_(0), updateTime_(0) {
}

// 带参数的构造函数
Image::Image(int id, const std::string& imageId, int postId, int userId)
    : id_(id), imageId_(imageId), postId_(postId), displayOrder_(0), userId_(userId),
      fileSize_(0), width_(0), height_(0),
      createTime_(std::time(nullptr)), updateTime_(std::time(nullptr)) {
}

// 将图片对象转换为JSON
Json::Value Image::toJson() const {
    Json::Value json;

    json["id"] = id_;
    json["image_id"] = imageId_;
    json["post_id"] = postId_;
    json["display_order"] = displayOrder_;
    json["user_id"] = userId_;
    
    // 使用UrlHelper为图片路径添加服务器URL前缀
    json["file_url"] = UrlHelper::toFullUrl(fileUrl_);
    json["thumbnail_url"] = UrlHelper::toFullUrl(thumbnailUrl_);
    
    json["file_size"] = static_cast<Json::Int64>(fileSize_);
    json["width"] = width_;
    json["height"] = height_;
    json["mime_type"] = mimeType_;
    json["create_time"] = static_cast<Json::Int64>(createTime_);
    json["update_time"] = static_cast<Json::Int64>(updateTime_);

    return json;
}

// 从JSON创建图片对象
Image Image::fromJson(const Json::Value& j) {
    Image image;

    if (j.isMember("id") && j["id"].isInt()) {
        image.id_ = j["id"].asInt();
    }

    if (j.isMember("image_id") && j["image_id"].isString()) {
        image.imageId_ = j["image_id"].asString();
    }

    if (j.isMember("post_id") && j["post_id"].isInt()) {
        image.postId_ = j["post_id"].asInt();
    }

    if (j.isMember("display_order") && j["display_order"].isInt()) {
        image.displayOrder_ = j["display_order"].asInt();
    }

    if (j.isMember("user_id") && j["user_id"].isInt()) {
        image.userId_ = j["user_id"].asInt();
    }

    if (j.isMember("file_url") && j["file_url"].isString()) {
        image.fileUrl_ = j["file_url"].asString();
    }

    if (j.isMember("thumbnail_url") && j["thumbnail_url"].isString()) {
        image.thumbnailUrl_ = j["thumbnail_url"].asString();
    }

    if (j.isMember("file_size") && j["file_size"].isInt64()) {
        image.fileSize_ = j["file_size"].asInt64();
    }

    if (j.isMember("width") && j["width"].isInt()) {
        image.width_ = j["width"].asInt();
    }

    if (j.isMember("height") && j["height"].isInt()) {
        image.height_ = j["height"].asInt();
    }

    if (j.isMember("mime_type") && j["mime_type"].isString()) {
        image.mimeType_ = j["mime_type"].asString();
    }

    if (j.isMember("create_time") && j["create_time"].isInt64()) {
        image.createTime_ = static_cast<std::time_t>(j["create_time"].asInt64());
    }

    if (j.isMember("update_time") && j["update_time"].isInt64()) {
        image.updateTime_ = static_cast<std::time_t>(j["update_time"].asInt64());
    }

    return image;
}

// 验证图片数据
std::string Image::validate() const {
    // 验证帖子ID
    if (postId_ <= 0) {
        return "无效的帖子ID";
    }

    // 验证显示顺序
    if (displayOrder_ < 0 || displayOrder_ > 8) {
        return "显示顺序必须在0-8之间";
    }

    // 验证文件大小（最大5MB）
    const long long MAX_FILE_SIZE = 5 * 1024 * 1024;  // 5MB
    if (fileSize_ > MAX_FILE_SIZE) {
        return "文件大小不能超过5MB";
    }

    // 验证MIME类型
    if (mimeType_ != "image/jpeg" && mimeType_ != "image/png" && mimeType_ != "image/webp") {
        return "不支持的图片格式，仅支持JPEG、PNG、WebP";
    }

    // 验证用户ID
    if (userId_ <= 0) {
        return "无效的用户ID";
    }

    return "";  // 验证通过
}

