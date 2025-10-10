/**
 * @file user_repository.h
 * @brief 用户数据访问层
 * @author Shared Parking Team
 * @date 2025-01-XX
 */

#pragma once

#include "models/user.h"
#include <optional>
#include <string>

/**
 * @brief 用户数据访问类
 * 
 * 封装所有用户相关的数据库操作，使用 MySQL 预编译语句防止 SQL 注入
 */
class UserRepository {
public:
    /**
     * @brief 构造函数
     */
    UserRepository();
    
    /**
     * @brief 析构函数
     */
    ~UserRepository() = default;
    
    /**
     * @brief 创建用户
     * 
     * 插入新用户到数据库
     * 
     * @param user 用户对象
     * @return true 创建成功，false 创建失败
     */
    bool createUser(const User& user);
    
    /**
     * @brief 根据物理ID查询用户
     * 
     * @param id 物理ID（自增主键）
     * @return 用户对象（未找到返回 std::nullopt）
     */
    std::optional<User> findById(int id);
    
    /**
     * @brief 根据逻辑ID查询用户
     * 
     * @param userId 逻辑ID（业务生成）
     * @return 用户对象（未找到返回 std::nullopt）
     */
    std::optional<User> findByUserId(const std::string& userId);
    
    /**
     * @brief 根据用户名查询用户
     * 
     * @param username 用户名
     * @return 用户对象（未找到返回 std::nullopt）
     */
    std::optional<User> findByUsername(const std::string& username);
    
    /**
     * @brief 根据邮箱查询用户
     * 
     * @param email 邮箱
     * @return 用户对象（未找到返回 std::nullopt）
     */
    std::optional<User> findByEmail(const std::string& email);
    
    /**
     * @brief 根据手机号查询用户
     * 
     * @param phone 手机号
     * @return 用户对象（未找到返回 std::nullopt）
     */
    std::optional<User> findByPhone(const std::string& phone);
    
    /**
     * @brief 更新用户信息
     * 
     * 根据用户ID更新用户信息
     * 
     * @param user 用户对象
     * @return true 更新成功，false 更新失败
     */
    bool updateUser(const User& user);
    
    /**
     * @brief 检查用户名是否存在
     * 
     * @param username 用户名
     * @return true 存在，false 不存在
     */
    bool usernameExists(const std::string& username);
    
    /**
     * @brief 检查邮箱是否存在
     * 
     * @param email 邮箱
     * @return true 存在，false 不存在
     */
    bool emailExists(const std::string& email);
    
    /**
     * @brief 检查手机号是否存在
     *
     * @param phone 手机号
     * @return true 存在，false 不存在
     */
    bool phoneExists(const std::string& phone);

    /**
     * @brief 更新用户基本信息
     *
     * 只更新允许用户自主修改的字段
     *
     * @param userId 用户ID
     * @param realName 真实姓名
     * @param email 邮箱
     * @param avatarUrl 头像URL
     * @param phone 手机号
     * @param bio 个人简介
     * @param gender 性别
     * @param location 所在地
     * @return true 更新成功，false 更新失败
     */
    bool updateUserProfile(int userId,
                          const std::string& realName,
                          const std::string& email,
                          const std::string& avatarUrl,
                          const std::string& phone,
                          const std::string& bio,
                          const std::string& gender,
                          const std::string& location);

    /**
     * @brief 检查邮箱是否被其他用户使用
     *
     * @param email 邮箱
     * @param excludeUserId 排除的用户ID（当前用户）
     * @return true 已被使用，false 未被使用
     */
    bool emailExistsForOtherUser(const std::string& email, int excludeUserId);

    /**
     * @brief 检查手机号是否被其他用户使用
     *
     * @param phone 手机号
     * @param excludeUserId 排除的用户ID（当前用户）
     * @return true 已被使用，false 未被使用
     */
    bool phoneExistsForOtherUser(const std::string& phone, int excludeUserId);

private:
    /**
     * @brief 从结果集构建 User 对象
     * 
     * @param stmt MySQL 预编译语句
     * @return 用户对象
     */
    User buildUserFromStatement(void* stmt);
    
    /**
     * @brief 执行查询并返回单个用户
     * 
     * @param query SQL 查询语句
     * @param paramValue 参数值
     * @return 用户对象（未找到返回 std::nullopt）
     */
    std::optional<User> executeQuerySingleUser(const char* query, const std::string& paramValue);
};

