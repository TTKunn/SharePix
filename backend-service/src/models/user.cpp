/**
 * @file user.cpp
 * @brief User model implementation
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#include "models/user.h"
#include "utils/url_helper.h"
#include <regex>
#include <sstream>
#include <iomanip>

// 默认构造函数
User::User()
    : id_(0), role_(UserRole::USER), status_(UserStatus::ACTIVE),
      deviceCount_(0), createTime_(0), updateTime_(0) {
}

// 带参数的构造函数
User::User(int id, const std::string& userId, const std::string& username, const std::string& phone)
    : id_(id), userId_(userId), username_(username), phone_(phone),
      role_(UserRole::USER), status_(UserStatus::ACTIVE), deviceCount_(0),
      createTime_(std::time(nullptr)), updateTime_(std::time(nullptr)) {
}

// 将用户对象转换为JSON
Json::Value User::toJson(bool includeSecrets) const {
    Json::Value json;

    json["id"] = id_;
    json["user_id"] = userId_;
    json["username"] = username_;
    json["real_name"] = realName_;
    json["phone"] = phone_;
    json["email"] = email_;
    json["role"] = roleToString(role_);
    json["status"] = statusToString(status_);
    
    // 使用UrlHelper为头像URL添加服务器URL前缀
    json["avatar_url"] = UrlHelper::toFullUrl(avatarUrl_);
    
    json["bio"] = bio_;
    json["gender"] = gender_;
    json["location"] = location_;
    json["device_count"] = deviceCount_;
    json["create_time"] = static_cast<Json::Int64>(createTime_);
    json["update_time"] = static_cast<Json::Int64>(updateTime_);

    // 仅在请求时包含敏感信息（用于内部使用）
    if (includeSecrets) {
        json["password"] = password_;
        json["salt"] = salt_;
    }

    return json;
}

// 从JSON创建用户对象
User User::fromJson(const Json::Value& j) {
    User user;

    if (j.isMember("id") && j["id"].isInt()) {
        user.id_ = j["id"].asInt();
    }

    if (j.isMember("user_id") && j["user_id"].isString()) {
        user.userId_ = j["user_id"].asString();
    }

    if (j.isMember("username") && j["username"].isString()) {
        user.username_ = j["username"].asString();
    }

    if (j.isMember("password") && j["password"].isString()) {
        user.password_ = j["password"].asString();
    }

    if (j.isMember("salt") && j["salt"].isString()) {
        user.salt_ = j["salt"].asString();
    }

    if (j.isMember("real_name") && j["real_name"].isString()) {
        user.realName_ = j["real_name"].asString();
    }

    if (j.isMember("phone") && j["phone"].isString()) {
        user.phone_ = j["phone"].asString();
    }

    if (j.isMember("email") && j["email"].isString()) {
        user.email_ = j["email"].asString();
    }

    if (j.isMember("role") && j["role"].isString()) {
        user.role_ = stringToRole(j["role"].asString());
    }

    if (j.isMember("status") && j["status"].isString()) {
        user.status_ = stringToStatus(j["status"].asString());
    }

    if (j.isMember("avatar_url") && j["avatar_url"].isString()) {
        user.avatarUrl_ = j["avatar_url"].asString();
    }

    if (j.isMember("bio") && j["bio"].isString()) {
        user.bio_ = j["bio"].asString();
    }

    if (j.isMember("gender") && j["gender"].isString()) {
        user.gender_ = j["gender"].asString();
    }

    if (j.isMember("location") && j["location"].isString()) {
        user.location_ = j["location"].asString();
    }

    if (j.isMember("device_count") && j["device_count"].isInt()) {
        user.deviceCount_ = j["device_count"].asInt();
    }

    if (j.isMember("create_time") && j["create_time"].isInt64()) {
        user.createTime_ = j["create_time"].asInt64();
    }

    if (j.isMember("update_time") && j["update_time"].isInt64()) {
        user.updateTime_ = j["update_time"].asInt64();
    }

    return user;
}

// 验证用户数据
std::string User::validate() const {
    // 验证用户名
    if (username_.empty()) {
        return "用户名不能为空";
    }

    if (username_.length() < 3) {
        return "用户名至少需要3个字符";
    }

    if (username_.length() > 50) {
        return "用户名不能超过50个字符";
    }

    // 用户名只能包含字母、数字和下划线
    std::regex usernameRegex("^[a-zA-Z0-9_]+$");
    if (!std::regex_match(username_, usernameRegex)) {
        return "用户名只能包含字母、数字和下划线";
    }

    // 验证真实姓名
    if (realName_.empty()) {
        return "真实姓名不能为空";
    }

    if (realName_.length() > 50) {
        return "真实姓名不能超过50个字符";
    }

    // 验证手机号
    if (phone_.empty()) {
        return "手机号不能为空";
    }

    // 中国手机号验证（11位数字，1开头）
    std::regex phoneRegex("^1[3-9]\\d{9}$");
    if (!std::regex_match(phone_, phoneRegex)) {
        return "手机号格式不正确";
    }

    // 验证邮箱（可选）
    if (!email_.empty()) {
        std::regex emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        if (!std::regex_match(email_, emailRegex)) {
            return "邮箱格式不正确";
        }

        if (email_.length() > 100) {
            return "邮箱不能超过100个字符";
        }
    }

    return ""; // 验证通过
}

// 将状态转换为字符串
std::string User::statusToString(UserStatus status) {
    switch (status) {
        case UserStatus::ACTIVE:
            return "active";
        case UserStatus::INACTIVE:
            return "inactive";
        case UserStatus::BANNED:
            return "banned";
        default:
            return "unknown";
    }
}

// 将字符串转换为状态
UserStatus User::stringToStatus(const std::string& status) {
    if (status == "active") {
        return UserStatus::ACTIVE;
    } else if (status == "inactive") {
        return UserStatus::INACTIVE;
    } else if (status == "banned") {
        return UserStatus::BANNED;
    }
    return UserStatus::INACTIVE;
}

// 将角色转换为字符串
std::string User::roleToString(UserRole role) {
    switch (role) {
        case UserRole::USER:
            return "user";
        case UserRole::ADMIN:
            return "admin";
        default:
            return "user";
    }
}

// 将字符串转换为角色
UserRole User::stringToRole(const std::string& role) {
    if (role == "user") {
        return UserRole::USER;
    } else if (role == "admin") {
        return UserRole::ADMIN;
    }
    return UserRole::USER;
}

