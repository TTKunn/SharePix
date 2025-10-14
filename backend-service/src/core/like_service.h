/**
 * @file like_service.h
 * @brief 点赞服务 - 业务逻辑层
 * @author Knot Team
 * @date 2025-10-12
 */

#pragma once

#include <string>
#include <memory>

// 前向声明
class LikeRepository;
class PostRepository;

/**
 * @brief 点赞结果结构体
 */
struct LikeResult {
    bool success;          // 操作是否成功
    int statusCode;        // HTTP状态码
    std::string message;   // 消息
    int likeCount;         // 当前点赞数
    bool hasLiked;         // 用户是否已点赞

    LikeResult()
        : success(false), statusCode(500), message(""), likeCount(0), hasLiked(false) {}
};

/**
 * @brief 点赞状态查询结果结构体
 */
struct LikeStatusResult {
    bool success;          // 查询是否成功
    int statusCode;        // HTTP状态码
    std::string message;   // 消息
    bool hasLiked;         // 用户是否已点赞
    int likeCount;         // 当前点赞数

    LikeStatusResult()
        : success(false), statusCode(500), message(""), hasLiked(false), likeCount(0) {}
};

/**
 * @brief 点赞服务类
 *
 * 负责点赞相关的业务逻辑：
 * - 点赞帖子
 * - 取消点赞
 * - 查询点赞状态
 */
class LikeService {
public:
    /**
     * @brief 构造函数
     */
    LikeService();

    /**
     * @brief 析构函数
     */
    ~LikeService();

    /**
     * @brief 点赞帖子
     *
     * 业务流程：
     * 1. 查询帖子是否存在
     * 2. 检查用户是否已点赞（幂等性）
     * 3. 开启事务
     * 4. 创建点赞记录
     * 5. 增加帖子点赞数
     * 6. 提交事务
     *
     * @param userId 用户ID（物理ID）
     * @param postId 帖子业务ID（POST_XXXXXX）
     * @return LikeResult 点赞结果
     */
    LikeResult likePost(int userId, const std::string& postId);

    /**
     * @brief 取消点赞
     *
     * 业务流程：
     * 1. 查询帖子是否存在
     * 2. 检查用户是否已点赞
     * 3. 开启事务
     * 4. 删除点赞记录
     * 5. 减少帖子点赞数
     * 6. 提交事务
     *
     * @param userId 用户ID（物理ID）
     * @param postId 帖子业务ID（POST_XXXXXX）
     * @return LikeResult 取消点赞结果
     */
    LikeResult unlikePost(int userId, const std::string& postId);

    /**
     * @brief 检查用户是否点赞过某帖子
     *
     * @param userId 用户ID（物理ID）
     * @param postId 帖子ID（物理ID）
     * @return true 已点赞，false 未点赞
     */
    bool hasLiked(int userId, int postId);

    /**
     * @brief 获取用户对帖子的点赞状态
     *
     * @param userId 用户ID（物理ID）
     * @param postId 帖子业务ID（POST_XXXXXX）
     * @return LikeStatusResult 点赞状态
     */
    LikeStatusResult getLikeStatus(int userId, const std::string& postId);

private:
    std::unique_ptr<LikeRepository> likeRepo_;
    std::unique_ptr<PostRepository> postRepo_;
};
