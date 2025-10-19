/**
 * @file comment_repository.h
 * @brief 评论数据访问层定义
 * @author Knot Team
 * @date 2025-10-19
 */

#pragma once

#include "models/comment.h"
#include <mysql/mysql.h>
#include <optional>
#include <vector>

/**
 * @brief 评论数据访问类
 *
 * 负责评论数据的CRUD操作
 */
class CommentRepository {
public:
    /**
     * @brief 构造函数
     */
    CommentRepository() = default;

    /**
     * @brief 析构函数
     */
    ~CommentRepository() = default;

    /**
     * @brief 创建评论记录
     * @param conn MySQL连接
     * @param comment 评论对象
     * @return 成功返回true，失败返回false
     */
    bool create(MYSQL* conn, const Comment& comment);

    /**
     * @brief 通过业务ID查询评论
     * @param conn MySQL连接
     * @param commentId 业务逻辑ID
     * @return 成功返回Comment对象，失败返回nullopt
     */
    std::optional<Comment> findByCommentId(MYSQL* conn, const std::string& commentId);

    /**
     * @brief 查询帖子的评论列表（分页，按时间倒序）
     * @param conn MySQL连接
     * @param postId 帖子ID（物理ID）
     * @param limit 限制数量
     * @param offset 偏移量
     * @return 评论列表
     */
    std::vector<Comment> findByPostId(MYSQL* conn, int postId, int limit = 20, int offset = 0);

    /**
     * @brief 统计帖子的评论数
     * @param conn MySQL连接
     * @param postId 帖子ID（物理ID）
     * @return 评论总数
     */
    int countByPostId(MYSQL* conn, int postId);

    /**
     * @brief 删除评论（通过业务ID）
     * @param conn MySQL连接
     * @param commentId 业务逻辑ID
     * @return 成功返回true，失败返回false
     */
    bool deleteByCommentId(MYSQL* conn, const std::string& commentId);

    /**
     * @brief 检查评论是否存在
     * @param conn MySQL连接
     * @param commentId 业务逻辑ID
     * @return 存在返回true，否则返回false
     */
    bool existsByCommentId(MYSQL* conn, const std::string& commentId);

    /**
     * @brief 检查评论是否属于某用户
     * @param conn MySQL连接
     * @param commentId 业务逻辑ID
     * @param userId 用户ID
     * @return 是评论作者返回true，否则返回false
     */
    bool isCommentOwner(MYSQL* conn, const std::string& commentId, int userId);

    /**
     * @brief 获取用户的评论列表
     * @param conn MySQL连接
     * @param userId 用户ID
     * @param limit 限制数量
     * @param offset 偏移量
     * @return 评论列表
     */
    std::vector<Comment> findByUserId(MYSQL* conn, int userId, int limit = 20, int offset = 0);

private:
    /**
     * @brief 从预编译语句构建Comment对象
     * @param stmt MYSQL_STMT指针
     * @return Comment对象
     */
    Comment buildCommentFromStatement(MYSQL_STMT* stmt);
};
