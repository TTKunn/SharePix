/**
 * @file comment_repository.cpp
 * @brief 评论数据访问层实现
 * @author Knot Team
 * @date 2025-10-19
 */

#include "database/comment_repository.h"
#include "database/mysql_statement.h"
#include "utils/logger.h"
#include <cstring>
#include <stdexcept>

// 创建评论记录
bool CommentRepository::create(MYSQL* conn, const Comment& comment) {
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
        const char* query = "INSERT INTO comments (comment_id, post_id, user_id, content) VALUES (?, ?, ?, ?)";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[4];
        memset(bind, 0, sizeof(bind));

        std::string commentId = comment.getCommentId();
        unsigned long commentIdLength = commentId.length();
        int postId = comment.getPostId();
        int userId = comment.getUserId();
        std::string content = comment.getContent();
        unsigned long contentLength = content.length();

        // comment_id
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(commentId.c_str());
        bind[0].buffer_length = commentIdLength;
        bind[0].length = &commentIdLength;

        // post_id
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &postId;

        // user_id
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &userId;

        // content
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = const_cast<char*>(content.c_str());
        bind[3].buffer_length = contentLength;
        bind[3].length = &contentLength;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::info("Comment created (comment_id=" + commentId + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in CommentRepository::create: " + std::string(e.what()));
        return false;
    }
}

// 通过业务ID查询评论
std::optional<Comment> CommentRepository::findByCommentId(MYSQL* conn, const std::string& commentId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return std::nullopt;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return std::nullopt;
        }

        const char* query = "SELECT id, comment_id, post_id, user_id, content, UNIX_TIMESTAMP(create_time) "
                           "FROM comments WHERE comment_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        unsigned long commentIdLength = commentId.length();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(commentId.c_str());
        bind[0].buffer_length = commentIdLength;
        bind[0].length = &commentIdLength;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定结果
        MYSQL_BIND result[6];
        memset(result, 0, sizeof(result));

        int id;
        char commentIdBuf[37];
        unsigned long commentIdLen;
        int postId;
        int userId;
        char content[1001];
        unsigned long contentLen;
        long long createTime;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &id;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = commentIdBuf;
        result[1].buffer_length = sizeof(commentIdBuf);
        result[1].length = &commentIdLen;

        result[2].buffer_type = MYSQL_TYPE_LONG;
        result[2].buffer = &postId;

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &userId;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = content;
        result[4].buffer_length = sizeof(content);
        result[4].length = &contentLen;

        result[5].buffer_type = MYSQL_TYPE_LONGLONG;
        result[5].buffer = &createTime;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            Comment comment;
            comment.setId(id);
            comment.setCommentId(std::string(commentIdBuf, commentIdLen));
            comment.setPostId(postId);
            comment.setUserId(userId);
            comment.setContent(std::string(content, contentLen));
            comment.setCreateTime(static_cast<std::time_t>(createTime));
            return comment;
        }

        return std::nullopt;

    } catch (const std::exception& e) {
        Logger::error("Exception in CommentRepository::findByCommentId: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 查询帖子的评论列表（分页，按时间倒序）
std::vector<Comment> CommentRepository::findByPostId(MYSQL* conn, int postId, int limit, int offset) {
    std::vector<Comment> comments;

    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return comments;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return comments;
        }

        const char* query = "SELECT id, comment_id, post_id, user_id, content, UNIX_TIMESTAMP(create_time) "
                           "FROM comments WHERE post_id = ? ORDER BY create_time DESC LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &postId;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &limit;

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        // 绑定结果
        MYSQL_BIND result[6];
        memset(result, 0, sizeof(result));

        int id;
        char commentIdBuf[37];
        unsigned long commentIdLen;
        int postIdResult;
        int userId;
        char content[1001];
        unsigned long contentLen;
        long long createTime;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &id;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = commentIdBuf;
        result[1].buffer_length = sizeof(commentIdBuf);
        result[1].length = &commentIdLen;

        result[2].buffer_type = MYSQL_TYPE_LONG;
        result[2].buffer = &postIdResult;

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &userId;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = content;
        result[4].buffer_length = sizeof(content);
        result[4].length = &contentLen;

        result[5].buffer_type = MYSQL_TYPE_LONGLONG;
        result[5].buffer = &createTime;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Comment comment;
            comment.setId(id);
            comment.setCommentId(std::string(commentIdBuf, commentIdLen));
            comment.setPostId(postIdResult);
            comment.setUserId(userId);
            comment.setContent(std::string(content, contentLen));
            comment.setCreateTime(static_cast<std::time_t>(createTime));
            comments.push_back(comment);
        }

        Logger::info("Found " + std::to_string(comments.size()) + " comments for post_id=" + std::to_string(postId));
        return comments;

    } catch (const std::exception& e) {
        Logger::error("Exception in CommentRepository::findByPostId: " + std::string(e.what()));
        return comments;
    }
}

// 统计帖子的评论数
int CommentRepository::countByPostId(MYSQL* conn, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        const char* query = "SELECT COUNT(*) FROM comments WHERE post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

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

        long long count;
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
        Logger::error("Exception in CommentRepository::countByPostId: " + std::string(e.what()));
        return 0;
    }
}

// 删除评论（通过业务ID）
bool CommentRepository::deleteByCommentId(MYSQL* conn, const std::string& commentId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        const char* query = "DELETE FROM comments WHERE comment_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        unsigned long commentIdLength = commentId.length();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(commentId.c_str());
        bind[0].buffer_length = commentIdLength;
        bind[0].length = &commentIdLength;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::info("Comment deleted (comment_id=" + commentId + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in CommentRepository::deleteByCommentId: " + std::string(e.what()));
        return false;
    }
}

// 检查评论是否存在
bool CommentRepository::existsByCommentId(MYSQL* conn, const std::string& commentId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        const char* query = "SELECT COUNT(*) FROM comments WHERE comment_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        unsigned long commentIdLength = commentId.length();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(commentId.c_str());
        bind[0].buffer_length = commentIdLength;
        bind[0].length = &commentIdLength;

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

        long long count;
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
        Logger::error("Exception in CommentRepository::existsByCommentId: " + std::string(e.what()));
        return false;
    }
}

// 检查评论是否属于某用户
bool CommentRepository::isCommentOwner(MYSQL* conn, const std::string& commentId, int userId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        const char* query = "SELECT COUNT(*) FROM comments WHERE comment_id = ? AND user_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        unsigned long commentIdLength = commentId.length();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(commentId.c_str());
        bind[0].buffer_length = commentIdLength;
        bind[0].length = &commentIdLength;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &userId;

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

        long long count;
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
        Logger::error("Exception in CommentRepository::isCommentOwner: " + std::string(e.what()));
        return false;
    }
}

// 获取用户的评论列表
std::vector<Comment> CommentRepository::findByUserId(MYSQL* conn, int userId, int limit, int offset) {
    std::vector<Comment> comments;

    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return comments;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return comments;
        }

        const char* query = "SELECT id, comment_id, post_id, user_id, content, UNIX_TIMESTAMP(create_time) "
                           "FROM comments WHERE user_id = ? ORDER BY create_time DESC LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &userId;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &limit;

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        // 绑定结果
        MYSQL_BIND result[6];
        memset(result, 0, sizeof(result));

        int id;
        char commentIdBuf[37];
        unsigned long commentIdLen;
        int postId;
        int userIdResult;
        char content[1001];
        unsigned long contentLen;
        long long createTime;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &id;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = commentIdBuf;
        result[1].buffer_length = sizeof(commentIdBuf);
        result[1].length = &commentIdLen;

        result[2].buffer_type = MYSQL_TYPE_LONG;
        result[2].buffer = &postId;

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &userIdResult;

        result[4].buffer_type = MYSQL_TYPE_STRING;
        result[4].buffer = content;
        result[4].buffer_length = sizeof(content);
        result[4].length = &contentLen;

        result[5].buffer_type = MYSQL_TYPE_LONGLONG;
        result[5].buffer = &createTime;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return comments;
        }

        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Comment comment;
            comment.setId(id);
            comment.setCommentId(std::string(commentIdBuf, commentIdLen));
            comment.setPostId(postId);
            comment.setUserId(userIdResult);
            comment.setContent(std::string(content, contentLen));
            comment.setCreateTime(static_cast<std::time_t>(createTime));
            comments.push_back(comment);
        }

        Logger::info("Found " + std::to_string(comments.size()) + " comments for user_id=" + std::to_string(userId));
        return comments;

    } catch (const std::exception& e) {
        Logger::error("Exception in CommentRepository::findByUserId: " + std::string(e.what()));
        return comments;
    }
}

// 从预编译语句构建Comment对象
Comment CommentRepository::buildCommentFromStatement(MYSQL_STMT* stmt) {
    Comment comment;
    // 此方法预留,暂不实现
    return comment;
}
