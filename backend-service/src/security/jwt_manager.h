/**
 * @file jwt_manager.h
 * @brief JWT 令牌管理模块
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#pragma once

#include <string>
#include <json/json.h>

/**
 * @brief JWT 令牌管理类
 * 
 * 使用 jwt-cpp 库实现 JWT 令牌的生成、验证和解析
 * - 算法: HS256 (HMAC-SHA256)
 * - 访问令牌有效期: 1 小时
 * - 刷新令牌有效期: 24 小时
 */
class JWTManager {
public:
    /**
     * @brief 构造函数
     * 
     * 从 ConfigManager 加载 JWT 配置（密钥、签发者等）
     */
    JWTManager();
    
    /**
     * @brief 析构函数
     */
    ~JWTManager() = default;
    
    /**
     * @brief 生成访问令牌
     * 
     * 生成有效期为 1 小时的访问令牌
     * 
     * @param userId 用户ID
     * @param username 用户名
     * @return JWT 令牌字符串（失败返回空字符串）
     */
    std::string generateAccessToken(int userId, const std::string& username);
    
    /**
     * @brief 生成刷新令牌
     * 
     * 生成有效期为 24 小时的刷新令牌
     * 
     * @param userId 用户ID
     * @param username 用户名
     * @return JWT 令牌字符串（失败返回空字符串）
     */
    std::string generateRefreshToken(int userId, const std::string& username);
    
    /**
     * @brief 验证令牌
     * 
     * 验证令牌的签名和有效期
     * 
     * @param token JWT 令牌
     * @return true 令牌有效，false 令牌无效
     */
    bool validateToken(const std::string& token);
    
    /**
     * @brief 解析令牌内容
     * 
     * 解析令牌并返回其中的声明（claims）
     * 
     * @param token JWT 令牌
     * @return JSON 对象包含令牌内容（失败返回空对象）
     */
    Json::Value decodeToken(const std::string& token);

private:
    /**
     * @brief 生成令牌（内部方法）
     * 
     * @param userId 用户ID
     * @param username 用户名
     * @param expiresInSeconds 有效期（秒）
     * @return JWT 令牌字符串
     */
    std::string generateToken(int userId, const std::string& username, int expiresInSeconds);
    
    std::string secret_;      // JWT 密钥
    std::string issuer_;      // 签发者
    int accessTokenExpiry_;   // 访问令牌有效期（秒）
    int refreshTokenExpiry_;  // 刷新令牌有效期（秒）
};

