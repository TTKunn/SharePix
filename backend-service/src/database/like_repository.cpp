/**
 * @file like_repository.cpp
 * @brief 点赞数据访问层实现
 * @author Knot Team
 * @date 2025-10-10
 */

#include "database/like_repository.h"
#include "database/mysql_statement.h"
#include "utils/logger.h"
#include <cstring>
#include <stdexcept>

// 创建点赞记录
bool LikeRepository::create(MYSQL* conn, int userId, int postId) {
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
        const char* query = "INSERT INTO likes (user_id, post_id) VALUES (?, ?)";

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
                Logger::warning("User already liked this post (user_id=" + std::to_string(userId) +
                               ", post_id=" + std::to_string(postId) + ")");
            } else {
                Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            }
            return false;
        }

        Logger::info("Like created (user_id=" + std::to_string(userId) +
                    ", post_id=" + std::to_string(postId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in LikeRepository::create: " + std::string(e.what()));
        return false;
    }
}

// 删除点赞记录（取消点赞）
bool LikeRepository::deleteByUserAndPost(MYSQL* conn, int userId, int postId) {
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
        const char* query = "DELETE FROM likes WHERE user_id = ? AND post_id = ?";

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
            Logger::warning("No like found to delete (user_id=" + std::to_string(userId) +
                           ", post_id=" + std::to_string(postId) + ")");
            return false;
        }

        Logger::info("Like deleted (user_id=" + std::to_string(userId) +
                    ", post_id=" + std::to_string(postId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in LikeRepository::deleteByUserAndPost: " + std::string(e.what()));
        return false;
    }
}

// 检查用户是否点赞过某帖子
bool LikeRepository::exists(MYSQL* conn, int userId, int postId) {
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
        const char* query = "SELECT COUNT(*) FROM likes WHERE user_id = ? AND post_id = ?";

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
        Logger::error("Exception in LikeRepository::exists: " + std::string(e.what()));
        return false;
    }
}

// 获取帖子的点赞总数
int LikeRepository::countByPostId(MYSQL* conn, int postId) {
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
        const char* query = "SELECT COUNT(*) FROM likes WHERE post_id = ?";

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
        Logger::error("Exception in LikeRepository::countByPostId: " + std::string(e.what()));
        return 0;
    }
}

// 获取用户的点赞列表
std::vector<Like> LikeRepository::findByUserId(MYSQL* conn, int userId, int limit, int offset) {
    std::vector<Like> likes;

    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return likes;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return likes;
        }

        // SQL 查询语句
        const char* query = "SELECT id, user_id, post_id, UNIX_TIMESTAMP(create_time) as create_time "
                           "FROM likes WHERE user_id = ? ORDER BY create_time DESC LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return likes;
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
            return likes;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return likes;
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
            return likes;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return likes;
        }

        // 获取所有结果
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Like like;
            like.setId(static_cast<int>(id));
            like.setUserId(static_cast<int>(user_id));
            like.setPostId(static_cast<int>(post_id));
            like.setCreateTime(static_cast<std::time_t>(create_time));
            likes.push_back(like);
        }

        Logger::info("Found " + std::to_string(likes.size()) + " likes for user " + std::to_string(userId));
        return likes;

    } catch (const std::exception& e) {
        Logger::error("Exception in LikeRepository::findByUserId: " + std::string(e.what()));
        return likes;
    }
}
