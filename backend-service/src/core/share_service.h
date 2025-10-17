/**
 * @file share_service.h
 * @brief 分享服务业务逻辑层
 * @author Knot Development Team
 * @date 2025-10-16
 */

#pragma once

#include "models/share_link.h"
#include "models/post.h"
#include <string>
#include <optional>
#include <cstdint>
#include <json/json.h>

/**
 * @brief 创建分享链接结果
 */
struct CreateShareLinkResult {
    bool success;
    std::string message;
    std::string shortCode;
    std::string shortUrl;      // http://host:port/s/ABC12345
    Json::Value deepLinks;     // Deep Link URLs for different platforms
    bool isReused;             // true if returned existing link
};

/**
 * @brief 解析分享链接结果
 */
struct ResolveShareLinkResult {
    bool success;
    std::string message;
    std::optional<Post> post;  // 帖子详情
    bool expired;              // 链接是否过期
};

/**
 * @brief 分享服务类
 */
class ShareService {
public:
    /**
     * @brief 创建帖子分享链接
     * @param postId 帖子物理ID
     * @param creatorId 创建者ID（可选，未登录用户为nullopt）
     * @param expireDays 过期天数（0表示永不过期）
     * @return 创建结果
     */
    static CreateShareLinkResult createShareLink(
        int64_t postId,
        const std::optional<int64_t>& creatorId = std::nullopt,
        int expireDays = 0
    );
    
    /**
     * @brief 解析分享链接获取帖子信息
     * @param shortCode 短链接码
     * @return 解析结果（包含帖子详情）
     */
    static ResolveShareLinkResult resolveShareLink(const std::string& shortCode);
    
    /**
     * @brief 删除分享链接
     * @param id 分享链接ID
     * @param userId 操作用户ID
     * @return true if successful, false otherwise
     */
    static bool deleteShareLink(int64_t id, int64_t userId);

private:
    /**
     * @brief 生成Deep Link URLs
     * @param postId 帖子业务ID（如POST_2025Q4_ABC123）
     * @return Deep Link JSON对象
     */
    static Json::Value generateDeepLinks(const std::string& postId);
    
    /**
     * @brief 检查帖子是否存在
     * @param postId 帖子物理ID
     * @return true if exists, false otherwise
     */
    static bool postExists(int64_t postId);
};





