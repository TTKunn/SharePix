/**
 * @file favorite_service.h
 * @brief 收藏服务 - 业务逻辑层
 * @author Knot Team
 * @date 2025-10-12
 */

#pragma once

#include <string>
#include <memory>
#include <vector>
#include "models/post.h"

// 前向声明
class FavoriteRepository;
class PostRepository;

/**
 * @brief 收藏结果结构体
 */
struct FavoriteResult {
    bool success;          // 操作是否成功
    int statusCode;        // HTTP状态码
    std::string message;   // 消息
    int favoriteCount;     // 当前收藏数
    bool hasFavorited;     // 用户是否已收藏

    FavoriteResult()
        : success(false), statusCode(500), message(""), favoriteCount(0), hasFavorited(false) {}
};

/**
 * @brief 收藏状态查询结果结构体
 */
struct FavoriteStatusResult {
    bool success;          // 查询是否成功
    int statusCode;        // HTTP状态码
    std::string message;   // 消息
    bool hasFavorited;     // 用户是否已收藏
    int favoriteCount;     // 当前收藏数

    FavoriteStatusResult()
        : success(false), statusCode(500), message(""), hasFavorited(false), favoriteCount(0) {}
};

/**
 * @brief 用户收藏列表查询结果结构体
 */
struct FavoriteListResult {
    bool success;          // 查询是否成功
    int statusCode;        // HTTP状态码
    std::string message;   // 消息
    std::vector<Post> posts;  // 收藏的帖子列表
    int total;             // 总数

    FavoriteListResult()
        : success(false), statusCode(500), message(""), total(0) {}
};

/**
 * @brief 收藏服务类
 *
 * 负责收藏相关的业务逻辑：
 * - 收藏帖子
 * - 取消收藏
 * - 查询收藏状态
 */
class FavoriteService {
public:
    /**
     * @brief 构造函数
     */
    FavoriteService();

    /**
     * @brief 析构函数
     */
    ~FavoriteService();

    /**
     * @brief 收藏帖子
     *
     * 业务流程：
     * 1. 查询帖子是否存在
     * 2. 检查用户是否已收藏（幂等性）
     * 3. 开启事务
     * 4. 创建收藏记录
     * 5. 增加帖子收藏数
     * 6. 提交事务
     *
     * @param userId 用户ID（物理ID）
     * @param postId 帖子业务ID（POST_XXXXXX）
     * @return FavoriteResult 收藏结果
     */
    FavoriteResult favoritePost(int userId, const std::string& postId);

    /**
     * @brief 取消收藏
     *
     * 业务流程：
     * 1. 查询帖子是否存在
     * 2. 检查用户是否已收藏
     * 3. 开启事务
     * 4. 删除收藏记录
     * 5. 减少帖子收藏数
     * 6. 提交事务
     *
     * @param userId 用户ID（物理ID）
     * @param postId 帖子业务ID（POST_XXXXXX）
     * @return FavoriteResult 取消收藏结果
     */
    FavoriteResult unfavoritePost(int userId, const std::string& postId);

    /**
     * @brief 检查用户是否收藏过某帖子
     *
     * @param userId 用户ID（物理ID）
     * @param postId 帖子ID（物理ID）
     * @return true 已收藏，false 未收藏
     */
    bool hasFavorited(int userId, int postId);

    /**
     * @brief 获取用户对帖子的收藏状态
     *
     * @param userId 用户ID（物理ID）
     * @param postId 帖子业务ID（POST_XXXXXX）
     * @return FavoriteStatusResult 收藏状态
     */
    FavoriteStatusResult getFavoriteStatus(int userId, const std::string& postId);

    /**
     * @brief 获取用户收藏列表
     *
     * @param userId 用户ID（物理ID）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return FavoriteListResult 收藏列表
     */
    FavoriteListResult getUserFavorites(int userId, int page, int pageSize);

private:
    std::unique_ptr<FavoriteRepository> favoriteRepo_;
    std::unique_ptr<PostRepository> postRepo_;
};
