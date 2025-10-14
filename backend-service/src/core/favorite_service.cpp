/**
 * @file favorite_service.cpp
 * @brief 收藏服务实现
 * @author Knot Team
 * @date 2025-10-12
 */

#include "core/favorite_service.h"
#include "database/favorite_repository.h"
#include "database/post_repository.h"
#include "database/connection_guard.h"
#include "database/connection_pool.h"
#include "utils/logger.h"

// 构造函数
FavoriteService::FavoriteService() {
    favoriteRepo_ = std::make_unique<FavoriteRepository>();
    postRepo_ = std::make_unique<PostRepository>();
    Logger::info("FavoriteService initialized");
}

// 析构函数
FavoriteService::~FavoriteService() {
    Logger::info("FavoriteService destroyed");
}

// 收藏帖子
FavoriteResult FavoriteService::favoritePost(int userId, const std::string& postId) {
    FavoriteResult result;

    try {
        Logger::info("User " + std::to_string(userId) + " favoriting post " + postId);

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

        // 2. 检查是否已收藏（幂等性）
        if (favoriteRepo_->exists(conn, userId, post->getId())) {
            result.success = true;
            result.statusCode = 200;
            result.message = "已收藏";
            result.favoriteCount = post->getFavoriteCount();
            result.hasFavorited = true;
            return result;
        }

        // 3. 开启事务
        if (mysql_query(conn, "START TRANSACTION") != 0) {
            Logger::error("Failed to start transaction: " + std::string(mysql_error(conn)));
            result.statusCode = 500;
            result.message = "事务开启失败";
            return result;
        }

        // 4. 创建收藏记录
        if (!favoriteRepo_->create(conn, userId, post->getId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "创建收藏记录失败";
            return result;
        }

        // 5. 增加帖子收藏数
        if (!postRepo_->incrementFavoriteCount(conn, post->getId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "更新收藏数失败";
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

        // 7. 查询最新收藏数
        auto updatedPost = postRepo_->findByPostId(postId);
        int newFavoriteCount = updatedPost.has_value() ? updatedPost->getFavoriteCount() : (post->getFavoriteCount() + 1);

        result.success = true;
        result.statusCode = 200;
        result.message = "收藏成功";
        result.favoriteCount = newFavoriteCount;
        result.hasFavorited = true;

        Logger::info("Post favorited successfully");
        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in favoritePost: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}

// 取消收藏
FavoriteResult FavoriteService::unfavoritePost(int userId, const std::string& postId) {
    FavoriteResult result;

    try {
        Logger::info("User " + std::to_string(userId) + " unfavoriting post " + postId);

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

        // 2. 检查是否已收藏
        if (!favoriteRepo_->exists(conn, userId, post->getId())) {
            result.success = true;
            result.statusCode = 200;
            result.message = "未收藏";
            result.favoriteCount = post->getFavoriteCount();
            result.hasFavorited = false;
            return result;
        }

        // 3. 开启事务
        if (mysql_query(conn, "START TRANSACTION") != 0) {
            Logger::error("Failed to start transaction: " + std::string(mysql_error(conn)));
            result.statusCode = 500;
            result.message = "事务开启失败";
            return result;
        }

        // 4. 删除收藏记录
        if (!favoriteRepo_->deleteByUserAndPost(conn, userId, post->getId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "删除收藏记录失败";
            return result;
        }

        // 5. 减少帖子收藏数
        if (!postRepo_->decrementFavoriteCount(conn, post->getId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "更新收藏数失败";
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

        // 7. 查询最新收藏数
        auto updatedPost = postRepo_->findByPostId(postId);
        int newFavoriteCount = updatedPost.has_value() ? updatedPost->getFavoriteCount() : (post->getFavoriteCount() - 1);

        result.success = true;
        result.statusCode = 200;
        result.message = "取消收藏成功";
        result.favoriteCount = newFavoriteCount;
        result.hasFavorited = false;

        Logger::info("Post unfavorited successfully");
        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in unfavoritePost: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}

// 检查用户是否收藏过某帖子
bool FavoriteService::hasFavorited(int userId, int postId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            return false;
        }

        return favoriteRepo_->exists(connGuard.get(), userId, postId);

    } catch (const std::exception& e) {
        Logger::error("Exception in hasFavorited: " + std::string(e.what()));
        return false;
    }
}

// 获取收藏状态
FavoriteStatusResult FavoriteService::getFavoriteStatus(int userId, const std::string& postId) {
    FavoriteStatusResult result;

    try {
        Logger::info("Getting favorite status for user " + std::to_string(userId) + " on post " + postId);

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

        // 2. 查询用户是否已收藏
        bool favorited = favoriteRepo_->exists(conn, userId, post->getId());

        // 3. 获取帖子总收藏数
        int favoriteCount = favoriteRepo_->countByPostId(conn, post->getId());

        // 4. 构建结果
        result.success = true;
        result.statusCode = 200;
        result.message = "查询成功";
        result.hasFavorited = favorited;
        result.favoriteCount = favoriteCount;

        Logger::info("Favorite status query successful: hasFavorited=" + std::string(favorited ? "true" : "false") + 
                     ", favoriteCount=" + std::to_string(favoriteCount));

        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in getFavoriteStatus: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}

// 获取用户收藏列表
FavoriteListResult FavoriteService::getUserFavorites(int userId, int page, int pageSize) {
    FavoriteListResult result;

    try {
        Logger::info("Getting favorites for user " + std::to_string(userId) + 
                     ", page=" + std::to_string(page) + ", pageSize=" + std::to_string(pageSize));

        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = connGuard.get();

        // 1. 获取用户收藏的帖子列表
        result.posts = favoriteRepo_->getUserFavorites(conn, userId, page, pageSize);

        // 2. 获取总数
        result.total = favoriteRepo_->getUserFavoriteCount(conn, userId);

        // 3. 构建结果
        result.success = true;
        result.statusCode = 200;
        result.message = "查询成功";

        Logger::info("User favorites query successful: total=" + std::to_string(result.total) + 
                     ", returned=" + std::to_string(result.posts.size()));

        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in getUserFavorites: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}
