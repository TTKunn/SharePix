/**
 * @file user_repository.cpp
 * @brief 用户数据访问层实现
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#include "database/user_repository.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include "database/mysql_statement.h"
#include "models/user_stats.h"
#include "utils/logger.h"
#include <mysql/mysql.h>
#include <cstring>
#include <memory>

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
    
    // 准备结果绑定（19个字段：原17个 + following_count + follower_count）
    MYSQL_BIND result[19];
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
    char bio[501] = {0};
    char gender[25] = {0};
    char location[101] = {0};
    int deviceCount;
    MYSQL_TIME createTime, updateTime;
    int followingCount;
    int followerCount;
    
    unsigned long userId_length, username_length, password_length, salt_length;
    unsigned long realName_length, phone_length, email_length;
    unsigned long role_length, status_length, avatarUrl_length;
    unsigned long bio_length, gender_length, location_length;

    bool email_is_null, avatarUrl_is_null;
    bool bio_is_null, gender_is_null, location_is_null;
    
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

    // bio (可为空)
    result[11].buffer_type = MYSQL_TYPE_STRING;
    result[11].buffer = bio;
    result[11].buffer_length = sizeof(bio);
    result[11].length = &bio_length;
    result[11].is_null = &bio_is_null;

    // gender (可为空)
    result[12].buffer_type = MYSQL_TYPE_STRING;
    result[12].buffer = gender;
    result[12].buffer_length = sizeof(gender);
    result[12].length = &gender_length;
    result[12].is_null = &gender_is_null;

    // location (可为空)
    result[13].buffer_type = MYSQL_TYPE_STRING;
    result[13].buffer = location;
    result[13].buffer_length = sizeof(location);
    result[13].length = &location_length;
    result[13].is_null = &location_is_null;

    // device_count
    result[14].buffer_type = MYSQL_TYPE_LONG;
    result[14].buffer = &deviceCount;

    // create_time
    result[15].buffer_type = MYSQL_TYPE_TIMESTAMP;
    result[15].buffer = &createTime;

    // update_time
    result[16].buffer_type = MYSQL_TYPE_TIMESTAMP;
    result[16].buffer = &updateTime;
    
    // following_count
    result[17].buffer_type = MYSQL_TYPE_LONG;
    result[17].buffer = &followingCount;
    
    // follower_count
    result[18].buffer_type = MYSQL_TYPE_LONG;
    result[18].buffer = &followerCount;
    
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

        if (!bio_is_null) {
            user.setBio(std::string(bio, bio_length));
        }

        if (!gender_is_null) {
            user.setGender(std::string(gender, gender_length));
        }

        if (!location_is_null) {
            user.setLocation(std::string(location, location_length));
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
        
        // 设置关注计数
        user.setFollowingCount(followingCount);
        user.setFollowerCount(followerCount);
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

// 更新用户基本信息
bool UserRepository::updateUserProfile(int userId,
                                      const std::string& realName,
                                      const std::string& email,
                                      const std::string& phone,
                                      const std::string& avatarUrl,
                                      const std::string& bio,
                                      const std::string& gender,
                                      const std::string& location) {
    try {
        // 使用ConnectionGuard自动管理连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("获取数据库连接失败");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            Logger::error("创建MySQL语句失败");
            return false;
        }

        // 准备SQL语句 - 只更新允许修改的字段
        const char* query =
            "UPDATE users SET "
            "real_name = ?, email = ?, phone = ?, avatar_url = ?, "
            "bio = ?, gender = ?, location = ?, "
            "update_time = CURRENT_TIMESTAMP "
            "WHERE id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("准备SQL语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[8];
        memset(bind, 0, sizeof(bind));

        // 准备NULL标志变量
        bool email_is_null = email.empty();
        bool avatar_is_null = avatarUrl.empty();
        bool bio_is_null = bio.empty();
        bool gender_is_null = gender.empty();
        bool location_is_null = location.empty();

        // real_name
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)realName.c_str();
        bind[0].buffer_length = realName.length();

        // email (可为空)
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)email.c_str();
        bind[1].buffer_length = email.length();
        bind[1].is_null = &email_is_null;

        // phone
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)phone.c_str();
        bind[2].buffer_length = phone.length();

        // avatar_url (可为空)
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = (char*)avatarUrl.c_str();
        bind[3].buffer_length = avatarUrl.length();
        bind[3].is_null = &avatar_is_null;

        // bio (可为空)
        bind[4].buffer_type = MYSQL_TYPE_STRING;
        bind[4].buffer = (char*)bio.c_str();
        bind[4].buffer_length = bio.length();
        bind[4].is_null = &bio_is_null;

        // gender (可为空)
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = (char*)gender.c_str();
        bind[5].buffer_length = gender.length();
        bind[5].is_null = &gender_is_null;

        // location (可为空)
        bind[6].buffer_type = MYSQL_TYPE_STRING;
        bind[6].buffer = (char*)location.c_str();
        bind[6].buffer_length = location.length();
        bind[6].is_null = &location_is_null;

        // userId (WHERE条件)
        bind[7].buffer_type = MYSQL_TYPE_LONG;
        bind[7].buffer = (char*)&userId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("执行SQL语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::info("用户信息更新成功: userId=" + std::to_string(userId));
        return true;

    } catch (const std::exception& e) {
        Logger::error("updateUserProfile异常: " + std::string(e.what()));
        return false;
    }
}

// 检查邮箱是否被其他用户使用
bool UserRepository::emailExistsForOtherUser(const std::string& email, int excludeUserId) {
    if (email.empty()) {
        return false;
    }

    try {
        // 使用ConnectionGuard自动管理连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("获取数据库连接失败");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }

        const char* query = "SELECT COUNT(*) FROM users WHERE email = ? AND id != ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("准备SQL语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)email.c_str();
        bind[0].buffer_length = email.length();

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = (char*)&excludeUserId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("执行SQL语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定结果
        MYSQL_BIND result[1];
        memset(result, 0, sizeof(result));

        long long count = 0;
        result[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result[0].buffer = (char*)&count;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("绑定结果失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            return count > 0;
        }

        return false;

    } catch (const std::exception& e) {
        Logger::error("emailExistsForOtherUser异常: " + std::string(e.what()));
        return false;
    }
}

// 检查手机号是否被其他用户使用
bool UserRepository::phoneExistsForOtherUser(const std::string& phone, int excludeUserId) {
    try {
        // 使用ConnectionGuard自动管理连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("获取数据库连接失败");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }

        const char* query = "SELECT COUNT(*) FROM users WHERE phone = ? AND id != ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("准备SQL语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)phone.c_str();
        bind[0].buffer_length = phone.length();

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = (char*)&excludeUserId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("执行SQL语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定结果
        MYSQL_BIND result[1];
        memset(result, 0, sizeof(result));

        long long count = 0;
        result[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result[0].buffer = (char*)&count;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("绑定结果失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            return count > 0;
        }

        return false;

    } catch (const std::exception& e) {
        Logger::error("phoneExistsForOtherUser异常: " + std::string(e.what()));
        return false;
    }
}

// 原子递增用户的关注数
bool UserRepository::incrementFollowingCount(MYSQL* conn, int64_t userId) {
    try {
        if (!conn) {
            Logger::error("数据库连接为空");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // SQL 原子更新语句
        const char* query = "UPDATE users SET following_count = following_count + 1 WHERE id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("预编译语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &userId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("执行SQL失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::debug("关注数+1 (user_id=" + std::to_string(userId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("incrementFollowingCount异常: " + std::string(e.what()));
        return false;
    }
}

// 原子递减用户的关注数
bool UserRepository::decrementFollowingCount(MYSQL* conn, int64_t userId) {
    try {
        if (!conn) {
            Logger::error("数据库连接为空");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // SQL 原子更新语句（防止减成负数）
        const char* query = "UPDATE users SET following_count = following_count - 1 WHERE id = ? AND following_count > 0";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("预编译语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &userId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("执行SQL失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::debug("关注数-1 (user_id=" + std::to_string(userId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("decrementFollowingCount异常: " + std::string(e.what()));
        return false;
    }
}

// 原子递增用户的粉丝数
bool UserRepository::incrementFollowerCount(MYSQL* conn, int64_t userId) {
    try {
        if (!conn) {
            Logger::error("数据库连接为空");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // SQL 原子更新语句
        const char* query = "UPDATE users SET follower_count = follower_count + 1 WHERE id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("预编译语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &userId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("执行SQL失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::debug("粉丝数+1 (user_id=" + std::to_string(userId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("incrementFollowerCount异常: " + std::string(e.what()));
        return false;
    }
}

// 原子递减用户的粉丝数
bool UserRepository::decrementFollowerCount(MYSQL* conn, int64_t userId) {
    try {
        if (!conn) {
            Logger::error("数据库连接为空");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // SQL 原子更新语句（防止减成负数）
        const char* query = "UPDATE users SET follower_count = follower_count - 1 WHERE id = ? AND follower_count > 0";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("预编译语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[0].buffer = &userId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("执行SQL失败: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::debug("粉丝数-1 (user_id=" + std::to_string(userId) + ")");
        return true;

    } catch (const std::exception& e) {
        Logger::error("decrementFollowerCount异常: " + std::string(e.what()));
        return false;
    }
}

// 获取用户统计信息
std::optional<UserStats> UserRepository::getUserStats(MYSQL* conn, const std::string& userId) {
    try {
        if (!conn) {
            Logger::error("数据库连接为空");
            return std::nullopt;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return std::nullopt;
        }

        // SQL 查询语句 - 联表查询用户统计
        const char* query = 
            "SELECT u.user_id, u.following_count, u.follower_count, "
            "COUNT(DISTINCT p.id) as post_count, "
            "COALESCE(SUM(p.like_count), 0) as total_likes "
            "FROM users u "
            "LEFT JOIN posts p ON u.id = p.user_id "
            "WHERE u.user_id = ? "
            "GROUP BY u.id";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("预编译语句失败: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        // 需要可修改的副本
        std::string userIdCopy = userId;
        unsigned long userIdLength = userIdCopy.length();

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = const_cast<char*>(userIdCopy.c_str());
        bind[0].buffer_length = userIdCopy.length();
        bind[0].length = &userIdLength;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("执行SQL失败: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定结果
        char user_id_buf[256];
        int following_count = 0, follower_count = 0, post_count = 0, total_likes = 0;
        unsigned long user_id_length;
        bool is_null[5];

        MYSQL_BIND result[5];
        memset(result, 0, sizeof(result));

        result[0].buffer_type = MYSQL_TYPE_STRING;
        result[0].buffer = user_id_buf;
        result[0].buffer_length = sizeof(user_id_buf);
        result[0].length = &user_id_length;
        result[0].is_null = &is_null[0];

        result[1].buffer_type = MYSQL_TYPE_LONG;
        result[1].buffer = &following_count;
        result[1].is_null = &is_null[1];

        result[2].buffer_type = MYSQL_TYPE_LONG;
        result[2].buffer = &follower_count;
        result[2].is_null = &is_null[2];

        result[3].buffer_type = MYSQL_TYPE_LONG;
        result[3].buffer = &post_count;
        result[3].is_null = &is_null[3];

        result[4].buffer_type = MYSQL_TYPE_LONG;
        result[4].buffer = &total_likes;
        result[4].is_null = &is_null[4];

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("绑定结果失败: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 获取结果
        if (mysql_stmt_fetch(stmt.get()) == 0) {
            user_id_buf[user_id_length] = '\0';
            UserStats stats(
                std::string(user_id_buf),
                following_count,
                follower_count,
                post_count,
                total_likes
            );
            return stats;
        }

        return std::nullopt;

    } catch (const std::exception& e) {
        Logger::error("getUserStats异常: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 批量获取用户信息
std::unordered_map<int, User> UserRepository::batchGetUsers(
    MYSQL* conn,
    const std::vector<int>& userIds
) {
    std::unordered_map<int, User> result;
    
    try {
        // 空列表检查
        if (userIds.empty()) {
            Logger::info("batchGetUsers: 用户ID列表为空");
            return result;
        }
        
        if (!conn) {
            Logger::error("batchGetUsers: 数据库连接为空");
            return result;
        }
        
        Logger::info("batchGetUsers: 批量查询 " + std::to_string(userIds.size()) + " 个用户信息");
        
        // ========================================
        // 第1步: 构建SQL - SELECT * FROM users WHERE id IN (?, ?, ...)
        // ========================================
        std::ostringstream sqlBuilder;
        sqlBuilder << "SELECT id, user_id, username, real_name, avatar_url, "
                   << "bio, gender, location, following_count, follower_count "
                   << "FROM users WHERE id IN (";
        
        for (size_t i = 0; i < userIds.size(); i++) {
            if (i > 0) sqlBuilder << ", ";
            sqlBuilder << "?";
        }
        sqlBuilder << ")";
        
        std::string sql = sqlBuilder.str();
        Logger::debug("batchGetUsers SQL: " + sql);
        
        // ========================================
        // 第2步: 准备预编译语句
        // ========================================
        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            Logger::error("batchGetUsers: 创建语句失败");
            return result;
        }
        
        if (mysql_stmt_prepare(stmt.get(), sql.c_str(), sql.length()) != 0) {
            Logger::error("batchGetUsers: 预编译失败: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }
        
        // ========================================
        // 第3步: 绑定参数
        // ========================================
        std::vector<MYSQL_BIND> binds(userIds.size());
        std::memset(binds.data(), 0, userIds.size() * sizeof(MYSQL_BIND));
        
        for (size_t i = 0; i < userIds.size(); i++) {
            binds[i].buffer_type = MYSQL_TYPE_LONG;
            binds[i].buffer = const_cast<int*>(&userIds[i]);
            binds[i].is_null = 0;
        }
        
        if (mysql_stmt_bind_param(stmt.get(), binds.data()) != 0) {
            Logger::error("batchGetUsers: 绑定参数失败: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }
        
        // ========================================
        // 第4步: 执行查询
        // ========================================
        auto startTime = std::chrono::steady_clock::now();
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("batchGetUsers: 执行查询失败: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }
        
        // ========================================
        // 第5步: 绑定结果列
        // ========================================
        int id;
        char userIdBuf[128], usernameBuf[128], realNameBuf[128], avatarBuf[256];
        char bioBuf[501], genderBuf[25], locationBuf[101];
        unsigned long userIdLen, usernameLen, realNameLen, avatarLen;
        unsigned long bioLen, genderLen, locationLen;
        bool avatarIsNull, bioIsNull, genderIsNull, locationIsNull;
        int followingCount, followerCount;
        
        MYSQL_BIND resultBinds[10];
        std::memset(resultBinds, 0, sizeof(resultBinds));
        
        // id
        resultBinds[0].buffer_type = MYSQL_TYPE_LONG;
        resultBinds[0].buffer = &id;
        
        // user_id
        resultBinds[1].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[1].buffer = userIdBuf;
        resultBinds[1].buffer_length = sizeof(userIdBuf);
        resultBinds[1].length = &userIdLen;
        
        // username
        resultBinds[2].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[2].buffer = usernameBuf;
        resultBinds[2].buffer_length = sizeof(usernameBuf);
        resultBinds[2].length = &usernameLen;
        
        // real_name
        resultBinds[3].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[3].buffer = realNameBuf;
        resultBinds[3].buffer_length = sizeof(realNameBuf);
        resultBinds[3].length = &realNameLen;
        
        // avatar_url (可为空)
        resultBinds[4].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[4].buffer = avatarBuf;
        resultBinds[4].buffer_length = sizeof(avatarBuf);
        resultBinds[4].length = &avatarLen;
        resultBinds[4].is_null = &avatarIsNull;
        
        // bio (可为空)
        resultBinds[5].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[5].buffer = bioBuf;
        resultBinds[5].buffer_length = sizeof(bioBuf);
        resultBinds[5].length = &bioLen;
        resultBinds[5].is_null = &bioIsNull;
        
        // gender (可为空)
        resultBinds[6].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[6].buffer = genderBuf;
        resultBinds[6].buffer_length = sizeof(genderBuf);
        resultBinds[6].length = &genderLen;
        resultBinds[6].is_null = &genderIsNull;
        
        // location (可为空)
        resultBinds[7].buffer_type = MYSQL_TYPE_STRING;
        resultBinds[7].buffer = locationBuf;
        resultBinds[7].buffer_length = sizeof(locationBuf);
        resultBinds[7].length = &locationLen;
        resultBinds[7].is_null = &locationIsNull;
        
        // following_count
        resultBinds[8].buffer_type = MYSQL_TYPE_LONG;
        resultBinds[8].buffer = &followingCount;
        
        // follower_count
        resultBinds[9].buffer_type = MYSQL_TYPE_LONG;
        resultBinds[9].buffer = &followerCount;
        
        if (mysql_stmt_bind_result(stmt.get(), resultBinds) != 0) {
            Logger::error("batchGetUsers: 绑定结果失败: " + std::string(mysql_stmt_error(stmt.get())));
            return result;
        }
        
        // ========================================
        // 第6步: 读取结果集
        // ========================================
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            User user;
            user.setId(id);
            user.setUserId(std::string(userIdBuf, userIdLen));
            user.setUsername(std::string(usernameBuf, usernameLen));
            user.setRealName(std::string(realNameBuf, realNameLen));
            
            if (!avatarIsNull) {
                user.setAvatarUrl(std::string(avatarBuf, avatarLen));
            }
            
            if (!bioIsNull) {
                user.setBio(std::string(bioBuf, bioLen));
            }
            
            if (!genderIsNull) {
                user.setGender(std::string(genderBuf, genderLen));
            }
            
            if (!locationIsNull) {
                user.setLocation(std::string(locationBuf, locationLen));
            }
            
            user.setFollowingCount(followingCount);
            user.setFollowerCount(followerCount);
            
            result[id] = user;
        }
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        ).count();
        
        Logger::info("batchGetUsers: 批量查询完成，找到 " + std::to_string(result.size()) + 
                    "/" + std::to_string(userIds.size()) + " 个用户，耗时: " + 
                    std::to_string(duration) + "ms");
        
        return result;
        
    } catch (const std::exception& e) {
        Logger::error("batchGetUsers异常: " + std::string(e.what()));
        return result;
    }
}

// 更新用户头像URL
bool UserRepository::updateAvatarUrl(int userId, const std::string& avatarUrl) {
    try {
        Logger::info("更新用户头像URL: userId=" + std::to_string(userId));
        
        // 1. 使用ConnectionGuard自动管理连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("获取数据库连接失败");
            return false;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 2. 准备SQL语句
        const char* query = "UPDATE users SET avatar_url = ?, update_time = CURRENT_TIMESTAMP WHERE id = ?";
        
        // 3. 创建预编译语句
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            Logger::error("mysql_stmt_init失败");
            return false;
        }
        
        // 4. 预编译SQL
        if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
            Logger::error("mysql_stmt_prepare失败: " + std::string(mysql_stmt_error(stmt)));
            mysql_stmt_close(stmt);
            return false;
        }
        
        // 5. 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));
        
        // avatar_url (可为空)
        bool avatar_is_null = avatarUrl.empty();
        
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)avatarUrl.c_str();
        bind[0].buffer_length = avatarUrl.length();
        bind[0].is_null = &avatar_is_null;
        
        // userId
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = (char*)&userId;
        bind[1].is_null = nullptr;
        
        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            Logger::error("mysql_stmt_bind_param失败: " + std::string(mysql_stmt_error(stmt)));
            mysql_stmt_close(stmt);
            return false;
        }
        
        // 6. 执行更新
        if (mysql_stmt_execute(stmt) != 0) {
            Logger::error("mysql_stmt_execute失败: " + std::string(mysql_stmt_error(stmt)));
            mysql_stmt_close(stmt);
            return false;
        }
        
        // 7. 检查影响行数
        my_ulonglong affected = mysql_stmt_affected_rows(stmt);
        mysql_stmt_close(stmt);
        
        if (affected == 0) {
            Logger::warning("更新头像URL失败: 用户不存在, userId=" + std::to_string(userId));
            return false;
        }
        
        Logger::info("头像URL更新成功: userId=" + std::to_string(userId));
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("updateAvatarUrl异常: " + std::string(e.what()));
        return false;
    }
}

