/**
 * @file favorite_repository.cpp
 * @brief 收藏数据访问层实现
 * @author Knot Team
 * @date 2025-10-10
 */

#include "database/favorite_repository.h"
#include "database/mysql_statement.h"
#include "utils/logger.h"
#include <cstring>
#include <stdexcept>

// 创建收藏记录
bool FavoriteRepository::create(MYSQL* conn, int userId, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // SQL 插入语句
        const char* query = "INSERT INTO favorites (user_id, post_id) VALUES (?, ?)";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        // user_id
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &userId;

        // post_id
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            // 检查是否是唯一约束冲突（ER_DUP_ENTRY = 1062）
            unsigned int err_no = mysql_stmt_errno(stmt.get());
            if (err_no == 1062) {
                Logger::warning("User already favorited this post (user_id=" + std::to_string(userId) +
                               ", post_id=" + std::to_string(postId) + ")");
            } else {
                Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            }
            return false;
        }

        Logger::info("Favorite created (user_id=" + std::to_string(userId) +
                    ", post_id=" + std::to_string(postId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in FavoriteRepository::create: " + std::string(e.what()));
        return false;
    }
}

// 删除收藏记录（取消收藏）
bool FavoriteRepository::deleteByUserAndPost(MYSQL* conn, int userId, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // SQL 删除语句
        const char* query = "DELETE FROM favorites WHERE user_id = ? AND post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        // user_id
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &userId;

        // post_id
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 检查是否删除成功（affected_rows > 0）
        my_ulonglong affected_rows = mysql_stmt_affected_rows(stmt.get());
        if (affected_rows == 0) {
            Logger::warning("No favorite found to delete (user_id=" + std::to_string(userId) +
                           ", post_id=" + std::to_string(postId) + ")");
            return false;
        }

        Logger::info("Favorite deleted (user_id=" + std::to_string(userId) +
                    ", post_id=" + std::to_string(postId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in FavoriteRepository::deleteByUserAndPost: " + std::string(e.what()));
        return false;
    }
}

// 检查用户是否收藏过某帖子
bool FavoriteRepository::exists(MYSQL* conn, int userId, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // SQL 查询语句
        const char* query = "SELECT COUNT(*) FROM favorites WHERE user_id = ? AND post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        // user_id
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &userId;

        // post_id
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定结果
        MYSQL_BIND result[1];
        memset(result, 0, sizeof(result));

        long long count = 0;
        result[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result[0].buffer = &count;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            return count > 0;
        }

        return false;

    } catch (const std::exception& e) {
        Logger::error("Exception in FavoriteRepository::exists: " + std::string(e.what()));
        return false;
    }
}

// 获取帖子的收藏总数
int FavoriteRepository::countByPostId(MYSQL* conn, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        // SQL 查询语句
        const char* query = "SELECT COUNT(*) FROM favorites WHERE post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        // post_id
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定结果
        MYSQL_BIND result[1];
        memset(result, 0, sizeof(result));

        long long count = 0;
        result[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result[0].buffer = &count;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            return static_cast<int>(count);
        }

        return 0;

    } catch (const std::exception& e) {
        Logger::error("Exception in FavoriteRepository::countByPostId: " + std::string(e.what()));
        return 0;
    }
}

// 获取用户的收藏列表
std::vector<Favorite> FavoriteRepository::findByUserId(MYSQL* conn, int userId, int limit, int offset) {
    std::vector<Favorite> favorites;

    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return favorites;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return favorites;
        }

        // SQL 查询语句
        const char* query = "SELECT id, user_id, post_id, UNIX_TIMESTAMP(create_time) as create_time "
                           "FROM favorites WHERE user_id = ? ORDER BY create_time DESC LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return favorites;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        // user_id
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &userId;

        // limit
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &limit;

        // offset
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return favorites;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return favorites;
        }

        // 绑定结果
        MYSQL_BIND result[4];
        memset(result, 0, sizeof(result));

        long long id, user_id, post_id, create_time;

        result[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result[0].buffer = &id;

        result[1].buffer_type = MYSQL_TYPE_LONGLONG;
        result[1].buffer = &user_id;

        result[2].buffer_type = MYSQL_TYPE_LONGLONG;
        result[2].buffer = &post_id;

        result[3].buffer_type = MYSQL_TYPE_LONGLONG;
        result[3].buffer = &create_time;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return favorites;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return favorites;
        }

        // 获取所有结果
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Favorite favorite;
            favorite.setId(static_cast<int>(id));
            favorite.setUserId(static_cast<int>(user_id));
            favorite.setPostId(static_cast<int>(post_id));
            favorite.setCreateTime(static_cast<std::time_t>(create_time));
            favorites.push_back(favorite);
        }

        Logger::info("Found " + std::to_string(favorites.size()) + " favorites for user " + std::to_string(userId));
        return favorites;

    } catch (const std::exception& e) {
        Logger::error("Exception in FavoriteRepository::findByUserId: " + std::string(e.what()));
        return favorites;
    }
}
