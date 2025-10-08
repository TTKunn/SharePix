/**
 * @file tag.cpp
 * @brief 标签模型实现
 * @author Knot Team
 * @date 2025-10-07
 */

#include "models/tag.h"

// 默认构造函数
Tag::Tag()
    : id_(0), useCount_(0), createTime_(0) {
}

// 带参数的构造函数
Tag::Tag(int id, const std::string& name)
    : id_(id), name_(name), useCount_(0), createTime_(std::time(nullptr)) {
}

// 将标签对象转换为JSON
Json::Value Tag::toJson() const {
    Json::Value json;
    
    json["id"] = id_;
    json["name"] = name_;
    json["use_count"] = useCount_;
    json["create_time"] = static_cast<Json::Int64>(createTime_);
    
    return json;
}

// 从JSON创建标签对象
Tag Tag::fromJson(const Json::Value& j) {
    Tag tag;
    
    if (j.isMember("id") && j["id"].isInt()) {
        tag.id_ = j["id"].asInt();
    }
    
    if (j.isMember("name") && j["name"].isString()) {
        tag.name_ = j["name"].asString();
    }
    
    if (j.isMember("use_count") && j["use_count"].isInt()) {
        tag.useCount_ = j["use_count"].asInt();
    }
    
    if (j.isMember("create_time") && j["create_time"].isInt64()) {
        tag.createTime_ = static_cast<std::time_t>(j["create_time"].asInt64());
    }
    
    return tag;
}

// 验证标签数据
std::string Tag::validate() const {
    // 验证标签名称
    if (name_.empty()) {
        return "标签名称不能为空";
    }
    
    if (name_.length() > 50) {
        return "标签名称长度不能超过50个字符";
    }
    
    return "";  // 验证通过
}

