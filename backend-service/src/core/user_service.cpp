/**
 * @file user_service.cpp
 * @brief 用户服务层实现
 * @author Knot Team
 * @date 2025-10-18
 */

#include "core/user_service.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include "utils/logger.h"

// 构造函数
UserService::UserService() {
    userRepo_ = std::make_unique<UserRepository>();
    Logger::info("UserService initialized");
}

// 批量获取用户信息
std::unordered_map<int, User> UserService::batchGetUsers(
    const std::vector<int>& userIds
) {
    std::unordered_map<int, User> result;
    
    try {
        // 空列表检查
        if (userIds.empty()) {
            Logger::info("batchGetUsers: 用户ID列表为空");
            return result;
        }
        
        Logger::info("batchGetUsers: 批量查询 " + std::to_string(userIds.size()) + " 个用户信息");
        
        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("batchGetUsers: 获取数据库连接失败");
            return result;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 调用Repository批量查询
        result = userRepo_->batchGetUsers(conn, userIds);
        
        Logger::info("batchGetUsers: 批量查询完成，找到 " + 
                    std::to_string(result.size()) + " 个用户");
        
        return result;
        
    } catch (const std::exception& e) {
        Logger::error("batchGetUsers异常: " + std::string(e.what()));
        return result;
    }
}

