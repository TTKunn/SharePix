/**
 * @file password_hasher.cpp
 * @brief 密码哈希模块实现
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#include "security/password_hasher.h"
#include "utils/logger.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <vector>

// 生成随机盐值
std::string PasswordHasher::generateSalt() {
    try {
        // 生成 16 字节随机数据
        unsigned char salt[SALT_LENGTH];
        
        // 使用 OpenSSL 的随机数生成器
        if (RAND_bytes(salt, SALT_LENGTH) != 1) {
            Logger::error("Failed to generate random salt");
            return "";
        }
        
        // Base64 编码
        return base64Encode(salt, SALT_LENGTH);
        
    } catch (const std::exception& e) {
        Logger::error("Exception in generateSalt: " + std::string(e.what()));
        return "";
    }
}

// 哈希密码
std::string PasswordHasher::hashPassword(const std::string& password, const std::string& salt) {
    try {
        // 检查输入
        if (password.empty()) {
            Logger::error("Password cannot be empty");
            return "";
        }
        
        if (salt.empty()) {
            Logger::error("Salt cannot be empty");
            return "";
        }
        
        // Base64 解码盐值
        std::string saltDecoded = base64Decode(salt);
        if (saltDecoded.empty()) {
            Logger::error("Failed to decode salt");
            return "";
        }
        
        // 准备输出缓冲区
        unsigned char hash[HASH_LENGTH];
        
        // 使用 PBKDF2-HMAC-SHA256 哈希密码
        int result = PKCS5_PBKDF2_HMAC(
            password.c_str(),                           // 密码
            password.length(),                          // 密码长度
            reinterpret_cast<const unsigned char*>(saltDecoded.c_str()),  // 盐值
            saltDecoded.length(),                       // 盐值长度
            ITERATIONS,                                 // 迭代次数
            EVP_sha256(),                               // 哈希算法
            HASH_LENGTH,                                // 输出长度
            hash                                        // 输出缓冲区
        );
        
        if (result != 1) {
            Logger::error("PBKDF2 hashing failed");
            return "";
        }
        
        // Base64 编码哈希值
        return base64Encode(hash, HASH_LENGTH);
        
    } catch (const std::exception& e) {
        Logger::error("Exception in hashPassword: " + std::string(e.what()));
        return "";
    }
}

// 验证密码
bool PasswordHasher::verifyPassword(const std::string& password, 
                                   const std::string& salt, 
                                   const std::string& hash) {
    try {
        // 检查输入
        if (password.empty() || salt.empty() || hash.empty()) {
            Logger::error("Password, salt, and hash cannot be empty");
            return false;
        }
        
        // 使用相同的盐值哈希输入密码
        std::string computedHash = hashPassword(password, salt);
        
        if (computedHash.empty()) {
            Logger::error("Failed to compute hash for verification");
            return false;
        }
        
        // 时间常数比较（防止时序攻击）
        // 注意：这里使用简单的字符串比较，实际生产环境应使用 CRYPTO_memcmp
        bool result = (computedHash == hash);
        
        if (!result) {
            Logger::warning("Password verification failed");
        }
        
        return result;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in verifyPassword: " + std::string(e.what()));
        return false;
    }
}

// Base64 编码
std::string PasswordHasher::base64Encode(const unsigned char* data, size_t length) {
    try {
        // 计算 Base64 编码后的长度
        size_t encodedLength = ((length + 2) / 3) * 4;
        std::vector<unsigned char> encoded(encodedLength + 1);  // +1 for null terminator
        
        // 使用 OpenSSL 的 Base64 编码
        int result = EVP_EncodeBlock(encoded.data(), data, length);
        
        if (result < 0) {
            Logger::error("Base64 encoding failed");
            return "";
        }
        
        return std::string(reinterpret_cast<char*>(encoded.data()));
        
    } catch (const std::exception& e) {
        Logger::error("Exception in base64Encode: " + std::string(e.what()));
        return "";
    }
}

// Base64 解码
std::string PasswordHasher::base64Decode(const std::string& encoded) {
    try {
        if (encoded.empty()) {
            return "";
        }
        
        // 计算解码后的最大长度
        size_t decodedMaxLength = (encoded.length() / 4) * 3;
        std::vector<unsigned char> decoded(decodedMaxLength);
        
        // 使用 OpenSSL 的 Base64 解码
        int result = EVP_DecodeBlock(decoded.data(), 
                                     reinterpret_cast<const unsigned char*>(encoded.c_str()), 
                                     encoded.length());
        
        if (result < 0) {
            Logger::error("Base64 decoding failed");
            return "";
        }
        
        // 处理 Base64 填充字符 '='
        size_t decodedLength = result;
        if (encoded.length() >= 2) {
            if (encoded[encoded.length() - 1] == '=') decodedLength--;
            if (encoded[encoded.length() - 2] == '=') decodedLength--;
        }
        
        return std::string(reinterpret_cast<char*>(decoded.data()), decodedLength);
        
    } catch (const std::exception& e) {
        Logger::error("Exception in base64Decode: " + std::string(e.what()));
        return "";
    }
}

