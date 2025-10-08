/**
 * @file tag_repository.cpp
 * @brief 标签数据访问层实现
 * @author Knot Team
 * @date 2025-10-07
 */

#include "database/tag_repository.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include "utils/logger.h"
#include <mysql/mysql.h>
#include <cstring>
#include <memory>

// RAII 封装 MYSQL_STMT
class MySQLStatement {
public:
    explicit MySQLStatement(MYSQL* conn) {
        stmt_ = mysql_stmt_init(conn);
        if (!stmt_) {
            Logger::error("Failed to initialize MySQL statement");
        }
    }
    
    ~MySQLStatement() {
        if (stmt_) {
            mysql_stmt_close(stmt_);
        }
    }
    
    MYSQL_STMT* get() { return stmt_; }
    
    // 禁止拷贝
    MySQLStatement(const MySQLStatement&) = delete;
    MySQLStatement& operator=(const MySQLStatement&) = delete;
    
private:
    MYSQL_STMT* stmt_;
};

// 构造函数
TagRepository::TagRepository() {
    Logger::info("TagRepository initialized");
}

// 从预编译语句构建 Tag 对象
Tag TagRepository::buildTagFromStatement(void* stmtPtr) {
    MYSQL_STMT* stmt = static_cast<MYSQL_STMT*>(stmtPtr);
    Tag tag;
    
    // 准备结果绑定
    MYSQL_BIND result[4];
    memset(result, 0, sizeof(result));
    
    // 定义变量存储结果
    long long id;
    char name[51] = {0};
    int useCount;
    MYSQL_TIME createTime;
    
    unsigned long name_length;
    
    // 绑定结果
    result[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result[0].buffer = &id;
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = name;
    result[1].buffer_length = sizeof(name);
    result[1].length = &name_length;
    
    result[2].buffer_type = MYSQL_TYPE_LONG;
    result[2].buffer = &useCount;
    
    result[3].buffer_type = MYSQL_TYPE_TIMESTAMP;
    result[3].buffer = &createTime;
    
    // 绑定结果
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt)));
        return tag;
    }
    
    // 获取数据
    if (mysql_stmt_fetch(stmt) == 0) {
        tag.setId(static_cast<int>(id));
        tag.setName(std::string(name, name_length));
        tag.setUseCount(useCount);
        
        // 转换时间
        struct tm tm_create = {0};
        tm_create.tm_year = createTime.year - 1900;
        tm_create.tm_mon = createTime.month - 1;
        tm_create.tm_mday = createTime.day;
        tm_create.tm_hour = createTime.hour;
        tm_create.tm_min = createTime.minute;
        tm_create.tm_sec = createTime.second;
        tag.setCreateTime(mktime(&tm_create));
    }
    
    return tag;
}

// 根据名称查找标签
std::optional<Tag> TagRepository::findByName(const std::string& name) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return std::nullopt;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return std::nullopt;
        }
        
        const char* query = "SELECT * FROM tags WHERE name = ?";
        
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }
        
        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)name.c_str();
        bind[0].buffer_length = name.length();
        
        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }
        
        Tag tag = buildTagFromStatement(stmt.get());
        
        if (tag.getId() > 0) {
            return tag;
        }
        
        return std::nullopt;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in findByName: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 创建标签
bool TagRepository::createTag(const Tag& tag) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }
        
        const char* query = "INSERT INTO tags (name, use_count) VALUES (?, ?)";
        
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)tag.getName().c_str();
        bind[0].buffer_length = tag.getName().length();
        
        int useCount = tag.getUseCount();
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &useCount;
        
        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        Logger::info("Tag created successfully: " + tag.getName());
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in createTag: " + std::string(e.what()));
        return false;
    }
}

// 关联图片和标签
bool TagRepository::linkImageTag(int imageId, int tagId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }
        
        const char* query = "INSERT INTO image_tags (image_id, tag_id) VALUES (?, ?)";
        
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &imageId;
        
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &tagId;
        
        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in linkImageTag: " + std::string(e.what()));
        return false;
    }
}

// 获取图片的所有标签
std::vector<Tag> TagRepository::getImageTags(int imageId) {
    std::vector<Tag> tags;
    
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return tags;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return tags;
        }
        
        const char* query = "SELECT t.* FROM tags t INNER JOIN image_tags it ON t.id = it.tag_id WHERE it.image_id = ?";
        
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return tags;
        }
        
        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &imageId;
        
        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return tags;
        }
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return tags;
        }
        
        // 获取所有结果
        while (true) {
            Tag tag = buildTagFromStatement(stmt.get());
            if (tag.getId() > 0) {
                tags.push_back(tag);
                // 移动到下一行
                if (mysql_stmt_fetch(stmt.get()) != 0) {
                    break;
                }
            } else {
                break;
            }
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in getImageTags: " + std::string(e.what()));
    }
    
    return tags;
}

// 增加标签使用次数
bool TagRepository::incrementUseCount(int tagId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }
        
        const char* query = "UPDATE tags SET use_count = use_count + 1 WHERE id = ?";
        
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &tagId;
        
        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in incrementUseCount: " + std::string(e.what()));
        return false;
    }
}

