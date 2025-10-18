/**
 * @file post.cpp
 * @brief 帖子模型实现
 * @author Knot Team
 * @date 2025-10-08
 */

#include "models/post.h"
#include "utils/url_helper.h"

// 默认构造函数
Post::Post()
    : id_(0), userId_(0), imageCount_(0), likeCount_(0), favoriteCount_(0), viewCount_(0),
      status_(PostStatus::APPROVED), createTime_(0), updateTime_(0) {
}

// 带参数的构造函数
Post::Post(int id, const std::string& postId, int userId, const std::string& title)
    : id_(id), postId_(postId), userId_(userId), title_(title),
      imageCount_(0), likeCount_(0), favoriteCount_(0), viewCount_(0),
      status_(PostStatus::APPROVED), createTime_(std::time(nullptr)), updateTime_(std::time(nullptr)) {
}

// 添加图片到帖子
void Post::addImage(const Image& image) {
    images_.push_back(image);
    imageCount_ = static_cast<int>(images_.size());
}

// 清空所有图片
void Post::clearImages() {
    images_.clear();
    imageCount_ = 0;
}

// 将帖子对象转换为JSON
Json::Value Post::toJson(bool includeImages) const {
    Json::Value json;

    json["id"] = id_;
    json["post_id"] = postId_;
    json["user_id"] = userLogicalId_;  // 返回逻辑ID（前端使用）
    json["title"] = title_;
    json["description"] = description_;
    json["image_count"] = imageCount_;
    json["like_count"] = likeCount_;
    json["favorite_count"] = favoriteCount_;
    json["view_count"] = viewCount_;
    json["status"] = statusToString(status_);
    json["create_time"] = static_cast<Json::Int64>(createTime_);
    json["update_time"] = static_cast<Json::Int64>(updateTime_);
    
    // 添加封面图片URL
    std::string coverUrl = getCoverImageUrl();
    if (!coverUrl.empty()) {
        json["cover_image_url"] = coverUrl;
    }
    
    // 如果需要包含图片列表
    if (includeImages && !images_.empty()) {
        Json::Value imagesArray(Json::arrayValue);
        for (const auto& image : images_) {
            imagesArray.append(image.toJson());
        }
        json["images"] = imagesArray;
    }
    
    return json;
}

// 从JSON创建帖子对象
Post Post::fromJson(const Json::Value& j) {
    Post post;
    
    if (j.isMember("id") && j["id"].isInt()) {
        post.id_ = j["id"].asInt();
    }
    
    if (j.isMember("post_id") && j["post_id"].isString()) {
        post.postId_ = j["post_id"].asString();
    }
    
    if (j.isMember("user_id")) {
        if (j["user_id"].isInt()) {
            // 兼容旧格式（物理ID）
            post.userId_ = j["user_id"].asInt();
        } else if (j["user_id"].isString()) {
            // 新格式（逻辑ID）
            post.userLogicalId_ = j["user_id"].asString();
        }
    }
    
    if (j.isMember("title") && j["title"].isString()) {
        post.title_ = j["title"].asString();
    }
    
    if (j.isMember("description") && j["description"].isString()) {
        post.description_ = j["description"].asString();
    }
    
    if (j.isMember("image_count") && j["image_count"].isInt()) {
        post.imageCount_ = j["image_count"].asInt();
    }
    
    if (j.isMember("like_count") && j["like_count"].isInt()) {
        post.likeCount_ = j["like_count"].asInt();
    }
    
    if (j.isMember("favorite_count") && j["favorite_count"].isInt()) {
        post.favoriteCount_ = j["favorite_count"].asInt();
    }
    
    if (j.isMember("view_count") && j["view_count"].isInt()) {
        post.viewCount_ = j["view_count"].asInt();
    }
    
    if (j.isMember("status") && j["status"].isString()) {
        post.status_ = stringToStatus(j["status"].asString());
    }
    
    if (j.isMember("create_time") && j["create_time"].isInt64()) {
        post.createTime_ = static_cast<std::time_t>(j["create_time"].asInt64());
    }
    
    if (j.isMember("update_time") && j["update_time"].isInt64()) {
        post.updateTime_ = static_cast<std::time_t>(j["update_time"].asInt64());
    }
    
    // 解析图片列表
    if (j.isMember("images") && j["images"].isArray()) {
        for (const auto& imageJson : j["images"]) {
            post.images_.push_back(Image::fromJson(imageJson));
        }
    }
    
    return post;
}

// 验证帖子数据
std::string Post::validate() const {
    // 验证标题
    if (title_.empty()) {
        return "标题不能为空";
    }
    
    if (title_.length() > 255) {
        return "标题长度不能超过255个字符";
    }
    
    // 验证用户ID
    if (userId_ <= 0) {
        return "无效的用户ID";
    }
    
    // 验证图片数量
    if (imageCount_ < 0 || imageCount_ > 9) {
        return "图片数量必须在0-9之间";
    }
    
    // 验证图片列表与imageCount一致
    if (!images_.empty() && static_cast<int>(images_.size()) != imageCount_) {
        return "图片列表数量与imageCount不一致";
    }
    
    // 验证统计数据
    if (likeCount_ < 0 || favoriteCount_ < 0 || viewCount_ < 0) {
        return "统计数据不能为负数";
    }
    
    return "";  // 验证通过
}

// 获取封面图片（第一张图片）
std::string Post::getCoverImageUrl() const {
    if (images_.empty()) {
        return "";
    }
    // 使用UrlHelper为缩略图URL添加服务器URL前缀
    return UrlHelper::toFullUrl(images_[0].getThumbnailUrl());
}

// 将PostStatus转换为字符串
std::string Post::statusToString(PostStatus status) {
    switch (status) {
        case PostStatus::PENDING:
            return "PENDING";
        case PostStatus::APPROVED:
            return "APPROVED";
        case PostStatus::REJECTED:
            return "REJECTED";
        default:
            return "UNKNOWN";
    }
}

// 将字符串转换为PostStatus
PostStatus Post::stringToStatus(const std::string& statusStr) {
    if (statusStr == "PENDING") {
        return PostStatus::PENDING;
    } else if (statusStr == "APPROVED") {
        return PostStatus::APPROVED;
    } else if (statusStr == "REJECTED") {
        return PostStatus::REJECTED;
    } else {
        return PostStatus::APPROVED;  // 默认为已通过
    }
}

