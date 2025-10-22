/**
 * @file share_repository.cpp
 * @brief 分享数据访问层实现
 * @author Claude Code Assistant
 * @date 2025-10-22
 * @version v2.10.0
 */

#include "database/share_repository.h"
#include "database/mysql_statement.h"
#include "utils/logger.h"
#include <cstring>
#include <stdexcept>
#include <sstream>

// 创建分享记录
int ShareRepository::create(MYSQL* conn, const Share& share) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        // SQL 插入语句
        const char* query = "INSERT INTO shares (share_id, post_id, sender_id, receiver_id, share_message) "
                           "VALUES (?, ?, ?, ?, ?)";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定参数
        MYSQL_BIND bind[5];
        memset(bind, 0, sizeof(bind));

        std::string shareId = share.getShareId();
        int postId = share.getPostId();
        int senderId = share.getSenderId();
        int receiverId = share.getReceiverId();
        std::string shareMessage = share.getShareMessage();

        unsigned long shareIdLen = shareId.length();
        unsigned long shareMessageLen = shareMessage.length();

        // share_id
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)shareId.c_str();
        bind[0].buffer_length = shareId.length();
        bind[0].length = &shareIdLen;

        // post_id
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &postId;

        // sender_id
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &senderId;

        // receiver_id
        bind[3].buffer_type = MYSQL_TYPE_LONG;
        bind[3].buffer = &receiverId;

        // share_message
        bind[4].buffer_type = MYSQL_TYPE_STRING;
        bind[4].buffer = (char*)shareMessage.c_str();
        bind[4].buffer_length = shareMessage.length();
        bind[4].length = &shareMessageLen;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            // 检查是否是唯一约束冲突（ER_DUP_ENTRY = 1062）
            unsigned int err_no = mysql_stmt_errno(stmt.get());
            if (err_no == 1062) {
                Logger::warning("Share already exists (sender_id=" + std::to_string(senderId) +
                               ", receiver_id=" + std::to_string(receiverId) +
                               ", post_id=" + std::to_string(postId) + ")");
            } else {
                Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            }
            return 0;
        }

        int insertId = static_cast<int>(mysql_stmt_insert_id(stmt.get()));
        Logger::info("Share created (id=" + std::to_string(insertId) +
                    ", share_id=" + shareId + ")");
        return insertId;

    } catch (const std::exception& e) {
        Logger::error("Exception in ShareRepository::create: " + std::string(e.what()));
        return 0;
    }
}

// 根据ID查询分享记录
std::optional<Share> ShareRepository::findById(MYSQL* conn, int shareId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return std::nullopt;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return std::nullopt;
        }

        // SQL 查询语句
        const char* query = "SELECT id, share_id, post_id, sender_id, receiver_id, share_message, "
                           "UNIX_TIMESTAMP(create_time) as create_time "
                           "FROM shares WHERE id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &shareId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定结果
        MYSQL_BIND result[7];
        memset(result, 0, sizeof(result));

        int id;
        char share_id_buf[64];
        int post_id;
        int sender_id;
        int receiver_id;
        char share_message_buf[1024];
        long long create_time;

        unsigned long share_id_len;
        unsigned long share_message_len;
        bool share_id_null;
        bool share_message_null;
        bool create_time_null;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &id;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = share_id_buf;
        result[1].buffer_length = sizeof(share_id_buf);
        result[1].length = &share_id_len;
        result[1].is_null = &share_id_null;

        result[2].buffer_type = MYSQL_TYPE_LONG;
        result[2].buffer = &post_id;

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &sender_id;

        result[4].buffer_type = MYSQL_TYPE_LONG;
        result[4].buffer = &receiver_id;

        result[5].buffer_type = MYSQL_TYPE_STRING;
        result[5].buffer = share_message_buf;
        result[5].buffer_length = sizeof(share_message_buf);
        result[5].length = &share_message_len;
        result[5].is_null = &share_message_null;

        result[6].buffer_type = MYSQL_TYPE_LONGLONG;
        result[6].buffer = &create_time;
        result[6].is_null = &create_time_null;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        int fetch_result = mysql_stmt_fetch(stmt.get());
        if (fetch_result == 0) {
            // 构建Share对象
            Share share;
            share.setId(id);
            share.setShareId(share_id_null ? "" : std::string(share_id_buf, share_id_len));
            share.setPostId(post_id);
            share.setSenderId(sender_id);
            share.setReceiverId(receiver_id);
            share.setShareMessage(share_message_null ? "" : std::string(share_message_buf, share_message_len));
            share.setCreateTime(create_time_null ? 0 : static_cast<std::time_t>(create_time));

            return share;
        } else if (fetch_result == MYSQL_NO_DATA) {
            return std::nullopt;
        } else {
            Logger::error("Failed to fetch result: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in ShareRepository::findById: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 根据业务ID查询分享记录
std::optional<Share> ShareRepository::findByShareId(MYSQL* conn, const std::string& shareId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return std::nullopt;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return std::nullopt;
        }

        // SQL 查询语句
        const char* query = "SELECT id, share_id, post_id, sender_id, receiver_id, share_message, "
                           "UNIX_TIMESTAMP(create_time) as create_time "
                           "FROM shares WHERE share_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        unsigned long shareIdLen = shareId.length();

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)shareId.c_str();
        bind[0].buffer_length = shareId.length();
        bind[0].length = &shareIdLen;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定结果（与findById相同的逻辑）
        MYSQL_BIND result[7];
        memset(result, 0, sizeof(result));

        int id;
        char share_id_buf[64];
        int post_id;
        int sender_id;
        int receiver_id;
        char share_message_buf[1024];
        long long create_time;

        unsigned long share_id_len;
        unsigned long share_message_len;
        bool share_id_null;
        bool share_message_null;
        bool create_time_null;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &id;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = share_id_buf;
        result[1].buffer_length = sizeof(share_id_buf);
        result[1].length = &share_id_len;
        result[1].is_null = &share_id_null;

        result[2].buffer_type = MYSQL_TYPE_LONG;
        result[2].buffer = &post_id;

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &sender_id;

        result[4].buffer_type = MYSQL_TYPE_LONG;
        result[4].buffer = &receiver_id;

        result[5].buffer_type = MYSQL_TYPE_STRING;
        result[5].buffer = share_message_buf;
        result[5].buffer_length = sizeof(share_message_buf);
        result[5].length = &share_message_len;
        result[5].is_null = &share_message_null;

        result[6].buffer_type = MYSQL_TYPE_LONGLONG;
        result[6].buffer = &create_time;
        result[6].is_null = &create_time_null;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        int fetch_result = mysql_stmt_fetch(stmt.get());
        if (fetch_result == 0) {
            Share share;
            share.setId(id);
            share.setShareId(share_id_null ? "" : std::string(share_id_buf, share_id_len));
            share.setPostId(post_id);
            share.setSenderId(sender_id);
            share.setReceiverId(receiver_id);
            share.setShareMessage(share_message_null ? "" : std::string(share_message_buf, share_message_len));
            share.setCreateTime(create_time_null ? 0 : static_cast<std::time_t>(create_time));

            return share;
        } else if (fetch_result == MYSQL_NO_DATA) {
            return std::nullopt;
        } else {
            Logger::error("Failed to fetch result: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in ShareRepository::findByShareId: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 检查分享记录是否已存在
bool ShareRepository::exists(MYSQL* conn, int senderId, int receiverId, int postId) {
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
        const char* query = "SELECT 1 FROM shares WHERE sender_id = ? AND receiver_id = ? AND post_id = ? LIMIT 1";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &senderId;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &receiverId;

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        my_ulonglong num_rows = mysql_stmt_num_rows(stmt.get());
        return num_rows > 0;

    } catch (const std::exception& e) {
        Logger::error("Exception in ShareRepository::exists: " + std::string(e.what()));
        return false;
    }
}

// 查询接收到的分享列表
std::vector<Share> ShareRepository::findReceivedShares(MYSQL* conn, int receiverId, int limit, int offset) {
    std::vector<Share> shares;

    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return shares;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return shares;
        }

        // SQL 查询语句
        const char* query = "SELECT id, share_id, post_id, sender_id, receiver_id, share_message, "
                           "UNIX_TIMESTAMP(create_time) as create_time "
                           "FROM shares "
                           "WHERE receiver_id = ? "
                           "ORDER BY create_time DESC "
                           "LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &receiverId;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &limit;

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        // 绑定结果
        MYSQL_BIND result[7];
        memset(result, 0, sizeof(result));

        int id;
        char share_id_buf[64];
        int post_id;
        int sender_id;
        int receiver_id_result;
        char share_message_buf[1024];
        long long create_time;

        unsigned long share_id_len;
        unsigned long share_message_len;
        bool share_id_null;
        bool share_message_null;
        bool create_time_null;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &id;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = share_id_buf;
        result[1].buffer_length = sizeof(share_id_buf);
        result[1].length = &share_id_len;
        result[1].is_null = &share_id_null;

        result[2].buffer_type = MYSQL_TYPE_LONG;
        result[2].buffer = &post_id;

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &sender_id;

        result[4].buffer_type = MYSQL_TYPE_LONG;
        result[4].buffer = &receiver_id_result;

        result[5].buffer_type = MYSQL_TYPE_STRING;
        result[5].buffer = share_message_buf;
        result[5].buffer_length = sizeof(share_message_buf);
        result[5].length = &share_message_len;
        result[5].is_null = &share_message_null;

        result[6].buffer_type = MYSQL_TYPE_LONGLONG;
        result[6].buffer = &create_time;
        result[6].is_null = &create_time_null;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Share share;
            share.setId(id);
            share.setShareId(share_id_null ? "" : std::string(share_id_buf, share_id_len));
            share.setPostId(post_id);
            share.setSenderId(sender_id);
            share.setReceiverId(receiver_id_result);
            share.setShareMessage(share_message_null ? "" : std::string(share_message_buf, share_message_len));
            share.setCreateTime(create_time_null ? 0 : static_cast<std::time_t>(create_time));

            shares.push_back(share);
        }

        Logger::debug("Found " + std::to_string(shares.size()) + " received shares for receiver_id=" +
                     std::to_string(receiverId));

    } catch (const std::exception& e) {
        Logger::error("Exception in ShareRepository::findReceivedShares: " + std::string(e.what()));
    }

    return shares;
}

// 查询发送出的分享列表
std::vector<Share> ShareRepository::findSentShares(MYSQL* conn, int senderId, int limit, int offset) {
    std::vector<Share> shares;

    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return shares;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return shares;
        }

        // SQL 查询语句
        const char* query = "SELECT id, share_id, post_id, sender_id, receiver_id, share_message, "
                           "UNIX_TIMESTAMP(create_time) as create_time "
                           "FROM shares "
                           "WHERE sender_id = ? "
                           "ORDER BY create_time DESC "
                           "LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &senderId;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &limit;

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        // 绑定结果（与findReceivedShares相同的逻辑）
        MYSQL_BIND result[7];
        memset(result, 0, sizeof(result));

        int id;
        char share_id_buf[64];
        int post_id;
        int sender_id_result;
        int receiver_id;
        char share_message_buf[1024];
        long long create_time;

        unsigned long share_id_len;
        unsigned long share_message_len;
        bool share_id_null;
        bool share_message_null;
        bool create_time_null;

        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &id;

        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = share_id_buf;
        result[1].buffer_length = sizeof(share_id_buf);
        result[1].length = &share_id_len;
        result[1].is_null = &share_id_null;

        result[2].buffer_type = MYSQL_TYPE_LONG;
        result[2].buffer = &post_id;

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &sender_id_result;

        result[4].buffer_type = MYSQL_TYPE_LONG;
        result[4].buffer = &receiver_id;

        result[5].buffer_type = MYSQL_TYPE_STRING;
        result[5].buffer = share_message_buf;
        result[5].buffer_length = sizeof(share_message_buf);
        result[5].length = &share_message_len;
        result[5].is_null = &share_message_null;

        result[6].buffer_type = MYSQL_TYPE_LONGLONG;
        result[6].buffer = &create_time;
        result[6].is_null = &create_time_null;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return shares;
        }

        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Share share;
            share.setId(id);
            share.setShareId(share_id_null ? "" : std::string(share_id_buf, share_id_len));
            share.setPostId(post_id);
            share.setSenderId(sender_id_result);
            share.setReceiverId(receiver_id);
            share.setShareMessage(share_message_null ? "" : std::string(share_message_buf, share_message_len));
            share.setCreateTime(create_time_null ? 0 : static_cast<std::time_t>(create_time));

            shares.push_back(share);
        }

        Logger::debug("Found " + std::to_string(shares.size()) + " sent shares for sender_id=" +
                     std::to_string(senderId));

    } catch (const std::exception& e) {
        Logger::error("Exception in ShareRepository::findSentShares: " + std::string(e.what()));
    }

    return shares;
}

// 统计接收到的分享数量
int ShareRepository::countReceivedShares(MYSQL* conn, int receiverId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        const char* query = "SELECT COUNT(*) FROM shares WHERE receiver_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &receiverId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

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
        Logger::error("Exception in ShareRepository::countReceivedShares: " + std::string(e.what()));
        return 0;
    }
}

// 统计发送出的分享数量
int ShareRepository::countSentShares(MYSQL* conn, int senderId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        const char* query = "SELECT COUNT(*) FROM shares WHERE sender_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &senderId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

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
        Logger::error("Exception in ShareRepository::countSentShares: " + std::string(e.what()));
        return 0;
    }
}

// 删除分享记录
bool ShareRepository::deleteById(MYSQL* conn, int shareId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        const char* query = "DELETE FROM shares WHERE id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &shareId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        my_ulonglong affected_rows = mysql_stmt_affected_rows(stmt.get());
        if (affected_rows == 0) {
            Logger::warning("No share found to delete (id=" + std::to_string(shareId) + ")");
            return false;
        }

        Logger::info("Share deleted (id=" + std::to_string(shareId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in ShareRepository::deleteById: " + std::string(e.what()));
        return false;
    }
}

// 统计帖子的分享次数
int ShareRepository::countPostShares(MYSQL* conn, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return 0;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return 0;
        }

        const char* query = "SELECT COUNT(*) FROM shares WHERE post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

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
        Logger::error("Exception in ShareRepository::countPostShares: " + std::string(e.what()));
        return 0;
    }
}
