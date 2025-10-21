/**
 * @file follow_repository.cpp
 * @brief 关注数据访问层实现
 * @author Knot Team
 * @date 2025-10-16
 */

#include "database/follow_repository.h"
#include "database/mysql_statement.h"
#include "utils/logger.h"
#include <cstring>
#include <stdexcept>
#include <sstream>

// 创建关注记录
bool FollowRepository::create(MYSQL* conn, int64_t followerId, int64_t followeeId) {
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
        const char* query = "INSERT INTO follows (follower_id, followee_id) VALUES (?, ?)";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        // follower_id
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &followerId;

        // followee_id
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &followeeId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            // 检查是否是唯一约束冲突（ER_DUP_ENTRY = 1062）
            unsigned int err_no = mysql_stmt_errno(stmt.get());
            if (err_no == 1062) {
                Logger::warning("User already following (follower_id=" + std::to_string(followerId) +
                               ", followee_id=" + std::to_string(followeeId) + ")");
            } else {
                Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            }
            return false;
        }

        Logger::info("Follow created (follower_id=" + std::to_string(followerId) +
                    ", followee_id=" + std::to_string(followeeId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::create: " + std::string(e.what()));
        return false;
    }
}

// 删除关注记录（取消关注）
bool FollowRepository::deleteByFollowerAndFollowee(MYSQL* conn, int64_t followerId, int64_t followeeId) {
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
        const char* query = "DELETE FROM follows WHERE follower_id = ? AND followee_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        // follower_id
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &followerId;

        // followee_id
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &followeeId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 检查是否真的删除了记录
        my_ulonglong affected_rows = mysql_stmt_affected_rows(stmt.get());
        if (affected_rows == 0) {
            Logger::warning("No follow relationship found to delete (follower_id=" + std::to_string(followerId) +
                           ", followee_id=" + std::to_string(followeeId) + ")");
            return false;
        }

        Logger::info("Follow deleted (follower_id=" + std::to_string(followerId) +
                    ", followee_id=" + std::to_string(followeeId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::deleteByFollowerAndFollowee: " + std::string(e.what()));
        return false;
    }
}

// 检查关注关系是否存在
bool FollowRepository::exists(MYSQL* conn, int64_t followerId, int64_t followeeId) {
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
        const char* query = "SELECT 1 FROM follows WHERE follower_id = ? AND followee_id = ? LIMIT 1";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        // follower_id
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &followerId;

        // followee_id
        bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[1].buffer = &followeeId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定结果
        int result = 0;
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));

        result_bind[0].buffer_type = MYSQL_TYPE_LONG;
        result_bind[0].buffer = &result;

        if (mysql_stmt_bind_result(stmt.get(), result_bind) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 获取结果
        int fetch_result = mysql_stmt_fetch(stmt.get());
        return (fetch_result == 0);  // 0表示成功获取一行数据

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::exists: " + std::string(e.what()));
        return false;
    }
}

// 统计用户关注的人数
int FollowRepository::countFollowing(MYSQL* conn, int64_t userId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        // SQL 统计语句
        const char* query = "SELECT COUNT(*) FROM follows WHERE follower_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
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
        int64_t count = 0;
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));

        result_bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[0].buffer = &count;

        if (mysql_stmt_bind_result(stmt.get(), result_bind) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 获取结果
        if (mysql_stmt_fetch(stmt.get()) == 0) {
            return static_cast<int>(count);
        }

        return 0;

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::countFollowing: " + std::string(e.what()));
        return 0;
    }
}

// 统计用户的粉丝数
int FollowRepository::countFollowers(MYSQL* conn, int64_t userId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        // SQL 统计语句
        const char* query = "SELECT COUNT(*) FROM follows WHERE followee_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
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
        int64_t count = 0;
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));

        result_bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[0].buffer = &count;

        if (mysql_stmt_bind_result(stmt.get(), result_bind) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 获取结果
        if (mysql_stmt_fetch(stmt.get()) == 0) {
            return static_cast<int>(count);
        }

        return 0;

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::countFollowers: " + std::string(e.what()));
        return 0;
    }
}

// 查询用户关注的人列表（我关注的人）
std::vector<Follow> FollowRepository::findFollowingByUserId(MYSQL* conn, int64_t userId, int limit, int offset) {
    std::vector<Follow> follows;
    
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return follows;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return follows;
        }

        // SQL 查询语句
        const char* query = 
            "SELECT id, follower_id, followee_id, UNIX_TIMESTAMP(create_time) as create_time "
            "FROM follows "
            "WHERE follower_id = ? "
            "ORDER BY create_time DESC "
            "LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return follows;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        // userId
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &userId;

        // limit
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &limit;

        // offset
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return follows;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return follows;
        }

        // 绑定结果
        int64_t id, follower_id, followee_id, create_time;
        MYSQL_BIND result_bind[4];
        memset(result_bind, 0, sizeof(result_bind));

        result_bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[0].buffer = &id;

        result_bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[1].buffer = &follower_id;

        result_bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[2].buffer = &followee_id;

        result_bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[3].buffer = &create_time;

        if (mysql_stmt_bind_result(stmt.get(), result_bind) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return follows;
        }

        // 获取所有结果
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Follow follow(id, follower_id, followee_id);
            follow.setCreateTime(static_cast<std::time_t>(create_time));
            follows.push_back(follow);
        }

        Logger::debug("Found " + std::to_string(follows.size()) + " following for user_id=" + std::to_string(userId));
        return follows;

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::findFollowingByUserId: " + std::string(e.what()));
        return follows;
    }
}

// 查询用户的粉丝列表（关注我的人）
std::vector<Follow> FollowRepository::findFollowersByUserId(MYSQL* conn, int64_t userId, int limit, int offset) {
    std::vector<Follow> follows;
    
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return follows;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return follows;
        }

        // SQL 查询语句
        const char* query = 
            "SELECT id, follower_id, followee_id, UNIX_TIMESTAMP(create_time) as create_time "
            "FROM follows "
            "WHERE followee_id = ? "
            "ORDER BY create_time DESC "
            "LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return follows;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        // userId
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &userId;

        // limit
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &limit;

        // offset
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return follows;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return follows;
        }

        // 绑定结果
        int64_t id, follower_id, followee_id, create_time;
        MYSQL_BIND result_bind[4];
        memset(result_bind, 0, sizeof(result_bind));

        result_bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[0].buffer = &id;

        result_bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[1].buffer = &follower_id;

        result_bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[2].buffer = &followee_id;

        result_bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[3].buffer = &create_time;

        if (mysql_stmt_bind_result(stmt.get(), result_bind) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return follows;
        }

        // 获取所有结果
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Follow follow(id, follower_id, followee_id);
            follow.setCreateTime(static_cast<std::time_t>(create_time));
            follows.push_back(follow);
        }

        Logger::debug("Found " + std::to_string(follows.size()) + " followers for user_id=" + std::to_string(userId));
        return follows;

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::findFollowersByUserId: " + std::string(e.what()));
        return follows;
    }
}

// 批量检查关注关系
std::map<int64_t, bool> FollowRepository::batchCheckExists(MYSQL* conn, int64_t followerId, 
                                                             const std::vector<int64_t>& followeeIds) {
    std::map<int64_t, bool> result;
    
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return result;
        }

        if (followeeIds.empty()) {
            return result;
        }

        // 初始化所有用户为未关注
        for (int64_t followeeId : followeeIds) {
            result[followeeId] = false;
        }

        // 构建IN子句的占位符
        std::string placeholders;
        for (size_t i = 0; i < followeeIds.size(); ++i) {
            if (i > 0) placeholders += ",";
            placeholders += "?";
        }

        std::string query = "SELECT followee_id FROM follows WHERE follower_id = ? AND followee_id IN (" + placeholders + ")";

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return result;
        }

        if (mysql_stmt_prepare(stmt.get(), query.c_str(), query.length()) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }

        // 绑定参数
        std::vector<MYSQL_BIND> bind(followeeIds.size() + 1);
        memset(bind.data(), 0, sizeof(MYSQL_BIND) * bind.size());

        // follower_id
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = const_cast<int64_t*>(&followerId);

        // followee_ids
        std::vector<int64_t> followeeIdsCopy = followeeIds;  // 需要可修改的副本
        for (size_t i = 0; i < followeeIds.size(); ++i) {
            bind[i + 1].buffer_type = MYSQL_TYPE_LONGLONG;
            bind[i + 1].buffer = &followeeIdsCopy[i];
        }

        if (mysql_stmt_bind_param(stmt.get(), bind.data()) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }

        // 绑定结果
        int64_t followee_id;
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));

        result_bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[0].buffer = &followee_id;

        if (mysql_stmt_bind_result(stmt.get(), result_bind) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }

        // 获取所有结果，将已关注的用户标记为true
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            result[followee_id] = true;
        }

        Logger::debug("Batch checked " + std::to_string(followeeIds.size()) + " follow relationships for follower_id=" + std::to_string(followerId));
        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::batchCheckExists: " + std::string(e.what()));
        return result;
    }
}

// 从SQL结果构建Follow对象
Follow FollowRepository::buildFollowFromStatement(MYSQL_STMT* stmt) {
    Follow follow;
    // 这个方法目前暂时不需要，因为我们在查询方法中直接构建了Follow对象
    return follow;
}

// 查询互关用户ID列表
std::vector<int64_t> FollowRepository::findMutualFollowIds(MYSQL* conn, int64_t userId, int limit, int offset) {
    std::vector<int64_t> mutualFollowIds;

    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return mutualFollowIds;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return mutualFollowIds;
        }

        // SQL查询：使用INNER JOIN查找双向关注关系
        // f1: 我关注的人 (follower_id = userId)
        // f2: 关注我的人 (followee_id = userId)
        // JOIN条件: f1.followee_id = f2.follower_id (对方也关注我)
        // 注意：使用GROUP BY代替DISTINCT，这样可以在ORDER BY中使用聚合函数
        const char* query =
            "SELECT f1.followee_id "
            "FROM follows f1 "
            "INNER JOIN follows f2 "
            "  ON f1.followee_id = f2.follower_id "
            "  AND f1.follower_id = f2.followee_id "
            "WHERE f1.follower_id = ? "
            "GROUP BY f1.followee_id "
            "ORDER BY MAX(f1.create_time) DESC "
            "LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return mutualFollowIds;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        // userId
        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &userId;

        // limit
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &limit;

        // offset
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return mutualFollowIds;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return mutualFollowIds;
        }

        // 绑定结果
        int64_t mutual_user_id;
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));

        result_bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[0].buffer = &mutual_user_id;

        if (mysql_stmt_bind_result(stmt.get(), result_bind) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return mutualFollowIds;
        }

        // 获取所有结果
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            mutualFollowIds.push_back(mutual_user_id);
        }

        Logger::debug("Found " + std::to_string(mutualFollowIds.size()) +
                     " mutual follows for user_id=" + std::to_string(userId));
        return mutualFollowIds;

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::findMutualFollowIds: " + std::string(e.what()));
        return mutualFollowIds;
    }
}

// 统计互关用户数量
int FollowRepository::countMutualFollows(MYSQL* conn, int64_t userId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        // SQL查询：统计双向关注数量
        const char* query =
            "SELECT COUNT(DISTINCT f1.followee_id) "
            "FROM follows f1 "
            "INNER JOIN follows f2 "
            "  ON f1.followee_id = f2.follower_id "
            "  AND f1.follower_id = f2.followee_id "
            "WHERE f1.follower_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
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
        int64_t count;
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));

        result_bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result_bind[0].buffer = &count;

        if (mysql_stmt_bind_result(stmt.get(), result_bind) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            Logger::debug("Counted " + std::to_string(count) +
                         " mutual follows for user_id=" + std::to_string(userId));
            return static_cast<int>(count);
        }

        return 0;

    } catch (const std::exception& e) {
        Logger::error("Exception in FollowRepository::countMutualFollows: " + std::string(e.what()));
        return 0;
    }
}



