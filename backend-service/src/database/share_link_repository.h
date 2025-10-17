/**
 * @file share_link_repository.h
 * @brief 分享链接数据访问层
 * @author Knot Development Team
 * @date 2025-10-16
 */

#pragma once

#include "models/share_link.h"
#include <optional>
#include <vector>
#include <mysql/mysql.h>

/**
 * @brief 分享链接数据访问类
 * 
 * 负责share_links表的所有数据库操作
 */
class ShareLinkRepository {
public:
    /**
     * @brief 创建分享链接
     * @param link 分享链接对象（会被设置id）
     * @return true if successful, false otherwise
     */
    static bool create(ShareLink& link);
    
    /**
     * @brief 通过短码查找分享链接
     * @param shortCode 短链接码
     * @return ShareLink对象，如果不存在返回nullopt
     */
    static std::optional<ShareLink> findByShortCode(const std::string& shortCode);
    
    /**
     * @brief 通过目标ID查找分享链接（去重检查）
     * @param targetType 目标类型
     * @param targetId 目标ID
     * @param creatorId 创建者ID（可选，用于更精确的去重）
     * @return ShareLink对象，如果不存在返回nullopt
     */
    static std::optional<ShareLink> findByTargetId(
        ShareLink::TargetType targetType,
        int64_t targetId,
        const std::optional<int64_t>& creatorId = std::nullopt
    );
    
    /**
     * @brief 通过ID查找分享链接
     * @param id 物理ID
     * @return ShareLink对象，如果不存在返回nullopt
     */
    static std::optional<ShareLink> findById(int64_t id);
    
    /**
     * @brief 删除分享链接
     * @param id 物理ID
     * @return true if successful, false otherwise
     */
    static bool deleteById(int64_t id);
    
    /**
     * @brief 删除过期的分享链接
     * @return 删除的记录数
     */
    static int deleteExpired();
    
    /**
     * @brief 获取用户创建的分享链接列表
     * @param creatorId 创建者ID
     * @param limit 限制数量
     * @param offset 偏移量
     * @return 分享链接列表
     */
    static std::vector<ShareLink> findByCreatorId(
        int64_t creatorId,
        int limit = 20,
        int offset = 0
    );

private:
    /**
     * @brief 从MYSQL_ROW构造ShareLink对象
     * @param row 数据库行
     * @return ShareLink对象
     */
    static ShareLink rowToShareLink(MYSQL_ROW row);
};





