/**
 * @file comment_service.cpp
 * @brief 评论服务实现
 * @author Knot Team
 * @date 2025-10-19
 */

#include "core/comment_service.h"
#include "database/comment_repository.h"
#include "database/post_repository.h"
#include "database/user_repository.h"
#include "database/connection_guard.h"
#include "database/connection_pool.h"
#include "database/mysql_statement.h"
#include "utils/logger.h"
#include <cstring>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <cctype>

// 构造函数
CommentService::CommentService() {
    commentRepo_ = std::make_unique<CommentRepository>();
    postRepo_ = std::make_unique<PostRepository>();
    userRepo_ = std::make_unique<UserRepository>();
    Logger::info("CommentService initialized");
}

// 析构函数
CommentService::~CommentService() {
    Logger::info("CommentService destroyed");
}

// 生成评论业务ID
std::string CommentService::generateCommentId() {
    // 格式：CMT_2025Q4_ABC123
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);

    // 获取年份和季度
    int year = tm->tm_year + 1900;
    int quarter = (tm->tm_mon / 3) + 1;

    // 生成随机字符串（6位）
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);

    std::string randomStr;
    for (int i = 0; i < 6; ++i) {
        randomStr += charset[dis(gen)];
    }

    std::ostringstream oss;
    oss << "CMT_" << year << "Q" << quarter << "_" << randomStr;
    return oss.str();
}

// 验证评论内容
std::string CommentService::validateContent(const std::string& content) {
    // 验证评论内容是否为空
    if (content.empty()) {
        return "评论内容不能为空";
    }

    // 检查是否为纯空格
    bool allSpaces = std::all_of(content.begin(), content.end(), [](unsigned char c) {
        return std::isspace(c);
    });
    if (allSpaces) {
        return "评论内容不能为纯空格";
    }

    // 检查长度（这里简化处理，按字节计数）
    if (content.length() > 1000) {
        return "评论内容不能超过1000字符";
    }

    // 检查特殊控制字符
    for (char c : content) {
        if (c < 0x20 && c != '\n' && c != '\r' && c != '\t') {
            return "评论内容包含非法字符";
        }
    }

    return "";  // 验证通过
}

// 创建评论
CommentResult CommentService::createComment(int userId, const std::string& postId, const std::string& content) {
    CommentResult result;

    try {
        Logger::info("User " + std::to_string(userId) + " creating comment on post " + postId);

        // 1. 验证评论内容
        std::string validationError = validateContent(content);
        if (!validationError.empty()) {
            result.statusCode = 400;
            result.message = validationError;
            return result;
        }

        // 2. 查询帖子是否存在
        auto post = postRepo_->findByPostId(postId);
        if (!post.has_value()) {
            result.statusCode = 404;
            result.message = "帖子不存在";
            return result;
        }

        // 3. 生成业务ID
        std::string commentId = generateCommentId();

        // 4. 创建Comment对象
        Comment comment;
        comment.setCommentId(commentId);
        comment.setPostId(post->getId());
        comment.setUserId(userId);
        comment.setContent(content);
        comment.setCreateTime(std::time(nullptr));

        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = connGuard.get();

        // 5. 开启事务
        if (mysql_query(conn, "START TRANSACTION") != 0) {
            Logger::error("Failed to start transaction: " + std::string(mysql_error(conn)));
            result.statusCode = 500;
            result.message = "事务开启失败";
            return result;
        }

        // 6. 创建评论记录
        if (!commentRepo_->create(conn, comment)) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "创建评论失败";
            return result;
        }

        // 7. 增加帖子评论数
        if (!postRepo_->incrementCommentCount(conn, post->getId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "更新评论数失败";
            return result;
        }

        // 8. 提交事务
        if (mysql_query(conn, "COMMIT") != 0) {
            Logger::error("Failed to commit transaction: " + std::string(mysql_error(conn)));
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "事务提交失败";
            return result;
        }

        // 9. 查询最新评论数
        auto updatedPost = postRepo_->findByPostId(postId);
        int newCommentCount = updatedPost.has_value() ? updatedPost->getCommentCount() : (post->getCommentCount() + 1);

        result.success = true;
        result.statusCode = 201;
        result.message = "评论发表成功";
        result.comment = comment;
        result.commentCount = newCommentCount;

        Logger::info("Comment created successfully (comment_id=" + commentId + ")");
        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in createComment: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}

// 获取帖子的评论列表
CommentListResult CommentService::getCommentsByPost(const std::string& postId, int page, int pageSize) {
    CommentListResult result;

    try {
        Logger::info("Getting comments for post " + postId + " (page=" + std::to_string(page) + ", pageSize=" + std::to_string(pageSize) + ")");

        // 1. 验证分页参数
        if (page < 1) {
            page = 1;
        }
        if (pageSize < 1 || pageSize > 100) {
            pageSize = 20;
        }

        // 2. 查询帖子是否存在
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

        // 3. 查询评论列表（分页）
        int offset = (page - 1) * pageSize;
        std::vector<Comment> comments = commentRepo_->findByPostId(conn, post->getId(), pageSize, offset);

        // 4. 统计总评论数
        int total = commentRepo_->countByPostId(conn, post->getId());

        // 5. 计算是否有更多评论
        bool hasMore = (offset + comments.size()) < total;

        result.success = true;
        result.statusCode = 200;
        result.message = "获取评论列表成功";
        result.comments = comments;
        result.total = total;
        result.hasMore = hasMore;

        Logger::info("Found " + std::to_string(comments.size()) + " comments (total=" + std::to_string(total) + ")");
        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in getCommentsByPost: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}

// 删除评论
CommentResult CommentService::deleteComment(int userId, const std::string& commentId) {
    CommentResult result;

    try {
        Logger::info("User " + std::to_string(userId) + " deleting comment " + commentId);

        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = connGuard.get();

        // 1. 查询评论是否存在
        auto comment = commentRepo_->findByCommentId(conn, commentId);
        if (!comment.has_value()) {
            result.statusCode = 404;
            result.message = "评论不存在";
            return result;
        }

        // 2. 查询帖子信息（用于权限验证）
        // 通过物理ID查询帖子（comment->getPostId()返回的是物理ID）
        // 需要先获取帖子的业务ID，然后通过业务ID查询完整信息
        // 这里简化处理：直接通过getUserId查询权限
        // 实际上我们需要一个通过物理ID查询Post的方法，暂时用Repository层的方法
        ConnectionGuard connGuard2(DatabaseConnectionPool::getInstance());
        if (!connGuard2.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        // 通过SQL直接查询帖子的user_id
        MYSQL* conn2 = connGuard2.get();
        MySQLStatement stmt(conn2);
        if (!stmt.isValid()) {
            result.statusCode = 500;
            result.message = "数据库查询失败";
            return result;
        }

        const char* query = "SELECT user_id FROM posts WHERE id = ?";
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            result.statusCode = 500;
            result.message = "数据库查询失败";
            return result;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));
        int postPhysicalId = comment->getPostId();
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &postPhysicalId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            result.statusCode = 500;
            result.message = "数据库查询失败";
            return result;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            result.statusCode = 500;
            result.message = "数据库查询失败";
            return result;
        }

        MYSQL_BIND resultBind[1];
        memset(resultBind, 0, sizeof(resultBind));
        int postUserId;
        resultBind[0].buffer_type = MYSQL_TYPE_LONG;
        resultBind[0].buffer = &postUserId;

        if (mysql_stmt_bind_result(stmt.get(), resultBind) != 0) {
            result.statusCode = 500;
            result.message = "数据库查询失败";
            return result;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            result.statusCode = 500;
            result.message = "数据库查询失败";
            return result;
        }

        if (mysql_stmt_fetch(stmt.get()) != 0) {
            result.statusCode = 404;
            result.message = "帖子不存在";
            return result;
        }

        // 3. 权限验证：评论作者或帖子作者可以删除
        bool isCommentOwner = (comment->getUserId() == userId);
        bool isPostOwner = (postUserId == userId);

        if (!isCommentOwner && !isPostOwner) {
            result.statusCode = 403;
            result.message = "无权限删除该评论";
            return result;
        }

        // 4. 开启事务
        if (mysql_query(conn, "START TRANSACTION") != 0) {
            Logger::error("Failed to start transaction: " + std::string(mysql_error(conn)));
            result.statusCode = 500;
            result.message = "事务开启失败";
            return result;
        }

        // 5. 删除评论记录
        if (!commentRepo_->deleteByCommentId(conn, commentId)) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "删除评论失败";
            return result;
        }

        // 6. 减少帖子评论数
        if (!postRepo_->decrementCommentCount(conn, comment->getPostId())) {
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "更新评论数失败";
            return result;
        }

        // 7. 提交事务
        if (mysql_query(conn, "COMMIT") != 0) {
            Logger::error("Failed to commit transaction: " + std::string(mysql_error(conn)));
            mysql_query(conn, "ROLLBACK");
            result.statusCode = 500;
            result.message = "事务提交失败";
            return result;
        }

        // 8. 查询最新评论数（通过SQL直接查询comment_count字段）
        const char* countQuery = "SELECT comment_count FROM posts WHERE id = ?";
        MySQLStatement countStmt(conn);
        if (!countStmt.isValid()) {
            result.statusCode = 500;
            result.message = "查询评论数失败";
            return result;
        }

        if (mysql_stmt_prepare(countStmt.get(), countQuery, strlen(countQuery)) != 0) {
            result.statusCode = 500;
            result.message = "查询评论数失败";
            return result;
        }

        MYSQL_BIND countBind[1];
        memset(countBind, 0, sizeof(countBind));
        int postId = comment->getPostId();
        countBind[0].buffer_type = MYSQL_TYPE_LONG;
        countBind[0].buffer = &postId;

        if (mysql_stmt_bind_param(countStmt.get(), countBind) != 0) {
            result.statusCode = 500;
            result.message = "查询评论数失败";
            return result;
        }

        if (mysql_stmt_execute(countStmt.get()) != 0) {
            result.statusCode = 500;
            result.message = "查询评论数失败";
            return result;
        }

        MYSQL_BIND countResult[1];
        memset(countResult, 0, sizeof(countResult));
        int newCommentCount = 0;
        countResult[0].buffer_type = MYSQL_TYPE_LONG;
        countResult[0].buffer = &newCommentCount;

        if (mysql_stmt_bind_result(countStmt.get(), countResult) != 0) {
            result.statusCode = 500;
            result.message = "查询评论数失败";
            return result;
        }

        if (mysql_stmt_store_result(countStmt.get()) != 0) {
            result.statusCode = 500;
            result.message = "查询评论数失败";
            return result;
        }

        mysql_stmt_fetch(countStmt.get());

        result.success = true;
        result.statusCode = 200;
        result.message = "评论已删除";
        result.commentCount = newCommentCount;

        Logger::info("Comment deleted successfully (comment_id=" + commentId + ")");
        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in deleteComment: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}

// 获取用户的评论列表
CommentListResult CommentService::getCommentsByUser(int userId, int page, int pageSize) {
    CommentListResult result;

    try {
        Logger::info("Getting comments for user " + std::to_string(userId) + " (page=" + std::to_string(page) + ", pageSize=" + std::to_string(pageSize) + ")");

        // 1. 验证分页参数
        if (page < 1) {
            page = 1;
        }
        if (pageSize < 1 || pageSize > 100) {
            pageSize = 20;
        }

        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.statusCode = 500;
            result.message = "数据库连接失败";
            return result;
        }

        MYSQL* conn = connGuard.get();

        // 2. 查询评论列表（分页）
        int offset = (page - 1) * pageSize;
        std::vector<Comment> comments = commentRepo_->findByUserId(conn, userId, pageSize, offset);

        // 3. 统计总评论数（这里简化处理，通过查询结果判断是否有更多）
        int total = comments.size();
        bool hasMore = (comments.size() == pageSize);

        result.success = true;
        result.statusCode = 200;
        result.message = "获取评论列表成功";
        result.comments = comments;
        result.total = total;
        result.hasMore = hasMore;

        Logger::info("Found " + std::to_string(comments.size()) + " comments for user");
        return result;

    } catch (const std::exception& e) {
        Logger::error("Exception in getCommentsByUser: " + std::string(e.what()));
        result.statusCode = 500;
        result.message = "服务器内部错误";
        return result;
    }
}
