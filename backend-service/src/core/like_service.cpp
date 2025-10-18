/**
 * @file like_service.cpp
 * @brief 点赞服务实现
 * @author Knot Team
 * @date 2025-10-12
 */

#include "core/like_service.h"
#include "database/like_repository.h"
#include "database/post_repository.h"
#include "database/connection_guard.h"
#include "database/connection_pool.h"
#include "utils/logger.h"

// 构造函数
LikeService::LikeService() {
    likeRepo_ = std::make_unique<LikeRepository>();
    postRepo_ = std::make_unique<PostRepository>();
    Logger::info("LikeService initialized");
}

// 析构函数
LikeService::~LikeService() {
    Logger::info("LikeService destroyed");
}

// 点赞帖子
LikeResult LikeService::likePost(int userId, const std::string& postId) {
    LikeResult result;

    try {
        Logger::info("User " + std::to_string(userId) + " liking post " + postId);

        // 1. 查询帖子
        auto post = postRepo_->findByPostId(postId);
        if (!post.has_value()) {
            result.statusCode = 404;
            result.message = "帖子不存在";
            return result;
        }

        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = connGuard.get();

        // 2. 检查是否已点赞（幂等性）
        if (likeRepo_->exists(conn, userId, post->getId())) {
            result.success = true;
            result.statusCode = 200;
            result.message = "已点赞";
            result.likeCount = post->getLikeCount();
            result.hasLiked = true;
            return result;
        }

        // 3. 开启事务
        if (mysql_query(conn, "START TRANSACTION") != 0) {
            Logger::error("Failed to start transaction: " + std::string(mysql_error(conn)));
            result.statusCode = 500;
            result.message = "事务开启失败";
            return result;
        }

        // 4. 创建点赞记录
        if (!likeRepo_->create(conn, userId, post->getId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "创建点赞记录失败";
            return result;
        }

        // 5. 增加帖子点赞数
        if (!postRepo_->incrementLikeCount(conn, post->getId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "更新点赞数失败";
            return result;
        }

        // 6. 提交事务
        if (mysql_query(conn, "COMMIT") != 0) {
            Logger::error("Failed to commit transaction: " + std::string(mysql_error(conn)));
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "事务提交失败";
            return result;
        }

        // 7. 查询最新点赞数
        auto updatedPost = postRepo_->findByPostId(postId);
        int newLikeCount = updatedPost.has_value() ? updatedPost->getLikeCount() : (post->getLikeCount() + 1);

        result.success = true;
        result.statusCode = 200;
        result.message = "点赞成功";
        result.likeCount = newLikeCount;
        result.hasLiked = true;

        Logger::info("Post liked successfully");
        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in likePost: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}

// 取消点赞
LikeResult LikeService::unlikePost(int userId, const std::string& postId) {
    LikeResult result;

    try {
        Logger::info("User " + std::to_string(userId) + " unliking post " + postId);

        // 1. 查询帖子
        auto post = postRepo_->findByPostId(postId);
        if (!post.has_value()) {
            result.statusCode = 404;
            result.message = "帖子不存在";
            return result;
        }

        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = connGuard.get();

        // 2. 检查是否已点赞
        if (!likeRepo_->exists(conn, userId, post->getId())) {
            result.success = true;
            result.statusCode = 200;
            result.message = "未点赞";
            result.likeCount = post->getLikeCount();
            result.hasLiked = false;
            return result;
        }

        // 3. 开启事务
        if (mysql_query(conn, "START TRANSACTION") != 0) {
            Logger::error("Failed to start transaction: " + std::string(mysql_error(conn)));
            result.statusCode = 500;
            result.message = "事务开启失败";
            return result;
        }

        // 4. 删除点赞记录
        if (!likeRepo_->deleteByUserAndPost(conn, userId, post->getId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "删除点赞记录失败";
            return result;
        }

        // 5. 减少帖子点赞数
        if (!postRepo_->decrementLikeCount(conn, post->getId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "更新点赞数失败";
            return result;
        }

        // 6. 提交事务
        if (mysql_query(conn, "COMMIT") != 0) {
            Logger::error("Failed to commit transaction: " + std::string(mysql_error(conn)));
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "事务提交失败";
            return result;
        }

        // 7. 查询最新点赞数
        auto updatedPost = postRepo_->findByPostId(postId);
        int newLikeCount = updatedPost.has_value() ? updatedPost->getLikeCount() : (post->getLikeCount() - 1);

        result.success = true;
        result.statusCode = 200;
        result.message = "取消点赞成功";
        result.likeCount = newLikeCount;
        result.hasLiked = false;

        Logger::info("Post unliked successfully");
        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in unlikePost: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}

// 检查用户是否点赞过某帖子
bool LikeService::hasLiked(int userId, int postId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            return false;
        }

        return likeRepo_->exists(connGuard.get(), userId, postId);

    } catch (const std::exception& e) {
        Logger::error("Exception in hasLiked: " + std::string(e.what()));
        return false;
    }
}

// 获取点赞状态
LikeStatusResult LikeService::getLikeStatus(int userId, const std::string& postId) {
    LikeStatusResult result;

    try {
        Logger::info("Getting like status for user " + std::to_string(userId) + " on post " + postId);

        // 1. 查询帖子
        auto post = postRepo_->findByPostId(postId);
        if (!post.has_value()) {
            result.statusCode = 404;
            result.message = "帖子不存在";
            return result;
        }

        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = connGuard.get();

        // 2. 查询用户是否已点赞
        bool liked = likeRepo_->exists(conn, userId, post->getId());

        // 3. 获取帖子总点赞数
        int likeCount = likeRepo_->countByPostId(conn, post->getId());

        // 4. 构建结果
        result.success = true;
        result.statusCode = 200;
        result.message = "查询成功";
        result.hasLiked = liked;
        result.likeCount = likeCount;

        Logger::info("Like status query successful: hasLiked=" + std::string(liked ? "true" : "false") + 
                     ", likeCount=" + std::to_string(likeCount));

        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in getLikeStatus: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}

// 批量检查用户对多个帖子的点赞状态
std::unordered_map<int, bool> LikeService::batchCheckLikedStatus(
    int userId,
    const std::vector<int>& postIds
) {
    std::unordered_map<int, bool> result;
    
    try {
        // 空列表检查
        if (postIds.empty()) {
            Logger::info("batchCheckLikedStatus: 帖子ID列表为空");
            return result;
        }
        
        Logger::info("batchCheckLikedStatus: 批量查询用户 " + std::to_string(userId) + 
                    " 对 " + std::to_string(postIds.size()) + " 个帖子的点赞状态");
        
        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("batchCheckLikedStatus: 获取数据库连接失败");
            return result;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 调用Repository批量查询
        result = likeRepo_->batchExistsForPosts(conn, userId, postIds);
        
        // 统计信息
        int likedCount = 0;
        for (const auto& pair : result) {
            if (pair.second) likedCount++;
        }
        
        Logger::info("batchCheckLikedStatus: 批量查询完成，" + std::to_string(likedCount) + 
                    "/" + std::to_string(postIds.size()) + " 个帖子已点赞");
        
        return result;
        
    } catch (const std::exception& e) {
        Logger::error("batchCheckLikedStatus异常: " + std::string(e.what()));
        return result;
    }
}
