/**
 * @file share_link_repository.cpp
 * @brief 分享链接数据访问层实现
 * @author Knot Development Team
 * @date 2025-10-16
 */

#include "database/share_link_repository.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include "utils/logger.h"
#include <cstring>
#include <sstream>

// 创建分享链接
bool ShareLinkRepository::create(ShareLink& link) {
    ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
    if (!connGuard.isValid()) {
        Logger::error("Failed to get database connection");
        return false;
    }
    
    MYSQL* conn = connGuard.get();
    
    // 准备SQL语句
    const char* sql = "INSERT INTO share_links "
                      "(short_code, target_type, target_id, creator_id, create_time, expire_time) "
                      "VALUES (?, ?, ?, ?, FROM_UNIXTIME(?), FROM_UNIXTIME(?))";
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        Logger::error("Failed to initialize statement");
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return false;
    }
    
    // 绑定参数
    MYSQL_BIND bind[6];
    memset(bind, 0, sizeof(bind));
    
    // short_code
    std::string shortCode = link.getShortCode();
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = const_cast<char*>(shortCode.c_str());
    bind[0].buffer_length = shortCode.length();
    
    // target_type
    std::string targetType = ShareLink::targetTypeToString(link.getTargetType());
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = const_cast<char*>(targetType.c_str());
    bind[1].buffer_length = targetType.length();
    
    // target_id
    int64_t targetId = link.getTargetId();
    bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[2].buffer = &targetId;
    
    // creator_id (可选)
    int64_t creatorId = 0;
    bool creatorIdIsNull = !link.getCreatorId().has_value();
    if (!creatorIdIsNull) {
        creatorId = link.getCreatorId().value();
    }
    bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[3].buffer = &creatorId;
    bind[3].is_null = &creatorIdIsNull;
    
    // create_time
    int64_t createTime = static_cast<int64_t>(link.getCreateTime());
    bind[4].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[4].buffer = &createTime;
    
    // expire_time (可选)
    int64_t expireTime = 0;
    bool expireTimeIsNull = !link.getExpireTime().has_value();
    if (!expireTimeIsNull) {
        expireTime = static_cast<int64_t>(link.getExpireTime().value());
    }
    bind[5].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[5].buffer = &expireTime;
    bind[5].is_null = &expireTimeIsNull;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return false;
    }
    
    // 执行语句
    if (mysql_stmt_execute(stmt)) {
        Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return false;
    }
    
    // 获取插入的ID
    link.setId(mysql_stmt_insert_id(stmt));
    
    mysql_stmt_close(stmt);
    
    Logger::info("Created share link: " + shortCode + " for target " + std::to_string(targetId));
    return true;
}

// 通过短码查找
std::optional<ShareLink> ShareLinkRepository::findByShortCode(const std::string& shortCode) {
    ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
    if (!connGuard.isValid()) {
        Logger::error("Failed to get database connection");
        return std::nullopt;
    }
    
    MYSQL* conn = connGuard.get();
    
    const char* sql = "SELECT id, short_code, target_type, target_id, creator_id, "
                      "UNIX_TIMESTAMP(create_time) AS create_time, "
                      "UNIX_TIMESTAMP(expire_time) AS expire_time "
                      "FROM share_links WHERE short_code = ?";
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        Logger::error("Failed to initialize statement");
        return std::nullopt;
    }
    
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    // 绑定参数
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = const_cast<char*>(shortCode.c_str());
    bind[0].buffer_length = shortCode.length();
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    // 执行查询
    if (mysql_stmt_execute(stmt)) {
        Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    // 绑定结果
    MYSQL_BIND result[7];
    memset(result, 0, sizeof(result));
    
    int64_t id, targetId, creatorId, createTime, expireTime;
    char shortCodeBuf[11], targetTypeBuf[20];
    bool creatorIdIsNull, expireTimeIsNull;
    unsigned long shortCodeLen, targetTypeLen;
    
    result[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result[0].buffer = &id;
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = shortCodeBuf;
    result[1].buffer_length = sizeof(shortCodeBuf);
    result[1].length = &shortCodeLen;
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = targetTypeBuf;
    result[2].buffer_length = sizeof(targetTypeBuf);
    result[2].length = &targetTypeLen;
    
    result[3].buffer_type = MYSQL_TYPE_LONGLONG;
    result[3].buffer = &targetId;
    
    result[4].buffer_type = MYSQL_TYPE_LONGLONG;
    result[4].buffer = &creatorId;
    result[4].is_null = &creatorIdIsNull;
    
    result[5].buffer_type = MYSQL_TYPE_LONGLONG;
    result[5].buffer = &createTime;
    
    result[6].buffer_type = MYSQL_TYPE_LONGLONG;
    result[6].buffer = &expireTime;
    result[6].is_null = &expireTimeIsNull;
    
    if (mysql_stmt_bind_result(stmt, result)) {
        Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    // 获取结果
    int fetchResult = mysql_stmt_fetch(stmt);
    if (fetchResult == 0) {
        // 构造ShareLink对象
        ShareLink link;
        link.setId(id);
        link.setShortCode(std::string(shortCodeBuf, shortCodeLen));
        link.setTargetType(ShareLink::stringToTargetType(std::string(targetTypeBuf, targetTypeLen)));
        link.setTargetId(targetId);
        
        if (!creatorIdIsNull) {
            link.setCreatorId(creatorId);
        }
        
        link.setCreateTime(static_cast<time_t>(createTime));
        
        if (!expireTimeIsNull) {
            link.setExpireTime(static_cast<time_t>(expireTime));
        }
        
        mysql_stmt_close(stmt);
        return link;
    }
    
    mysql_stmt_close(stmt);
    return std::nullopt;
}

// 通过目标ID查找（去重检查）
std::optional<ShareLink> ShareLinkRepository::findByTargetId(
    ShareLink::TargetType targetType,
    int64_t targetId,
    const std::optional<int64_t>& creatorId
) {
    ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
    if (!connGuard.isValid()) {
        Logger::error("Failed to get database connection");
        return std::nullopt;
    }
    
    MYSQL* conn = connGuard.get();
    
    // SQL语句根据是否有creatorId而不同
    std::string sql = "SELECT id, short_code, target_type, target_id, creator_id, "
                      "UNIX_TIMESTAMP(create_time) AS create_time, "
                      "UNIX_TIMESTAMP(expire_time) AS expire_time "
                      "FROM share_links WHERE target_type = ? AND target_id = ?";
    
    if (creatorId.has_value()) {
        sql += " AND creator_id = ?";
    }
    
    sql += " AND (expire_time IS NULL OR expire_time > NOW()) LIMIT 1";
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        Logger::error("Failed to initialize statement");
        return std::nullopt;
    }
    
    if (mysql_stmt_prepare(stmt, sql.c_str(), sql.length())) {
        Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    // 绑定参数
    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));
    
    std::string targetTypeStr = ShareLink::targetTypeToString(targetType);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = const_cast<char*>(targetTypeStr.c_str());
    bind[0].buffer_length = targetTypeStr.length();
    
    bind[1].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[1].buffer = &targetId;
    
    int64_t creatorIdVal = 0;
    if (creatorId.has_value()) {
        creatorIdVal = creatorId.value();
        bind[2].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[2].buffer = &creatorIdVal;
        
        if (mysql_stmt_bind_param(stmt, bind)) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt)));
            mysql_stmt_close(stmt);
            return std::nullopt;
        }
    } else {
        // 只绑定前两个参数
        if (mysql_stmt_bind_param(stmt, bind)) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt)));
            mysql_stmt_close(stmt);
            return std::nullopt;
        }
    }
    
    // 执行查询
    if (mysql_stmt_execute(stmt)) {
        Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    // 绑定结果
    MYSQL_BIND result[7];
    memset(result, 0, sizeof(result));
    
    int64_t id, targetIdRes, creatorIdRes, createTime, expireTime;
    char shortCodeBuf[11], targetTypeBuf[20];
    bool creatorIdIsNull, expireTimeIsNull;
    unsigned long shortCodeLen, targetTypeLen;
    
    result[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result[0].buffer = &id;
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = shortCodeBuf;
    result[1].buffer_length = sizeof(shortCodeBuf);
    result[1].length = &shortCodeLen;
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = targetTypeBuf;
    result[2].buffer_length = sizeof(targetTypeBuf);
    result[2].length = &targetTypeLen;
    
    result[3].buffer_type = MYSQL_TYPE_LONGLONG;
    result[3].buffer = &targetIdRes;
    
    result[4].buffer_type = MYSQL_TYPE_LONGLONG;
    result[4].buffer = &creatorIdRes;
    result[4].is_null = &creatorIdIsNull;
    
    result[5].buffer_type = MYSQL_TYPE_LONGLONG;
    result[5].buffer = &createTime;
    
    result[6].buffer_type = MYSQL_TYPE_LONGLONG;
    result[6].buffer = &expireTime;
    result[6].is_null = &expireTimeIsNull;
    
    if (mysql_stmt_bind_result(stmt, result)) {
        Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    // 获取结果
    int fetchResult = mysql_stmt_fetch(stmt);
    if (fetchResult == 0) {
        ShareLink link;
        link.setId(id);
        link.setShortCode(std::string(shortCodeBuf, shortCodeLen));
        link.setTargetType(ShareLink::stringToTargetType(std::string(targetTypeBuf, targetTypeLen)));
        link.setTargetId(targetIdRes);
        
        if (!creatorIdIsNull) {
            link.setCreatorId(creatorIdRes);
        }
        
        link.setCreateTime(static_cast<time_t>(createTime));
        
        if (!expireTimeIsNull) {
            link.setExpireTime(static_cast<time_t>(expireTime));
        }
        
        mysql_stmt_close(stmt);
        return link;
    }
    
    mysql_stmt_close(stmt);
    return std::nullopt;
}

// 通过ID查找
std::optional<ShareLink> ShareLinkRepository::findById(int64_t id) {
    ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
    if (!connGuard.isValid()) {
        Logger::error("Failed to get database connection");
        return std::nullopt;
    }
    
    MYSQL* conn = connGuard.get();
    
    const char* sql = "SELECT id, short_code, target_type, target_id, creator_id, "
                      "UNIX_TIMESTAMP(create_time) AS create_time, "
                      "UNIX_TIMESTAMP(expire_time) AS expire_time "
                      "FROM share_links WHERE id = ?";
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        Logger::error("Failed to initialize statement");
        return std::nullopt;
    }
    
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[0].buffer = &id;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    if (mysql_stmt_execute(stmt)) {
        Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    MYSQL_BIND result[7];
    memset(result, 0, sizeof(result));
    
    int64_t idRes, targetIdRes, creatorIdRes, createTime, expireTime;
    char shortCodeBuf[11], targetTypeBuf[20];
    bool creatorIdIsNull, expireTimeIsNull;
    unsigned long shortCodeLen, targetTypeLen;
    
    result[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result[0].buffer = &idRes;
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = shortCodeBuf;
    result[1].buffer_length = sizeof(shortCodeBuf);
    result[1].length = &shortCodeLen;
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = targetTypeBuf;
    result[2].buffer_length = sizeof(targetTypeBuf);
    result[2].length = &targetTypeLen;
    
    result[3].buffer_type = MYSQL_TYPE_LONGLONG;
    result[3].buffer = &targetIdRes;
    
    result[4].buffer_type = MYSQL_TYPE_LONGLONG;
    result[4].buffer = &creatorIdRes;
    result[4].is_null = &creatorIdIsNull;
    
    result[5].buffer_type = MYSQL_TYPE_LONGLONG;
    result[5].buffer = &createTime;
    
    result[6].buffer_type = MYSQL_TYPE_LONGLONG;
    result[6].buffer = &expireTime;
    result[6].is_null = &expireTimeIsNull;
    
    if (mysql_stmt_bind_result(stmt, result)) {
        Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return std::nullopt;
    }
    
    int fetchResult = mysql_stmt_fetch(stmt);
    if (fetchResult == 0) {
        ShareLink link;
        link.setId(idRes);
        link.setShortCode(std::string(shortCodeBuf, shortCodeLen));
        link.setTargetType(ShareLink::stringToTargetType(std::string(targetTypeBuf, targetTypeLen)));
        link.setTargetId(targetIdRes);
        
        if (!creatorIdIsNull) {
            link.setCreatorId(creatorIdRes);
        }
        
        link.setCreateTime(static_cast<time_t>(createTime));
        
        if (!expireTimeIsNull) {
            link.setExpireTime(static_cast<time_t>(expireTime));
        }
        
        mysql_stmt_close(stmt);
        return link;
    }
    
    mysql_stmt_close(stmt);
    return std::nullopt;
}

// 删除分享链接
bool ShareLinkRepository::deleteById(int64_t id) {
    ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
    if (!connGuard.isValid()) {
        Logger::error("Failed to get database connection");
        return false;
    }
    
    MYSQL* conn = connGuard.get();
    
    const char* sql = "DELETE FROM share_links WHERE id = ?";
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        Logger::error("Failed to initialize statement");
        return false;
    }
    
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return false;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[0].buffer = &id;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return false;
    }
    
    if (mysql_stmt_execute(stmt)) {
        Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return false;
    }
    
    my_ulonglong affected = mysql_stmt_affected_rows(stmt);
    mysql_stmt_close(stmt);
    
    if (affected > 0) {
        Logger::info("Deleted share link with id: " + std::to_string(id));
        return true;
    }
    
    return false;
}

// 删除过期链接
int ShareLinkRepository::deleteExpired() {
    ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
    if (!connGuard.isValid()) {
        Logger::error("Failed to get database connection");
        return 0;
    }
    
    MYSQL* conn = connGuard.get();
    
    const char* sql = "DELETE FROM share_links WHERE expire_time IS NOT NULL AND expire_time < NOW()";
    
    if (mysql_query(conn, sql)) {
        Logger::error("Failed to delete expired links: " + std::string(mysql_error(conn)));
        return 0;
    }
    
    my_ulonglong affected = mysql_affected_rows(conn);
    
    if (affected > 0) {
        Logger::info("Deleted " + std::to_string(affected) + " expired share links");
    }
    
    return static_cast<int>(affected);
}

// 获取用户创建的分享链接列表
std::vector<ShareLink> ShareLinkRepository::findByCreatorId(
    int64_t creatorId,
    int limit,
    int offset
) {
    std::vector<ShareLink> links;
    
    ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
    if (!connGuard.isValid()) {
        Logger::error("Failed to get database connection");
        return links;
    }
    
    MYSQL* conn = connGuard.get();
    
    const char* sql = "SELECT id, short_code, target_type, target_id, creator_id, "
                      "UNIX_TIMESTAMP(create_time) AS create_time, "
                      "UNIX_TIMESTAMP(expire_time) AS expire_time "
                      "FROM share_links WHERE creator_id = ? "
                      "ORDER BY create_time DESC LIMIT ? OFFSET ?";
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        Logger::error("Failed to initialize statement");
        return links;
    }
    
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return links;
    }
    
    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[0].buffer = &creatorId;
    
    bind[1].buffer_type = MYSQL_TYPE_LONG;
    bind[1].buffer = &limit;
    
    bind[2].buffer_type = MYSQL_TYPE_LONG;
    bind[2].buffer = &offset;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return links;
    }
    
    if (mysql_stmt_execute(stmt)) {
        Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return links;
    }
    
    MYSQL_BIND result[7];
    memset(result, 0, sizeof(result));
    
    int64_t id, targetIdRes, creatorIdRes, createTime, expireTime;
    char shortCodeBuf[11], targetTypeBuf[20];
    bool creatorIdIsNull, expireTimeIsNull;
    unsigned long shortCodeLen, targetTypeLen;
    
    result[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result[0].buffer = &id;
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = shortCodeBuf;
    result[1].buffer_length = sizeof(shortCodeBuf);
    result[1].length = &shortCodeLen;
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = targetTypeBuf;
    result[2].buffer_length = sizeof(targetTypeBuf);
    result[2].length = &targetTypeLen;
    
    result[3].buffer_type = MYSQL_TYPE_LONGLONG;
    result[3].buffer = &targetIdRes;
    
    result[4].buffer_type = MYSQL_TYPE_LONGLONG;
    result[4].buffer = &creatorIdRes;
    result[4].is_null = &creatorIdIsNull;
    
    result[5].buffer_type = MYSQL_TYPE_LONGLONG;
    result[5].buffer = &createTime;
    
    result[6].buffer_type = MYSQL_TYPE_LONGLONG;
    result[6].buffer = &expireTime;
    result[6].is_null = &expireTimeIsNull;
    
    if (mysql_stmt_bind_result(stmt, result)) {
        Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt)));
        mysql_stmt_close(stmt);
        return links;
    }
    
    while (mysql_stmt_fetch(stmt) == 0) {
        ShareLink link;
        link.setId(id);
        link.setShortCode(std::string(shortCodeBuf, shortCodeLen));
        link.setTargetType(ShareLink::stringToTargetType(std::string(targetTypeBuf, targetTypeLen)));
        link.setTargetId(targetIdRes);
        
        if (!creatorIdIsNull) {
            link.setCreatorId(creatorIdRes);
        }
        
        link.setCreateTime(static_cast<time_t>(createTime));
        
        if (!expireTimeIsNull) {
            link.setExpireTime(static_cast<time_t>(expireTime));
        }
        
        links.push_back(link);
    }
    
    mysql_stmt_close(stmt);
    
    Logger::debug("Found " + std::to_string(links.size()) + " share links for creator " + std::to_string(creatorId));
    
    return links;
}

