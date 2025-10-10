/**
 * @file favorite_repository.h
 * @brief 收藏数据访问层定义
 * @author Knot Team
 * @date 2025-10-10
 */

#pragma once

#include "models/favorite.h"
#include <mysql/mysql.h>
#include <optional>
#include <vector>

/**
 * @brief 收藏数据访问类
 *
 * 负责收藏数据的CRUD操作
 */
class FavoriteRepository {
public:
    /**
     * @brief 构造函数
     */
    FavoriteRepository() = default;

    /**
     * @brief 析构函数
     */
    ~FavoriteRepository() = default;

    /**
     * @brief 创建收藏记录
     * @param conn MySQL连接
     * @param userId 收藏用户ID
     * @param postId 被收藏帖子ID（物理ID）
     * @return 成功返回true，失败返回false
     */
    bool create(MYSQL* conn, int userId, int postId);

    /**
     * @brief 删除收藏记录（取消收藏）
     * @param conn MySQL连接
     * @param userId 收藏用户ID
     * @param postId 被收藏帖子ID（物理ID）
     * @return 成功返回true，失败返回false
     */
    bool deleteByUserAndPost(MYSQL* conn, int userId, int postId);

    /**
     * @brief 检查用户是否收藏过某帖子
     * @param conn MySQL连接
     * @param userId 用户ID
     * @param postId 帖子ID（物理ID）
     * @return 已收藏返回true，否则返回false
     */
    bool exists(MYSQL* conn, int userId, int postId);

    /**
     * @brief 获取帖子的收藏总数
     * @param conn MySQL连接
     * @param postId 帖子ID（物理ID）
     * @return 收藏总数
     */
    int countByPostId(MYSQL* conn, int postId);

    /**
     * @brief 获取用户的收藏列表
     * @param conn MySQL连接
     * @param userId 用户ID
     * @param limit 限制数量
     * @param offset 偏移量
     * @return 收藏列表
     */
    std::vector<Favorite> findByUserId(MYSQL* conn, int userId, int limit = 20, int offset = 0);

private:
    /**
     * @brief 从预编译语句构建Favorite对象
     * @param stmt MYSQL_STMT指针
     * @return Favorite对象
     */
    Favorite buildFavoriteFromStatement(MYSQL_STMT* stmt);
};
