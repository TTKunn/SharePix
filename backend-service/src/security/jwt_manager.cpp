/**
 * @file jwt_manager.cpp
 * @brief JWT 令牌管理模块实现
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#include "security/jwt_manager.h"
#include "utils/config_manager.h"
#include "utils/logger.h"
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/open-source-parsers-jsoncpp/traits.h>
#include <chrono>

// 使用 jsoncpp traits
using jwt_jsoncpp = jwt::basic_claim<jwt::traits::open_source_parsers_jsoncpp>;

// 构造函数
JWTManager::JWTManager() {
    try {
        // 从配置文件加载 JWT 配置
        secret_ = ConfigManager::getInstance().get<std::string>("jwt.secret");
        issuer_ = ConfigManager::getInstance().get<std::string>("jwt.issuer");
        accessTokenExpiry_ = ConfigManager::getInstance().get<int>("jwt.expires_in");
        refreshTokenExpiry_ = ConfigManager::getInstance().get<int>("jwt.refresh_expires_in");

        // 验证配置是否有效
        if (secret_.empty()) {
            Logger::error("JWT secret is empty, using default");
            secret_ = "default_secret_change_in_production";
        }
        if (issuer_.empty()) {
            Logger::error("JWT issuer is empty, using default");
            issuer_ = "shared-parking-auth";
        }

        Logger::info("JWTManager initialized successfully");
        Logger::debug("JWT issuer: " + issuer_);
        Logger::debug("JWT secret (first 10 chars): " + secret_.substr(0, std::min(10, static_cast<int>(secret_.length()))));
        Logger::debug("Access token expiry: " + std::to_string(accessTokenExpiry_) + " seconds");
        Logger::debug("Refresh token expiry: " + std::to_string(refreshTokenExpiry_) + " seconds");

    } catch (const std::exception& e) {
        Logger::error("Failed to initialize JWTManager: " + std::string(e.what()));
        // 使用默认值 - 注意:issuer必须与配置文件中的值一致
        secret_ = "default_secret_change_in_production";
        issuer_ = "shared-parking-auth";  // ⚠️ 修复:与config.json保持一致
        accessTokenExpiry_ = 3600;      // 1 小时
        refreshTokenExpiry_ = 86400;    // 24 小时
        Logger::warning("Using default JWT configuration");
    }
}

// 生成访问令牌
std::string JWTManager::generateAccessToken(int userId, const std::string& username) {
    return generateToken(userId, username, accessTokenExpiry_);
}

// 生成刷新令牌
std::string JWTManager::generateRefreshToken(int userId, const std::string& username) {
    return generateToken(userId, username, refreshTokenExpiry_);
}

// 生成令牌（内部方法）
std::string JWTManager::generateToken(int userId, const std::string& username, int expiresInSeconds) {
    try {
        // 获取当前时间
        auto now = std::chrono::system_clock::now();
        auto expiry = now + std::chrono::seconds(expiresInSeconds);
        
        // 创建 JWT 令牌
        auto token = jwt::create<jwt::traits::open_source_parsers_jsoncpp>()
            .set_issuer(issuer_)                                    // 签发者
            .set_subject(std::to_string(userId))                    // 主题（用户ID）
            .set_issued_at(now)                                     // 签发时间
            .set_expires_at(expiry)                                 // 过期时间
            .set_payload_claim("username", jwt_jsoncpp(username))   // 自定义声明：用户名
            .sign(jwt::algorithm::hs256{secret_});                  // 使用 HS256 签名
        
        Logger::debug("Generated JWT token for user: " + username);
        return token;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to generate JWT token: " + std::string(e.what()));
        return "";
    }
}

// 验证令牌
bool JWTManager::validateToken(const std::string& token) {
    try {
        if (token.empty()) {
            Logger::warning("Empty token provided for validation");
            return false;
        }

        Logger::debug("Validating token (first 30 chars): " + token.substr(0, std::min(30, static_cast<int>(token.length()))) + "...");
        Logger::debug("Using secret (first 10 chars): " + secret_.substr(0, std::min(10, static_cast<int>(secret_.length()))));
        Logger::debug("Expected issuer: " + issuer_);

        // 先解码token查看内容
        auto decoded = jwt::decode<jwt::traits::open_source_parsers_jsoncpp>(token);
        std::string tokenIssuer = decoded.get_issuer();
        Logger::debug("Token issuer: " + tokenIssuer);

        // 创建验证器
        auto verifier = jwt::verify<jwt::traits::open_source_parsers_jsoncpp>()
            .allow_algorithm(jwt::algorithm::hs256{secret_})    // 允许的算法
            .with_issuer(issuer_);                              // 验证签发者

        // 验证令牌
        verifier.verify(decoded);

        Logger::debug("Token validation successful");
        return true;

    } catch (const jwt::error::token_verification_exception& e) {
        Logger::warning("Token verification failed: " + std::string(e.what()));
        return false;
    } catch (const std::exception& e) {
        Logger::error("Exception in validateToken: " + std::string(e.what()));
        return false;
    }
}

// 解析令牌内容
Json::Value JWTManager::decodeToken(const std::string& token) {
    Json::Value result;
    
    try {
        if (token.empty()) {
            Logger::warning("Empty token provided for decoding");
            return result;
        }
        
        // 解码令牌
        auto decoded = jwt::decode<jwt::traits::open_source_parsers_jsoncpp>(token);
        
        // 提取声明
        result["issuer"] = decoded.get_issuer();
        result["subject"] = decoded.get_subject();
        
        // 提取时间戳
        if (decoded.has_issued_at()) {
            auto iat = decoded.get_issued_at();
            result["issued_at"] = static_cast<Json::Int64>(
                std::chrono::system_clock::to_time_t(iat)
            );
        }
        
        if (decoded.has_expires_at()) {
            auto exp = decoded.get_expires_at();
            result["expires_at"] = static_cast<Json::Int64>(
                std::chrono::system_clock::to_time_t(exp)
            );
        }
        
        // 提取自定义声明
        if (decoded.has_payload_claim("username")) {
            auto username_claim = decoded.get_payload_claim("username");
            result["username"] = username_claim.as_string();
        }
        
        Logger::debug("Token decoded successfully");
        return result;
        
    } catch (const std::exception& e) {
        Logger::error("Failed to decode token: " + std::string(e.what()));
        return result;
    }
}

