/**
 * @file user_service.h
 * @brief 用户服务层定义
 * @author Knot Team
 * @date 2025-10-18
 */

#pragma once

#include "database/user_repository.h"
#include "models/user.h"
#include <memory>
#include <unordered_map>
#include <vector>

/**
 * @brief 用户服务类
 *
 * 提供用户相关的业务逻辑服务
 */
class UserService {
public:
    /**
     * @brief 构造函数
     */
    UserService();

    /**
     * @brief 析构函数
     */
    ~UserService() = default;

    /**
     * @brief 批量获取用户信息
     * @param userIds 用户ID列表（物理ID）
     * @return 用户信息映射表（key=userId, value=User对象）
     *
     * @example
     *   std::vector<int> userIds = {1, 2, 3};
     *   auto usersMap = batchGetUsers(userIds);
     *   // usersMap = {1: User{...}, 2: User{...}, 3: User{...}}
     *
     * @note 该方法内部管理数据库连接，调用方无需传入conn
     */
    std::unordered_map<int, User> batchGetUsers(
        const std::vector<int>& userIds
    );

private:
    std::unique_ptr<UserRepository> userRepo_;
};

