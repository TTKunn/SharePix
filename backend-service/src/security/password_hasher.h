/**
 * @file password_hasher.h
 * @brief 密码哈希模块 - 使用 PBKDF2-HMAC-SHA256 算法
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#pragma once

#include <string>

/**
 * @brief 密码哈希工具类
 * 
 * 使用 OpenSSL 的 PBKDF2-HMAC-SHA256 算法实现密码哈希功能
 * - 迭代次数: 100,000 次（符合 OWASP 2023 标准）
 * - 盐值长度: 16 字节
 * - 哈希输出: 32 字节
 * - 编码方式: Base64
 */
class PasswordHasher {
public:
    /**
     * @brief 生成随机盐值
     * 
     * 使用 OpenSSL 的 RAND_bytes 生成 16 字节的随机盐值，
     * 并使用 Base64 编码返回
     * 
     * @return Base64 编码的盐值字符串（失败返回空字符串）
     */
    static std::string generateSalt();
    
    /**
     * @brief 哈希密码
     * 
     * 使用 PBKDF2-HMAC-SHA256 算法哈希密码
     * 
     * @param password 原始密码
     * @param salt Base64 编码的盐值
     * @return Base64 编码的哈希值（失败返回空字符串）
     */
    static std::string hashPassword(const std::string& password, const std::string& salt);
    
    /**
     * @brief 验证密码
     * 
     * 使用相同的盐值哈希输入密码，并与存储的哈希值比较
     * 
     * @param password 待验证的密码
     * @param salt Base64 编码的盐值
     * @param hash Base64 编码的存储哈希值
     * @return true 密码正确，false 密码错误
     */
    static bool verifyPassword(const std::string& password, 
                              const std::string& salt, 
                              const std::string& hash);

private:
    /**
     * @brief Base64 编码
     * 
     * @param data 原始数据
     * @param length 数据长度
     * @return Base64 编码的字符串
     */
    static std::string base64Encode(const unsigned char* data, size_t length);
    
    /**
     * @brief Base64 解码
     * 
     * @param encoded Base64 编码的字符串
     * @return 解码后的字符串
     */
    static std::string base64Decode(const std::string& encoded);
    
    // 常量定义
    static constexpr int SALT_LENGTH = 16;        // 盐值长度（字节）
    static constexpr int HASH_LENGTH = 32;        // 哈希长度（字节）
    static constexpr int ITERATIONS = 100000;     // PBKDF2 迭代次数
};

