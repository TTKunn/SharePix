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
#include <sstream>
#include <chrono>

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

// 获取用户收藏的帖子列表（带分页）
std::vector<Post> FavoriteRepository::getUserFavorites(MYSQL* conn, int userId, int page, int pageSize) {
    std::vector<Post> posts;

    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return posts;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return posts;
        }

        // 计算偏移量
        int offset = (page - 1) * pageSize;

        // SQL查询：联表查询收藏的帖子
        const char* query = R"(
            SELECT p.id, p.post_id, p.user_id, p.title, p.description, 
                   p.image_count, p.like_count, p.favorite_count, p.view_count, 
                   p.create_time, p.update_time 
            FROM posts p
            INNER JOIN favorites f ON p.id = f.post_id
            WHERE f.user_id = ?
            ORDER BY f.create_time DESC
            LIMIT ? OFFSET ?
        )";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &userId;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &pageSize;

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        // 绑定结果
        MYSQL_BIND result[11];
        memset(result, 0, sizeof(result));

        long long id, user_id, image_count, like_count, favorite_count, view_count, create_time, update_time;
        char post_id[64], title[512], description[2048];
        unsigned long post_id_len, title_len, description_len;
        bool post_id_null, title_null, description_null;

        result[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result[0].buffer = &id;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = post_id;
        result[1].buffer_length = sizeof(post_id);
        result[1].length = &post_id_len;
        result[1].is_null = &post_id_null;

        result[2].buffer_type = MYSQL_TYPE_LONGLONG;
        result[2].buffer = &user_id;

        result[3].buffer_type = MYSQL_TYPE_STRING;
        result[3].buffer = title;
        result[3].buffer_length = sizeof(title);
        result[3].length = &title_len;
        result[3].is_null = &title_null;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = description;
        result[4].buffer_length = sizeof(description);
        result[4].length = &description_len;
        result[4].is_null = &description_null;

        result[5].buffer_type = MYSQL_TYPE_LONGLONG;
        result[5].buffer = &image_count;

        result[6].buffer_type = MYSQL_TYPE_LONGLONG;
        result[6].buffer = &like_count;

        result[7].buffer_type = MYSQL_TYPE_LONGLONG;
        result[7].buffer = &favorite_count;

        result[8].buffer_type = MYSQL_TYPE_LONGLONG;
        result[8].buffer = &view_count;

        result[9].buffer_type = MYSQL_TYPE_LONGLONG;
        result[9].buffer = &create_time;

        result[10].buffer_type = MYSQL_TYPE_LONGLONG;
        result[10].buffer = &update_time;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        // 获取所有结果
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Post post;
            post.setId(static_cast<int>(id));
            post.setPostId(std::string(post_id, post_id_len));
            post.setUserId(static_cast<int>(user_id));
            if (!title_null) post.setTitle(std::string(title, title_len));
            if (!description_null) post.setDescription(std::string(description, description_len));
            post.setImageCount(static_cast<int>(image_count));
            post.setLikeCount(static_cast<int>(like_count));
            post.setFavoriteCount(static_cast<int>(favorite_count));
            post.setViewCount(static_cast<int>(view_count));
            post.setCreateTime(static_cast<std::time_t>(create_time));
            post.setUpdateTime(static_cast<std::time_t>(update_time));
            posts.push_back(post);
        }

        Logger::info("Found " + std::to_string(posts.size()) + " favorited posts for user " + std::to_string(userId));
        return posts;

    } catch (const std::exception& e) {
        Logger::error("Exception in FavoriteRepository::getUserFavorites: " + std::string(e.what()));
        return posts;
    }
}

// 获取用户收藏的帖子总数
int FavoriteRepository::getUserFavoriteCount(MYSQL* conn, int userId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        const char* query = "SELECT COUNT(*) FROM favorites WHERE user_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &userId;

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
        Logger::error("Exception in FavoriteRepository::getUserFavoriteCount: " + std::string(e.what()));
        return 0;
    }
}

// 批量检查用户对多个帖子的收藏状态
std::unordered_map<int, bool> FavoriteRepository::batchExistsForPosts(
    MYSQL* conn,
    int userId,
    const std::vector<int>& postIds
) {
    std::unordered_map<int, bool> result;
    
    try {
        // ========================================
        // 第1步：初始化所有帖子为"未收藏"
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
                    " 对 " + std::to_string(postIds.size()) + " 个帖子的收藏状态");
        
        // ========================================
        // 第2步：构建SQL
        // SELECT post_id FROM favorites 
        // WHERE user_id = ? AND post_id IN (?, ?, ...)
        // ========================================
        std::ostringstream sqlBuilder;
        sqlBuilder << "SELECT post_id FROM favorites WHERE user_id = ? AND post_id IN (";
        
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
        int favoritedPostId;
        MYSQL_BIND resultBind;
        std::memset(&resultBind, 0, sizeof(MYSQL_BIND));
        resultBind.buffer_type = MYSQL_TYPE_LONG;
        resultBind.buffer = &favoritedPostId;
        resultBind.is_null = 0;
        
        if (mysql_stmt_bind_result(stmt.get(), &resultBind) != 0) {
            Logger::error("batchExistsForPosts: 绑定结果失败: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }
        
        // ========================================
        // 第7步：读取结果集
        // ========================================
        int favoritedCount = 0;
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            result[favoritedPostId] = true;  // 标记为已收藏
            favoritedCount++;
        }
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        ).count();
        
        Logger::info("batchExistsForPosts: 批量查询完成，" + std::to_string(favoritedCount) + "/" + 
                    std::to_string(postIds.size()) + " 个帖子已收藏，耗时: " + 
                    std::to_string(duration) + "ms");
        
        return result;
        
    } catch (const std::exception& e) {
        Logger::error("batchExistsForPosts异常: " + std::string(e.what()));
        return result;
    }
}
