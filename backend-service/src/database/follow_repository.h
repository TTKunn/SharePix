/**
 * @file follow_repository.h
 * @brief 关注数据访问层定义
 * @author Knot Team
 * @date 2025-10-16
 */

#pragma once

#include "models/follow.h"
#include <mysql/mysql.h>
#include <optional>
#include <vector>
#include <map>
#include <string>

/**
 * @brief 关注数据访问类
 *
 * 负责关注数据的CRUD操作
 */
class FollowRepository {
public:
    /**
     * @brief 构造函数
     */
    FollowRepository() = default;

    /**
     * @brief 析构函数
     */
    ~FollowRepository() = default;

    /**
     * @brief 创建关注记录
     * @param conn MySQL连接
     * @param followerId 关注者ID（物理ID）
     * @param followeeId 被关注者ID（物理ID）
     * @return 成功返回true，失败返回false
     */
    bool create(MYSQL* conn, int64_t followerId, int64_t followeeId);

    /**
     * @brief 删除关注记录（取消关注）
     * @param conn MySQL连接
     * @param followerId 关注者ID（物理ID）
     * @param followeeId 被关注者ID（物理ID）
     * @return 成功返回true，失败返回false
     */
    bool deleteByFollowerAndFollowee(MYSQL* conn, int64_t followerId, int64_t followeeId);

    /**
     * @brief 检查关注关系是否存在
     * @param conn MySQL连接
     * @param followerId 关注者ID（物理ID）
     * @param followeeId 被关注者ID（物理ID）
     * @return 存在返回true，否则返回false
     */
    bool exists(MYSQL* conn, int64_t followerId, int64_t followeeId);

    /**
     * @brief 统计用户关注的人数
     * @param conn MySQL连接
     * @param userId 用户ID（物理ID）
     * @return 关注数
     */
    int countFollowing(MYSQL* conn, int64_t userId);

    /**
     * @brief 统计用户的粉丝数
     * @param conn MySQL连接
     * @param userId 用户ID（物理ID）
     * @return 粉丝数
     */
    int countFollowers(MYSQL* conn, int64_t userId);

    /**
     * @brief 查询用户关注的人列表（我关注的人）
     * @param conn MySQL连接
     * @param userId 用户ID（物理ID）
     * @param limit 每页数量
     * @param offset 偏移量
     * @return 关注列表（包含用户基本信息）
     */
    std::vector<Follow> findFollowingByUserId(MYSQL* conn, int64_t userId, int limit, int offset);

    /**
     * @brief 查询用户的粉丝列表（关注我的人）
     * @param conn MySQL连接
     * @param userId 用户ID（物理ID）
     * @param limit 每页数量
     * @param offset 偏移量
     * @return 粉丝列表（包含用户基本信息）
     */
    std::vector<Follow> findFollowersByUserId(MYSQL* conn, int64_t userId, int limit, int offset);

    /**
     * @brief 批量检查关注关系
     * @param conn MySQL连接
     * @param followerId 关注者ID（物理ID）
     * @param followeeIds 被关注者ID列表（物理ID）
     * @return Map<followeeId, isFollowing>
     */
    std::map<int64_t, bool> batchCheckExists(MYSQL* conn, int64_t followerId,
                                              const std::vector<int64_t>& followeeIds);

    /**
     * @brief 查询互关用户ID列表
     *
     * 查找同时满足以下条件的用户：
     * - 我关注了对方（A follows B）
     * - 对方也关注了我（B follows A）
     *
     * @param conn MySQL连接
     * @param userId 用户ID（物理ID）
     * @param limit 每页数量
     * @param offset 偏移量
     * @return 互关用户的物理ID列表
     *
     * @note SQL策略：使用INNER JOIN查询双向关注关系
     * @note 性能：利用idx_follower_create和idx_followee_create索引
     */
    std::vector<int64_t> findMutualFollowIds(MYSQL* conn, int64_t userId, int limit, int offset);

    /**
     * @brief 统计互关用户数量
     * @param conn MySQL连接
     * @param userId 用户ID（物理ID）
     * @return 互关用户数量
     */
    int countMutualFollows(MYSQL* conn, int64_t userId);

private:
    /**
     * @brief 从SQL结果构建Follow对象
     * @param stmt MySQL预编译语句
     * @return Follow对象
     */
    Follow buildFollowFromStatement(MYSQL_STMT* stmt);
};



