/**
 * @file comment_service.h
 * @brief 评论服务 - 业务逻辑层
 * @author Knot Team
 * @date 2025-10-19
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include "models/comment.h"

// 前向声明
class CommentRepository;
class PostRepository;
class UserRepository;

/**
 * @brief 评论结果结构体
 */
struct CommentResult {
    bool success;                    // 操作是否成功
    int statusCode;                  // HTTP状态码
    std::string message;             // 消息
    std::optional<Comment> comment;  // 评论对象
    int commentCount;                // 当前评论数

    CommentResult()
        : success(false), statusCode(500), message(""), comment(std::nullopt), commentCount(0) {}
};

/**
 * @brief 评论列表结果结构体
 */
struct CommentListResult {
    bool success;                    // 查询是否成功
    int statusCode;                  // HTTP状态码
    std::string message;             // 消息
    std::vector<Comment> comments;   // 评论列表
    int total;                       // 总评论数
    bool hasMore;                    // 是否有更多评论

    CommentListResult()
        : success(false), statusCode(500), message(""), comments(), total(0), hasMore(false) {}
};

/**
 * @brief 评论服务类
 *
 * 负责评论相关的业务逻辑：
 * - 创建评论
 * - 查询评论列表
 * - 删除评论
 */
class CommentService {
public:
    /**
     * @brief 构造函数
     */
    CommentService();

    /**
     * @brief 析构函数
     */
    ~CommentService();

    /**
     * @brief 创建评论
     *
     * 业务流程：
     * 1. 验证帖子是否存在
     * 2. 验证评论内容（1-1000字符）
     * 3. 生成业务ID（CMT_2025Q4_ABC123）
     * 4. 开启数据库事务
     * 5. 创建评论记录
     * 6. 增加帖子评论数（+1）
     * 7. 提交事务
     * 8. 返回评论对象（包含当前评论数）
     *
     * @param userId 用户ID（物理ID）
     * @param postId 帖子业务ID（POST_XXXXXX）
     * @param content 评论内容
     * @return CommentResult 评论结果
     */
    CommentResult createComment(int userId, const std::string& postId, const std::string& content);

    /**
     * @brief 获取帖子的评论列表（分页，按时间倒序）
     *
     * 业务流程：
     * 1. 验证帖子是否存在
     * 2. 查询评论列表（分页）
     * 3. 统计总评论数
     * 4. 返回评论列表
     *
     * @param postId 帖子业务ID（POST_XXXXXX）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量（默认20，最大100）
     * @return CommentListResult 评论列表结果
     */
    CommentListResult getCommentsByPost(const std::string& postId, int page = 1, int pageSize = 20);

    /**
     * @brief 删除评论
     *
     * 业务流程：
     * 1. 查询评论是否存在
     * 2. 验证权限：
     *    - 评论作者可以删除自己的评论
     *    - 帖子作者可以删除帖子下的任意评论
     * 3. 开启数据库事务
     * 4. 删除评论记录
     * 5. 减少帖子评论数（-1）
     * 6. 提交事务
     *
     * @param userId 用户ID（物理ID）
     * @param commentId 评论业务ID（CMT_XXXXXX）
     * @return CommentResult 删除结果
     */
    CommentResult deleteComment(int userId, const std::string& commentId);

    /**
     * @brief 获取用户的评论列表
     *
     * @param userId 用户ID（物理ID）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量（默认20，最大100）
     * @return CommentListResult 评论列表结果
     */
    CommentListResult getCommentsByUser(int userId, int page = 1, int pageSize = 20);

private:
    std::unique_ptr<CommentRepository> commentRepo_;
    std::unique_ptr<PostRepository> postRepo_;
    std::unique_ptr<UserRepository> userRepo_;

    /**
     * @brief 生成评论业务ID
     * @return 评论业务ID（例：CMT_2025Q4_ABC123）
     */
    std::string generateCommentId();

    /**
     * @brief 验证评论内容
     * @param content 评论内容
     * @return 错误信息，空字符串表示验证通过
     */
    std::string validateContent(const std::string& content);
};
