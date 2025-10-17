/**
 * @file share_link.h
 * @brief 分享链接模型类
 * @author Knot Development Team
 * @date 2025-10-16
 */

#pragma once

#include <string>
#include <ctime>
#include <json/json.h>
#include <optional>
#include <cstdint>

/**
 * @brief 分享链接模型类
 * 
 * 对应数据库表：share_links
 */
class ShareLink {
public:
    /**
     * @brief 目标类型枚举
     */
    enum class TargetType {
        POST,   // 帖子
        USER,   // 用户（预留）
        TAG     // 标签（预留）
    };

    // 构造函数
    ShareLink() = default;

    // Getters
    int64_t getId() const { return id_; }
    std::string getShortCode() const { return shortCode_; }
    TargetType getTargetType() const { return targetType_; }
    int64_t getTargetId() const { return targetId_; }
    std::optional<int64_t> getCreatorId() const { return creatorId_; }
    time_t getCreateTime() const { return createTime_; }
    std::optional<time_t> getExpireTime() const { return expireTime_; }

    // Setters
    void setId(int64_t id) { id_ = id; }
    void setShortCode(const std::string& code) { shortCode_ = code; }
    void setTargetType(TargetType type) { targetType_ = type; }
    void setTargetId(int64_t id) { targetId_ = id; }
    void setCreatorId(const std::optional<int64_t>& id) { creatorId_ = id; }
    void setCreateTime(time_t time) { createTime_ = time; }
    void setExpireTime(const std::optional<time_t>& time) { expireTime_ = time; }

    // 工具方法
    /**
     * @brief 检查链接是否过期
     * @return true if expired, false otherwise
     */
    bool isExpired() const;
    
    /**
     * @brief 获取完整的短链接URL
     * @param baseUrl 基础URL（默认从配置读取）
     * @return 完整URL（如: http://8.138.115.164:8080/s/ABC12345）
     */
    std::string getFullUrl(const std::string& baseUrl = "") const;
    
    /**
     * @brief 将TargetType转换为字符串
     * @param type 目标类型
     * @return 类型字符串
     */
    static std::string targetTypeToString(TargetType type);
    
    /**
     * @brief 将字符串转换为TargetType
     * @param str 类型字符串
     * @return 目标类型
     */
    static TargetType stringToTargetType(const std::string& str);

    // JSON序列化
    /**
     * @brief 转换为JSON对象
     * @return JSON对象
     */
    Json::Value toJson() const;
    
    /**
     * @brief 从JSON对象构造ShareLink
     * @param json JSON对象
     * @return ShareLink对象
     */
    static ShareLink fromJson(const Json::Value& json);

private:
    int64_t id_ = 0;                                    // 物理ID
    std::string shortCode_;                             // 短链接码（8位Base62）
    TargetType targetType_ = TargetType::POST;          // 目标类型
    int64_t targetId_ = 0;                              // 目标物理ID
    std::optional<int64_t> creatorId_;                  // 创建者ID（可选）
    time_t createTime_ = 0;                             // 创建时间
    std::optional<time_t> expireTime_;                  // 过期时间（可选）
};





