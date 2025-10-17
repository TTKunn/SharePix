/**
 * @file share_service.cpp
 * @brief 分享服务业务逻辑层实现
 * @author Knot Development Team
 * @date 2025-10-16
 */

#include "core/share_service.h"
#include "database/share_link_repository.h"
#include "database/post_repository.h"
#include "utils/id_generator.h"
#include "utils/config_manager.h"
#include "utils/logger.h"
#include <ctime>
#include <sstream>

// 创建帖子分享链接
CreateShareLinkResult ShareService::createShareLink(
    int64_t postId,
    const std::optional<int64_t>& creatorId,
    int expireDays
) {
    CreateShareLinkResult result;
    result.success = false;
    result.isReused = false;
    
    // 1. 检查帖子是否存在
    if (!postExists(postId)) {
        result.message = "帖子不存在";
        Logger::warning("Cannot create share link: post " + std::to_string(postId) + " not found");
        return result;
    }
    
    // 2. 检查是否已存在分享链接（去重）
    auto existingLink = ShareLinkRepository::findByTargetId(
        ShareLink::TargetType::POST,
        postId,
        creatorId
    );
    
    if (existingLink.has_value()) {
        // 检查现有链接是否过期
        if (!existingLink->isExpired()) {
            // 返回现有链接
            result.success = true;
            result.isReused = true;
            result.message = "返回已存在的分享链接";
            result.shortCode = existingLink->getShortCode();
            result.shortUrl = existingLink->getFullUrl();
            
            // 获取帖子信息以生成Deep Links
            PostRepository repo;
            auto post = repo.findById(postId);
            if (post.has_value()) {
                result.deepLinks = generateDeepLinks(post->getPostId());
            }
            
            Logger::info("Reusing existing share link: " + result.shortCode);
            return result;
        } else {
            // 过期的链接，删除后重新创建
            ShareLinkRepository::deleteById(existingLink->getId());
            Logger::info("Deleted expired share link: " + existingLink->getShortCode());
        }
    }
    
    // 3. 生成短码
    std::string shortCode = IdGenerator::generateShareCode();
    
    // 确保短码唯一性（极小概率冲突，但需要检查）
    int retryCount = 0;
    const int maxRetries = 5;
    while (ShareLinkRepository::findByShortCode(shortCode).has_value() && retryCount < maxRetries) {
        shortCode = IdGenerator::generateShareCode();
        retryCount++;
        Logger::warning("Short code collision detected, regenerating... (retry " + 
                       std::to_string(retryCount) + ")");
    }
    
    if (retryCount >= maxRetries) {
        result.message = "生成短码失败，请重试";
        Logger::error("Failed to generate unique short code after " + std::to_string(maxRetries) + " retries");
        return result;
    }
    
    // 4. 构造ShareLink对象
    ShareLink link;
    link.setShortCode(shortCode);
    link.setTargetType(ShareLink::TargetType::POST);
    link.setTargetId(postId);
    link.setCreatorId(creatorId);
    link.setCreateTime(std::time(nullptr));
    
    // 设置过期时间
    if (expireDays > 0) {
        time_t expireTime = std::time(nullptr) + (expireDays * 24 * 60 * 60);
        link.setExpireTime(expireTime);
    }
    
    // 5. 保存到数据库
    if (!ShareLinkRepository::create(link)) {
        result.message = "创建分享链接失败";
        Logger::error("Failed to create share link in database");
        return result;
    }
    
    // 6. 构造返回结果
    result.success = true;
    result.message = "分享链接创建成功";
    result.shortCode = shortCode;
    result.shortUrl = link.getFullUrl();
    
    // 7. 生成Deep Links
    PostRepository repo;
    auto post = repo.findById(postId);
    if (post.has_value()) {
        result.deepLinks = generateDeepLinks(post->getPostId());
    }
    
    Logger::info("Created share link: " + shortCode + " for post " + std::to_string(postId));
    
    return result;
}

// 解析分享链接获取帖子信息
ResolveShareLinkResult ShareService::resolveShareLink(const std::string& shortCode) {
    ResolveShareLinkResult result;
    result.success = false;
    result.expired = false;
    
    // 1. 查找分享链接
    auto link = ShareLinkRepository::findByShortCode(shortCode);
    
    if (!link.has_value()) {
        result.message = "分享链接不存在";
        Logger::warning("Share link not found: " + shortCode);
        return result;
    }
    
    // 2. 检查是否过期
    if (link->isExpired()) {
        result.expired = true;
        result.message = "分享链接已过期";
        Logger::info("Share link expired: " + shortCode);
        return result;
    }
    
    // 3. 根据目标类型获取内容
    if (link->getTargetType() == ShareLink::TargetType::POST) {
        PostRepository repo;
        auto post = repo.findById(link->getTargetId());
        
        if (!post.has_value()) {
            result.message = "目标帖子不存在";
            Logger::warning("Target post not found for share link: " + shortCode);
            return result;
        }
        
        result.success = true;
        result.message = "解析成功";
        result.post = post;
        
        Logger::info("Resolved share link " + shortCode + " to post " + std::to_string(link->getTargetId()));
        
    } else {
        result.message = "不支持的目标类型";
        Logger::error("Unsupported target type for share link: " + shortCode);
    }
    
    return result;
}

// 删除分享链接
bool ShareService::deleteShareLink(int64_t id, int64_t userId) {
    // 1. 查找分享链接
    auto link = ShareLinkRepository::findById(id);
    
    if (!link.has_value()) {
        Logger::warning("Share link not found for deletion: " + std::to_string(id));
        return false;
    }
    
    // 2. 权限检查：只有创建者可以删除
    if (link->getCreatorId().has_value() && link->getCreatorId().value() != userId) {
        Logger::warning("User " + std::to_string(userId) + 
                       " attempted to delete share link created by " + 
                       std::to_string(link->getCreatorId().value()));
        return false;
    }
    
    // 3. 执行删除
    bool deleted = ShareLinkRepository::deleteById(id);
    
    if (deleted) {
        Logger::info("User " + std::to_string(userId) + " deleted share link " + 
                    link->getShortCode());
    }
    
    return deleted;
}

// 生成Deep Link URLs（私有方法）
Json::Value ShareService::generateDeepLinks(const std::string& postId) {
    Json::Value deepLinks;
    
    auto& config = ConfigManager::getInstance();
    std::string domain = config.get<std::string>("server.domain", "knot.app");
    std::string env = config.get<std::string>("server.environment", "development");
    
    // 判断是否为生产环境
    bool isProduction = (env == "production");
    
    // iOS Universal Link（生产环境才使用域名验证）
    if (isProduction) {
        deepLinks["ios"] = "https://" + domain + "/post/" + postId;
    } else {
        // 开发环境iOS暂不支持自定义Scheme，需要Xcode配置
        deepLinks["ios"] = "knot://post/" + postId;
        deepLinks["ios_note"] = "开发环境：需要在Xcode中配置URL Scheme";
    }
    
    // Android App Link
    if (isProduction) {
        deepLinks["android"] = "https://" + domain + "/post/" + postId;
    } else {
        // 开发环境使用自定义Scheme
        deepLinks["android"] = "knot://post/" + postId;
        deepLinks["android_note"] = "开发环境：需要在AndroidManifest.xml中配置intent-filter";
    }
    
    // HarmonyOS - 推荐字段（根据环境自动选择）
    deepLinks["harmonyos"] = "knot://post/" + postId;  // 自定义Scheme，适合开发测试
    
    // HarmonyOS - 完整选项
    deepLinks["harmonyos_scheme"] = "knot://post/" + postId;  // 自定义Scheme（开发/测试推荐）
    if (isProduction) {
        deepLinks["harmonyos_app_link"] = "https://" + domain + "/post/" + postId;  // App Linking（生产环境推荐）
    }
    
    // 环境标识
    deepLinks["environment"] = env;
    deepLinks["recommendation"] = isProduction 
        ? "生产环境：建议使用域名验证的App Link" 
        : "开发环境：建议使用自定义Scheme (knot://)，无需域名备案";
    
    return deepLinks;
}

// 检查帖子是否存在（私有方法）
bool ShareService::postExists(int64_t postId) {
    PostRepository repo;
    auto post = repo.findById(postId);
    return post.has_value();
}

