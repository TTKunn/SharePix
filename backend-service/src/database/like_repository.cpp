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
#include <sstream>
#include <chrono>

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

// 批量检查用户对多个帖子的点赞状态
std::unordered_map<int, bool> LikeRepository::batchExistsForPosts(
    MYSQL* conn,
    int userId,
    const std::vector<int>& postIds
) {
    std::unordered_map<int, bool> result;
    
    try {
        // ========================================
        // 第1步：初始化所有帖子为"未点赞"
        // ========================================
        for (int postId : postIds) {
            result[postId] = false;
        }
        
        // 空列表检查
        if (postIds.empty()) {
            Logger::info("batchExistsForPosts: 帖子ID列表为空");
            return result;
        }
        
        if (!conn) {
            Logger::error("batchExistsForPosts: 数据库连接为空");
            return result;
        }
        
        Logger::info("batchExistsForPosts: 批量查询用户 " + std::to_string(userId) + 
                    " 对 " + std::to_string(postIds.size()) + " 个帖子的点赞状态");
        
        // ========================================
        // 第2步：构建SQL
        // SELECT post_id FROM likes 
        // WHERE user_id = ? AND post_id IN (?, ?, ...)
        // ========================================
        std::ostringstream sqlBuilder;
        sqlBuilder << "SELECT post_id FROM likes WHERE user_id = ? AND post_id IN (";
        
        for (size_t i = 0; i < postIds.size(); i++) {
            if (i > 0) sqlBuilder << ", ";
            sqlBuilder << "?";
        }
        sqlBuilder << ")";
        
        std::string sql = sqlBuilder.str();
        Logger::debug("batchExistsForPosts SQL: " + sql);
        
        // ========================================
        // 第3步：准备预编译语句
        // ========================================
        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            Logger::error("batchExistsForPosts: 创建语句失败");
            return result;
        }
        
        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            Logger::error("batchExistsForPosts: 预编译失败: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }
        
        // ========================================
        // 第4步：绑定参数
        // 参数数量 = 1 (user_id) + N (post_ids)
        // ========================================
        size_t paramCount = 1 + postIds.size();
        std::vector<MYSQL_BIND> binds(paramCount);
        std::memset(binds.data(), 0, paramCount * sizeof(MYSQL_BIND));
        
        // 绑定user_id
        binds[0].buffer_type = MYSQL_TYPE_LONG;
        binds[0].buffer = const_cast<int*>(&userId);
        binds[0].is_null = 0;
        
        // 绑定所有post_id
        for (size_t i = 0; i < postIds.size(); i++) {
            binds[i + 1].buffer_type = MYSQL_TYPE_LONG;
            binds[i + 1].buffer = const_cast<int*>(&postIds[i]);
            binds[i + 1].is_null = 0;
        }
        
        if (mysql_stmt_bind_param(stmt.get(), binds.data()) != 0) {
            Logger::error("batchExistsForPosts: 绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }
        
        // ========================================
        // 第5步：执行查询
        // ========================================
        auto startTime = std::chrono::steady_clock::now();
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("batchExistsForPosts: 执行查询失败: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }
        
        // ========================================
        // 第6步：绑定结果列
        // ========================================
        int likedPostId;
        MYSQL_BIND resultBind;
        std::memset(&resultBind, 0, sizeof(MYSQL_BIND));
        resultBind.buffer_type = MYSQL_TYPE_LONG;
        resultBind.buffer = &likedPostId;
        resultBind.is_null = 0;
        
        if (mysql_stmt_bind_result(stmt.get(), &resultBind) != 0) {
            Logger::error("batchExistsForPosts: 绑定结果失败: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }
        
        // ========================================
        // 第7步：读取结果集
        // ========================================
        int likedCount = 0;
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            result[likedPostId] = true;  // 标记为已点赞
            likedCount++;
        }
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        ).count();
        
        Logger::info("batchExistsForPosts: 批量查询完成，" + std::to_string(likedCount) + "/" + 
                    std::to_string(postIds.size()) + " 个帖子已点赞，耗时: " + 
                    std::to_string(duration) + "ms");
        
        return result;
        
    } catch (const std::exception& e) {
        Logger::error("batchExistsForPosts异常: " + std::string(e.what()));
        return result;
    }
}
