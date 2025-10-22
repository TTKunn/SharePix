/**
 * @file auth_service.h
 * @brief 认证服务 - 业务逻辑层
 * @author Knot Team
 * @date 2025-10-12
 */

#pragma once

#include "models/user.h"
#include <optional>
#include <string>
#include <utility>
#include <memory>

// 前向声明
class UserRepository;
class JWTManager;

// ============================================================================
// Result结构体定义
// ============================================================================

/**
 * @brief 注册结果结构体
 */
struct RegisterResult {
    bool success;
    std::string message;
    User user;

    RegisterResult() : success(false), message("") {}
};

/**
 * @brief 认证结果结构体（用于登录和刷新令牌）
 */
struct AuthResult {
    bool success;
    std::string message;
    std::string accessToken;
    std::string refreshToken;
    User user;

    AuthResult() : success(false), message("") {}
};

/**
 * @brief 令牌验证结果结构体
 */
struct TokenValidationResult {
    bool valid;
    std::string message;
    int userId;
    std::string username;

    TokenValidationResult() : valid(false), message(""), userId(0) {}
};

/**
 * @brief 更新用户信息结果结构体
 */
struct UpdateProfileResult {
    bool success;
    std::string message;
    User user;

    UpdateProfileResult() : success(false), message("") {}
};

/**
 * @brief 用户名检查结果结构体
 */
struct UsernameCheckResult {
    bool valid;       // 格式是否有效
    bool available;   // 是否可用（未被占用）
    std::string message;

    UsernameCheckResult() : valid(false), available(false), message("") {}
};

/**
 * @brief 头像上传结果结构体
 */
struct UploadAvatarResult {
    bool success;              // 上传是否成功
    std::string message;       // 错误信息（如果失败）
    std::string avatarUrl;     // 头像URL（相对路径）
    int width;                 // 图片宽度（固定200）
    int height;                // 图片高度（固定200）
    long long fileSize;        // 文件大小（字节）
    
    UploadAvatarResult() : success(false), width(0), height(0), fileSize(0) {}
};

// ============================================================================
// AuthService类定义
// ============================================================================

/**
 * @brief 认证服务类
 *
 * 负责用户认证相关的业务逻辑：
 * - 用户注册
 * - 用户登录
 * - JWT令牌管理
 * - 密码管理
 * - 用户信息管理
 */
class AuthService {
public:
    /**
     * @brief 构造函数
     */
    AuthService();

    /**
     * @brief 析构函数
     */
    ~AuthService();

    /**
     * @brief 用户注册
     *
     * @param username 用户名
     * @param password 密码
     * @param realName 真实姓名
     * @param phone 手机号
     * @param email 邮箱（可选）
     * @param role 用户角色
     * @return RegisterResult 注册结果
     */
    RegisterResult registerUser(
        const std::string& username,
        const std::string& password,
        const std::string& realName,
        const std::string& phone,
        const std::string& email,
        UserRole role
    );

    /**
     * @brief 用户登录
     *
     * 支持用户名/手机号/邮箱登录
     *
     * @param username 用户名/手机号/邮箱
     * @param password 密码
     * @return AuthResult 登录结果（包含令牌和用户信息）
     */
    AuthResult loginUser(
        const std::string& username,
        const std::string& password
    );

    /**
     * @brief 验证令牌
     *
     * @param token JWT令牌
     * @return TokenValidationResult 验证结果
     */
    TokenValidationResult validateToken(const std::string& token);

    /**
     * @brief 刷新令牌
     *
     * @param refreshToken 刷新令牌
     * @return AuthResult 新的令牌对
     */
    AuthResult refreshTokens(const std::string& refreshToken);

    /**
     * @brief 用户登出
     *
     * 注意：JWT是无状态的，这里只做简单验证
     *
     * @param accessToken 访问令牌
     * @return true 登出成功，false 登出失败
     */
    bool logoutUser(const std::string& accessToken);

    /**
     * @brief 修改密码
     *
     * @param userId 用户ID（物理ID）
     * @param oldPassword 旧密码
     * @param newPassword 新密码
     * @return true 修改成功，false 修改失败
     */
    bool changePassword(int userId, const std::string& oldPassword, const std::string& newPassword);

    /**
     * @brief 获取用户完整信息（v2.1.0）
     *
     * @param userId 用户ID（物理ID）
     * @return 用户对象（不含密码和盐值），失败返回nullopt
     */
    std::optional<User> getUserProfile(int userId);

    /**
     * @brief 更新用户信息（v2.1.0, v2.9.0新增username）
     *
     * @param userId 用户ID（物理ID）
     * @param username 用户名
     * @param realName 真实姓名
     * @param email 邮箱
     * @param phone 手机号
     * @param avatarUrl 头像URL
     * @param bio 个人简介
     * @param gender 性别
     * @param location 所在地
     * @return UpdateProfileResult 更新结果
     */
    UpdateProfileResult updateUserProfile(
        int userId,
        const std::string& username,
        const std::string& realName,
        const std::string& email,
        const std::string& phone,
        const std::string& avatarUrl,
        const std::string& bio,
        const std::string& gender,
        const std::string& location
    );

    /**
     * @brief 检查用户名可用性（v2.1.0）
     *
     * @param username 用户名
     * @return UsernameCheckResult 检查结果
     */
    UsernameCheckResult checkUsernameAvailability(const std::string& username);

    /**
     * @brief 获取用户公开信息（v2.1.0）
     *
     * @param userId 用户逻辑ID（USR_XXXXXX）
     * @return 用户公开信息，失败返回nullopt
     */
    std::optional<User> getUserPublicInfo(const std::string& userId);

    /**
     * @brief 上传用户头像（v2.6.0）
     * 
     * 处理流程：
     * 1. 获取用户信息（获取userId逻辑ID）
     * 2. 调用AvatarProcessor处理图片
     * 3. 删除旧头像文件
     * 4. 更新数据库中的avatar_url字段
     * 5. 返回结果（包含新头像URL）
     * 
     * @param userId 用户物理ID
     * @param tempFilePath 临时文件路径
     * @return UploadAvatarResult 上传结果
     */
    UploadAvatarResult uploadAvatar(int userId, const std::string& tempFilePath);

private:
    std::unique_ptr<UserRepository> userRepo_;
    std::unique_ptr<JWTManager> jwtManager_;

    /**
     * @brief 生成唯一用户ID
     *
     * 格式：USR_2025Q1_XXXXXX（6位随机字母数字）
     *
     * @return 用户逻辑ID
     */
    std::string generateUserId();

    /**
     * @brief 验证用户名格式
     *
     * @param username 用户名
     * @return true 格式正确，false 格式错误
     */
    bool validateUsername(const std::string& username);

    /**
     * @brief 验证密码格式
     *
     * @param password 密码
     * @return true 格式正确，false 格式错误
     */
    bool validatePassword(const std::string& password);

    /**
     * @brief 验证手机号格式
     *
     * @param phone 手机号
     * @return true 格式正确，false 格式错误
     */
    bool validatePhone(const std::string& phone);

    /**
     * @brief 验证邮箱格式
     *
     * @param email 邮箱
     * @return true 格式正确，false 格式错误
     */
    bool validateEmail(const std::string& email);
};
