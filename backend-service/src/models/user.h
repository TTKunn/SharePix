/**
 * @file user.h
 * @brief User model definition
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

/**
 * @brief User status enumeration
 */
enum class UserStatus {
    ACTIVE,      // 活跃
    INACTIVE,    // 未激活
    BANNED       // 已封禁
};

/**
 * @brief User role enumeration
 */
enum class UserRole {
    USER,        // 普通用户
    ADMIN        // 管理员
};

/**
 * @brief User model class
 */
class User {
public:
    /**
     * @brief Default constructor
     */
    User();
    
    /**
     * @brief Constructor with parameters
     * @param id Physical ID (auto-increment)
     * @param userId Logical ID (business generated)
     * @param username Username
     * @param phone Phone number
     */
    User(int id, const std::string& userId, const std::string& username, const std::string& phone);
    
    /**
     * @brief Destructor
     */
    ~User() = default;
    
    // Getters
    int getId() const { return id_; }
    const std::string& getUserId() const { return userId_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getPassword() const { return password_; }
    const std::string& getSalt() const { return salt_; }
    const std::string& getRealName() const { return realName_; }
    const std::string& getPhone() const { return phone_; }
    const std::string& getEmail() const { return email_; }
    UserRole getRole() const { return role_; }
    UserStatus getStatus() const { return status_; }
    const std::string& getAvatarUrl() const { return avatarUrl_; }
    int getDeviceCount() const { return deviceCount_; }
    std::time_t getCreateTime() const { return createTime_; }
    std::time_t getUpdateTime() const { return updateTime_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setUserId(const std::string& userId) { userId_ = userId; }
    void setUsername(const std::string& username) { username_ = username; }
    void setPassword(const std::string& password) { password_ = password; }
    void setSalt(const std::string& salt) { salt_ = salt; }
    void setRealName(const std::string& realName) { realName_ = realName; }
    void setPhone(const std::string& phone) { phone_ = phone; }
    void setEmail(const std::string& email) { email_ = email; }
    void setRole(UserRole role) { role_ = role; }
    void setStatus(UserStatus status) { status_ = status; }
    void setAvatarUrl(const std::string& avatarUrl) { avatarUrl_ = avatarUrl; }
    void setDeviceCount(int deviceCount) { deviceCount_ = deviceCount; }
    void setCreateTime(std::time_t createTime) { createTime_ = createTime; }
    void setUpdateTime(std::time_t updateTime) { updateTime_ = updateTime; }
    
    /**
     * @brief Convert user to JSON
     * @param includeSecrets Include password hash and salt
     * @return JSON representation
     */
    Json::Value toJson(bool includeSecrets = false) const;

    /**
     * @brief Create user from JSON
     * @param j JSON object
     * @return User object
     */
    static User fromJson(const Json::Value& j);
    
    /**
     * @brief Validate user data
     * @return Validation error message (empty if valid)
     */
    std::string validate() const;
    
    /**
     * @brief Check if user is active
     * @return true if active, false otherwise
     */
    bool isActive() const { return status_ == UserStatus::ACTIVE; }
    
    /**
     * @brief Convert UserStatus to string
     * @param status User status
     * @return String representation
     */
    static std::string statusToString(UserStatus status);

    /**
     * @brief Convert string to UserStatus
     * @param statusStr Status string
     * @return UserStatus enum value
     */
    static UserStatus stringToStatus(const std::string& statusStr);

    /**
     * @brief Convert UserRole to string
     * @param role User role
     * @return String representation
     */
    static std::string roleToString(UserRole role);

    /**
     * @brief Convert string to UserRole
     * @param roleStr Role string
     * @return UserRole enum value
     */
    static UserRole stringToRole(const std::string& roleStr);

private:
    int id_;                      // 物理ID（自增主键）
    std::string userId_;          // 逻辑ID（业务生成）
    std::string username_;        // 用户名
    std::string password_;        // 加密密码（password_hash）
    std::string salt_;            // 密码盐值
    std::string realName_;        // 真实姓名
    std::string phone_;           // 手机号
    std::string email_;           // 邮箱
    UserRole role_;               // 用户角色
    UserStatus status_;           // 账户状态
    std::string avatarUrl_;       // 头像URL
    int deviceCount_;             // 绑定设备数
    std::time_t createTime_;      // 创建时间
    std::time_t updateTime_;      // 更新时间
};
