/**
 * @file image.cpp
 * @brief 图片模型实现
 * @author Knot Team
 * @date 2025-10-07
 */

#include "models/image.h"

// 默认构造函数
Image::Image()
    : id_(0), userId_(0), fileSize_(0), width_(0), height_(0),
      likeCount_(0), favoriteCount_(0), viewCount_(0),
      status_(ImageStatus::APPROVED), createTime_(0), updateTime_(0) {
}

// 带参数的构造函数
Image::Image(int id, const std::string& imageId, int userId, const std::string& title)
    : id_(id), imageId_(imageId), userId_(userId), title_(title),
      fileSize_(0), width_(0), height_(0), likeCount_(0), favoriteCount_(0), viewCount_(0),
      status_(ImageStatus::APPROVED), createTime_(std::time(nullptr)), updateTime_(std::time(nullptr)) {
}

// 将图片对象转换为JSON
Json::Value Image::toJson() const {
    Json::Value json;
    
    json["id"] = id_;
    json["image_id"] = imageId_;
    json["user_id"] = userId_;
    json["title"] = title_;
    json["description"] = description_;
    json["file_url"] = fileUrl_;
    json["thumbnail_url"] = thumbnailUrl_;
    json["file_size"] = static_cast<Json::Int64>(fileSize_);
    json["width"] = width_;
    json["height"] = height_;
    json["mime_type"] = mimeType_;
    json["like_count"] = likeCount_;
    json["favorite_count"] = favoriteCount_;
    json["view_count"] = viewCount_;
    json["status"] = statusToString(status_);
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
    
    if (j.isMember("user_id") && j["user_id"].isInt()) {
        image.userId_ = j["user_id"].asInt();
    }
    
    if (j.isMember("title") && j["title"].isString()) {
        image.title_ = j["title"].asString();
    }
    
    if (j.isMember("description") && j["description"].isString()) {
        image.description_ = j["description"].asString();
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
    
    if (j.isMember("like_count") && j["like_count"].isInt()) {
        image.likeCount_ = j["like_count"].asInt();
    }
    
    if (j.isMember("favorite_count") && j["favorite_count"].isInt()) {
        image.favoriteCount_ = j["favorite_count"].asInt();
    }
    
    if (j.isMember("view_count") && j["view_count"].isInt()) {
        image.viewCount_ = j["view_count"].asInt();
    }
    
    if (j.isMember("status") && j["status"].isString()) {
        image.status_ = stringToStatus(j["status"].asString());
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
    // 验证标题
    if (title_.empty()) {
        return "标题不能为空";
    }
    
    if (title_.length() > 255) {
        return "标题长度不能超过255个字符";
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

// 将ImageStatus转换为字符串
std::string Image::statusToString(ImageStatus status) {
    switch (status) {
        case ImageStatus::PENDING:
            return "PENDING";
        case ImageStatus::APPROVED:
            return "APPROVED";
        case ImageStatus::REJECTED:
            return "REJECTED";
        default:
            return "APPROVED";
    }
}

// 将字符串转换为ImageStatus
ImageStatus Image::stringToStatus(const std::string& statusStr) {
    if (statusStr == "PENDING") {
        return ImageStatus::PENDING;
    } else if (statusStr == "APPROVED") {
        return ImageStatus::APPROVED;
    } else if (statusStr == "REJECTED") {
        return ImageStatus::REJECTED;
    } else {
        return ImageStatus::APPROVED;  // 默认值
    }
}

