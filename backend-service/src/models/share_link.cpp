/**
 * @file share_link.cpp
 * @brief 分享链接模型类实现
 * @author Knot Development Team
 * @date 2025-10-16
 */

#include "models/share_link.h"
#include "utils/config_manager.h"
#include "utils/logger.h"
#include <ctime>
#include <sstream>

// 检查链接是否过期
bool ShareLink::isExpired() const {
    if (!expireTime_.has_value()) {
        return false;  // 永久链接不过期
    }
    
    time_t now = std::time(nullptr);
    return now > expireTime_.value();
}

// 获取完整的短链接URL
std::string ShareLink::getFullUrl(const std::string& baseUrl) const {
    std::string base;
    
    if (baseUrl.empty()) {
        // 从配置读取基础URL
        auto& config = ConfigManager::getInstance();
        std::string host = config.get<std::string>("server.host", "localhost");
        int port = config.get<int>("server.port", 8080);
        
        std::ostringstream oss;
        oss << "http://" << host << ":" << port;
        base = oss.str();
    } else {
        base = baseUrl;
    }
    
    // 去除末尾的斜杠
    if (!base.empty() && base.back() == '/') {
        base.pop_back();
    }
    
    return base + "/s/" + shortCode_;
}

// TargetType转字符串
std::string ShareLink::targetTypeToString(TargetType type) {
    switch (type) {
        case TargetType::POST:
            return "POST";
        case TargetType::USER:
            return "USER";
        case TargetType::TAG:
            return "TAG";
        default:
            return "UNKNOWN";
    }
}

// 字符串转TargetType
ShareLink::TargetType ShareLink::stringToTargetType(const std::string& str) {
    if (str == "POST") {
        return TargetType::POST;
    } else if (str == "USER") {
        return TargetType::USER;
    } else if (str == "TAG") {
        return TargetType::TAG;
    } else {
        Logger::warning("Unknown target type: " + str + ", defaulting to POST");
        return TargetType::POST;
    }
}

// 转换为JSON
Json::Value ShareLink::toJson() const {
    Json::Value json;
    
    json["id"] = static_cast<Json::Int64>(id_);
    json["short_code"] = shortCode_;
    json["target_type"] = targetTypeToString(targetType_);
    json["target_id"] = static_cast<Json::Int64>(targetId_);
    
    if (creatorId_.has_value()) {
        json["creator_id"] = static_cast<Json::Int64>(creatorId_.value());
    } else {
        json["creator_id"] = Json::Value::null;
    }
    
    json["create_time"] = static_cast<Json::Int64>(createTime_);
    
    if (expireTime_.has_value()) {
        json["expire_time"] = static_cast<Json::Int64>(expireTime_.value());
    } else {
        json["expire_time"] = Json::Value::null;
    }
    
    json["is_expired"] = isExpired();
    json["full_url"] = getFullUrl();
    
    return json;
}

// 从JSON构造
ShareLink ShareLink::fromJson(const Json::Value& json) {
    ShareLink link;
    
    if (json.isMember("id") && json["id"].isInt64()) {
        link.id_ = json["id"].asInt64();
    }
    
    if (json.isMember("short_code") && json["short_code"].isString()) {
        link.shortCode_ = json["short_code"].asString();
    }
    
    if (json.isMember("target_type") && json["target_type"].isString()) {
        link.targetType_ = stringToTargetType(json["target_type"].asString());
    }
    
    if (json.isMember("target_id") && json["target_id"].isInt64()) {
        link.targetId_ = json["target_id"].asInt64();
    }
    
    if (json.isMember("creator_id") && !json["creator_id"].isNull() && json["creator_id"].isInt64()) {
        link.creatorId_ = json["creator_id"].asInt64();
    }
    
    if (json.isMember("create_time") && json["create_time"].isInt64()) {
        link.createTime_ = static_cast<time_t>(json["create_time"].asInt64());
    }
    
    if (json.isMember("expire_time") && !json["expire_time"].isNull() && json["expire_time"].isInt64()) {
        link.expireTime_ = static_cast<time_t>(json["expire_time"].asInt64());
    }
    
    return link;
}

