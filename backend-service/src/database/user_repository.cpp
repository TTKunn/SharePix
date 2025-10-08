/**
 * @file user_repository.cpp
 * @brief 用户数据访问层实现
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#include "database/user_repository.h"
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
UserRepository::UserRepository() {
    Logger::info("UserRepository initialized");
}

// 创建用户
bool UserRepository::createUser(const User& user) {
    try {
        // 使用ConnectionGuard自动管理连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }
        
        // SQL 插入语句
        const char* query = "INSERT INTO users (user_id, username, password, salt, real_name, phone, email, role, status, avatar_url, device_count) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        // 绑定参数
        MYSQL_BIND bind[11];
        memset(bind, 0, sizeof(bind));
        
        // user_id
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)user.getUserId().c_str();
        bind[0].buffer_length = user.getUserId().length();
        
        // username
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)user.getUsername().c_str();
        bind[1].buffer_length = user.getUsername().length();
        
        // password
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)user.getPassword().c_str();
        bind[2].buffer_length = user.getPassword().length();
        
        // salt
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = (char*)user.getSalt().c_str();
        bind[3].buffer_length = user.getSalt().length();
        
        // real_name
        bind[4].buffer_type = MYSQL_TYPE_STRING;
        bind[4].buffer = (char*)user.getRealName().c_str();
        bind[4].buffer_length = user.getRealName().length();
        
        // phone
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = (char*)user.getPhone().c_str();
        bind[5].buffer_length = user.getPhone().length();
        
        // email (可为空)
        std::string email = user.getEmail();
        bool email_is_null = email.empty();
        bind[6].buffer_type = MYSQL_TYPE_STRING;
        bind[6].buffer = (char*)email.c_str();
        bind[6].buffer_length = email.length();
        bind[6].is_null = &email_is_null;
        
        // role
        std::string role = User::roleToString(user.getRole());
        bind[7].buffer_type = MYSQL_TYPE_STRING;
        bind[7].buffer = (char*)role.c_str();
        bind[7].buffer_length = role.length();
        
        // status
        std::string status = User::statusToString(user.getStatus());
        bind[8].buffer_type = MYSQL_TYPE_STRING;
        bind[8].buffer = (char*)status.c_str();
        bind[8].buffer_length = status.length();
        
        // avatar_url (可为空)
        std::string avatarUrl = user.getAvatarUrl();
        bool avatar_is_null = avatarUrl.empty();
        bind[9].buffer_type = MYSQL_TYPE_STRING;
        bind[9].buffer = (char*)avatarUrl.c_str();
        bind[9].buffer_length = avatarUrl.length();
        bind[9].is_null = &avatar_is_null;
        
        // device_count
        int deviceCount = user.getDeviceCount();
        bind[10].buffer_type = MYSQL_TYPE_LONG;
        bind[10].buffer = &deviceCount;
        
        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        Logger::info("User created successfully: " + user.getUsername());
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in createUser: " + std::string(e.what()));
        return false;
    }
}

// 从预编译语句构建 User 对象
User UserRepository::buildUserFromStatement(void* stmtPtr) {
    MYSQL_STMT* stmt = static_cast<MYSQL_STMT*>(stmtPtr);
    User user;
    
    // 准备结果绑定
    MYSQL_BIND result[14];
    memset(result, 0, sizeof(result));
    
    // 定义变量存储结果
    long long id;
    char userId[37] = {0};
    char username[51] = {0};
    char password[256] = {0};
    char salt[65] = {0};
    char realName[51] = {0};
    char phone[21] = {0};
    char email[101] = {0};
    char role[10] = {0};
    char status[10] = {0};
    char avatarUrl[256] = {0};
    int deviceCount;
    MYSQL_TIME createTime, updateTime;
    
    unsigned long userId_length, username_length, password_length, salt_length;
    unsigned long realName_length, phone_length, email_length;
    unsigned long role_length, status_length, avatarUrl_length;
    
    bool email_is_null, avatarUrl_is_null;
    
    // 绑定结果（按照 SELECT * 的顺序）
    // id
    result[0].buffer_type = MYSQL_TYPE_LONGLONG;
    result[0].buffer = &id;
    
    // user_id
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = userId;
    result[1].buffer_length = sizeof(userId);
    result[1].length = &userId_length;
    
    // username
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = username;
    result[2].buffer_length = sizeof(username);
    result[2].length = &username_length;
    
    // password
    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = password;
    result[3].buffer_length = sizeof(password);
    result[3].length = &password_length;
    
    // salt
    result[4].buffer_type = MYSQL_TYPE_STRING;
    result[4].buffer = salt;
    result[4].buffer_length = sizeof(salt);
    result[4].length = &salt_length;
    
    // real_name
    result[5].buffer_type = MYSQL_TYPE_STRING;
    result[5].buffer = realName;
    result[5].buffer_length = sizeof(realName);
    result[5].length = &realName_length;
    
    // phone
    result[6].buffer_type = MYSQL_TYPE_STRING;
    result[6].buffer = phone;
    result[6].buffer_length = sizeof(phone);
    result[6].length = &phone_length;
    
    // email (可为空)
    result[7].buffer_type = MYSQL_TYPE_STRING;
    result[7].buffer = email;
    result[7].buffer_length = sizeof(email);
    result[7].length = &email_length;
    result[7].is_null = &email_is_null;
    
    // role
    result[8].buffer_type = MYSQL_TYPE_STRING;
    result[8].buffer = role;
    result[8].buffer_length = sizeof(role);
    result[8].length = &role_length;
    
    // status
    result[9].buffer_type = MYSQL_TYPE_STRING;
    result[9].buffer = status;
    result[9].buffer_length = sizeof(status);
    result[9].length = &status_length;
    
    // avatar_url (可为空)
    result[10].buffer_type = MYSQL_TYPE_STRING;
    result[10].buffer = avatarUrl;
    result[10].buffer_length = sizeof(avatarUrl);
    result[10].length = &avatarUrl_length;
    result[10].is_null = &avatarUrl_is_null;
    
    // device_count
    result[11].buffer_type = MYSQL_TYPE_LONG;
    result[11].buffer = &deviceCount;
    
    // create_time
    result[12].buffer_type = MYSQL_TYPE_TIMESTAMP;
    result[12].buffer = &createTime;
    
    // update_time
    result[13].buffer_type = MYSQL_TYPE_TIMESTAMP;
    result[13].buffer = &updateTime;
    
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt)));
        return user;
    }
    
    // 获取数据
    if (mysql_stmt_fetch(stmt) == 0) {
        user.setId(static_cast<int>(id));
        user.setUserId(std::string(userId, userId_length));
        user.setUsername(std::string(username, username_length));
        user.setPassword(std::string(password, password_length));
        user.setSalt(std::string(salt, salt_length));
        user.setRealName(std::string(realName, realName_length));
        user.setPhone(std::string(phone, phone_length));
        
        if (!email_is_null) {
            user.setEmail(std::string(email, email_length));
        }
        
        user.setRole(User::stringToRole(std::string(role, role_length)));
        user.setStatus(User::stringToStatus(std::string(status, status_length)));
        
        if (!avatarUrl_is_null) {
            user.setAvatarUrl(std::string(avatarUrl, avatarUrl_length));
        }
        
        user.setDeviceCount(deviceCount);
        
        // 转换时间
        struct tm tm_create = {0};
        tm_create.tm_year = createTime.year - 1900;
        tm_create.tm_mon = createTime.month - 1;
        tm_create.tm_mday = createTime.day;
        tm_create.tm_hour = createTime.hour;
        tm_create.tm_min = createTime.minute;
        tm_create.tm_sec = createTime.second;
        user.setCreateTime(mktime(&tm_create));
        
        struct tm tm_update = {0};
        tm_update.tm_year = updateTime.year - 1900;
        tm_update.tm_mon = updateTime.month - 1;
        tm_update.tm_mday = updateTime.day;
        tm_update.tm_hour = updateTime.hour;
        tm_update.tm_min = updateTime.minute;
        tm_update.tm_sec = updateTime.second;
        user.setUpdateTime(mktime(&tm_update));
    }
    
    return user;
}

// 执行查询并返回单个用户
std::optional<User> UserRepository::executeQuerySingleUser(const char* query, const std::string& paramValue) {
    try {
        // 使用ConnectionGuard自动管理连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return std::nullopt;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return std::nullopt;
        }

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)paramValue.c_str();
        bind[0].buffer_length = paramValue.length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 存储结果
        if (mysql_stmt_store_result(stmt.get()) != 0) {
            Logger::error("Failed to store result: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 检查是否有结果
        if (mysql_stmt_num_rows(stmt.get()) == 0) {
            return std::nullopt;
        }

        // 构建用户对象
        User user = buildUserFromStatement(stmt.get());
        return user;

    } catch (const std::exception& e) {
        Logger::error("Exception in executeQuerySingleUser: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 根据物理ID查询用户
std::optional<User> UserRepository::findById(int id) {
    const char* query = "SELECT * FROM users WHERE id = ?";
    return executeQuerySingleUser(query, std::to_string(id));
}

// 根据逻辑ID查询用户
std::optional<User> UserRepository::findByUserId(const std::string& userId) {
    const char* query = "SELECT * FROM users WHERE user_id = ?";
    return executeQuerySingleUser(query, userId);
}

// 根据用户名查询用户
std::optional<User> UserRepository::findByUsername(const std::string& username) {
    const char* query = "SELECT * FROM users WHERE username = ?";
    return executeQuerySingleUser(query, username);
}

// 根据邮箱查询用户
std::optional<User> UserRepository::findByEmail(const std::string& email) {
    if (email.empty()) {
        return std::nullopt;
    }
    const char* query = "SELECT * FROM users WHERE email = ?";
    return executeQuerySingleUser(query, email);
}

// 根据手机号查询用户
std::optional<User> UserRepository::findByPhone(const std::string& phone) {
    const char* query = "SELECT * FROM users WHERE phone = ?";
    return executeQuerySingleUser(query, phone);
}

// 更新用户信息
bool UserRepository::updateUser(const User& user) {
    try {
        // 使用ConnectionGuard自动管理连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }

        const char* query = "UPDATE users SET username = ?, password = ?, salt = ?, real_name = ?, phone = ?, email = ?, role = ?, status = ?, avatar_url = ?, device_count = ? WHERE id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[11];
        memset(bind, 0, sizeof(bind));

        // username
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)user.getUsername().c_str();
        bind[0].buffer_length = user.getUsername().length();

        // password
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)user.getPassword().c_str();
        bind[1].buffer_length = user.getPassword().length();

        // salt
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)user.getSalt().c_str();
        bind[2].buffer_length = user.getSalt().length();

        // real_name
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = (char*)user.getRealName().c_str();
        bind[3].buffer_length = user.getRealName().length();

        // phone
        bind[4].buffer_type = MYSQL_TYPE_STRING;
        bind[4].buffer = (char*)user.getPhone().c_str();
        bind[4].buffer_length = user.getPhone().length();

        // email (可为空)
        std::string email = user.getEmail();
        bool email_is_null = email.empty();
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = (char*)email.c_str();
        bind[5].buffer_length = email.length();
        bind[5].is_null = &email_is_null;

        // role
        std::string role = User::roleToString(user.getRole());
        bind[6].buffer_type = MYSQL_TYPE_STRING;
        bind[6].buffer = (char*)role.c_str();
        bind[6].buffer_length = role.length();

        // status
        std::string status = User::statusToString(user.getStatus());
        bind[7].buffer_type = MYSQL_TYPE_STRING;
        bind[7].buffer = (char*)status.c_str();
        bind[7].buffer_length = status.length();

        // avatar_url (可为空)
        std::string avatarUrl = user.getAvatarUrl();
        bool avatar_is_null = avatarUrl.empty();
        bind[8].buffer_type = MYSQL_TYPE_STRING;
        bind[8].buffer = (char*)avatarUrl.c_str();
        bind[8].buffer_length = avatarUrl.length();
        bind[8].is_null = &avatar_is_null;

        // device_count
        int deviceCount = user.getDeviceCount();
        bind[9].buffer_type = MYSQL_TYPE_LONG;
        bind[9].buffer = &deviceCount;

        // id (WHERE 条件)
        int id = user.getId();
        bind[10].buffer_type = MYSQL_TYPE_LONG;
        bind[10].buffer = &id;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::info("User updated successfully: " + user.getUsername());
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in updateUser: " + std::string(e.what()));
        return false;
    }
}

// 检查用户名是否存在
bool UserRepository::usernameExists(const std::string& username) {
    auto user = findByUsername(username);
    return user.has_value();
}

// 检查邮箱是否存在
bool UserRepository::emailExists(const std::string& email) {
    if (email.empty()) {
        return false;
    }
    auto user = findByEmail(email);
    return user.has_value();
}

// 检查手机号是否存在
bool UserRepository::phoneExists(const std::string& phone) {
    auto user = findByPhone(phone);
    return user.has_value();
}

