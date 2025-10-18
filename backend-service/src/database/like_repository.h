/**
 * @file like_repository.h
 * @brief 点赞数据访问层定义
 * @author Knot Team
 * @date 2025-10-10
 */

#pragma once

#include "models/like.h"
#include <mysql/mysql.h>
#include <optional>
#include <vector>
#include <unordered_map>

/**
 * @brief 点赞数据访问类
 *
 * 负责点赞数据的CRUD操作
 */
class LikeRepository {
public:
    /**
     * @brief 构造函数
     */
    LikeRepository() = default;

    /**
     * @brief 析构函数
     */
    ~LikeRepository() = default;

    /**
     * @brief 创建点赞记录
     * @param conn MySQL连接
     * @param userId 点赞用户ID
     * @param postId 被点赞帖子ID（物理ID）
     * @return 成功返回true，失败返回false
     */
    bool create(MYSQL* conn, int userId, int postId);

    /**
     * @brief 删除点赞记录（取消点赞）
     * @param conn MySQL连接
     * @param userId 点赞用户ID
     * @param postId 被点赞帖子ID（物理ID）
     * @return 成功返回true，失败返回false
     */
    bool deleteByUserAndPost(MYSQL* conn, int userId, int postId);

    /**
     * @brief 检查用户是否点赞过某帖子
     * @param conn MySQL连接
     * @param userId 用户ID
     * @param postId 帖子ID（物理ID）
     * @return 已点赞返回true，否则返回false
     */
    bool exists(MYSQL* conn, int userId, int postId);

    /**
     * @brief 获取帖子的点赞总数
     * @param conn MySQL连接
     * @param postId 帖子ID（物理ID）
     * @return 点赞总数
     */
    int countByPostId(MYSQL* conn, int postId);

    /**
     * @brief 获取用户的点赞列表
     * @param conn MySQL连接
     * @param userId 用户ID
     * @param limit 限制数量
     * @param offset 偏移量
     * @return 点赞列表
     */
    std::vector<Like> findByUserId(MYSQL* conn, int userId, int limit = 20, int offset = 0);

    /**
     * @brief 批量检查用户对多个帖子的点赞状态
     * @param conn MySQL连接
     * @param userId 用户ID（物理ID）
     * @param postIds 帖子ID列表（物理ID）
     * @return 点赞状态映射表（key=postId, value=是否点赞）
     *
     * @example
     *   std::vector<int> postIds = {1, 2, 3};
     *   auto result = batchExistsForPosts(conn, 123, postIds);
     *   // result = {1: true, 2: false, 3: true}
     */
    std::unordered_map<int, bool> batchExistsForPosts(
        MYSQL* conn,
        int userId,
        const std::vector<int>& postIds
    );

private:
    /**
     * @brief 从预编译语句构建Like对象
     * @param stmt MYSQL_STMT指针
     * @return Like对象
     */
    Like buildLikeFromStatement(MYSQL_STMT* stmt);
};
