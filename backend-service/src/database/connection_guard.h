/**
 * @file connection_guard.h
 * @brief RAII封装数据库连接，自动管理连接生命周期
 * @author Shared Parking Team
 * @date 2025-10-02
 * 
 * 设计目的：
 * 1. 自动从连接池获取连接
 * 2. 自动归还连接到连接池（通过析构函数）
 * 3. 防止连接泄漏
 * 4. 异常安全
 * 
 * 使用示例：
 * @code
 * bool MyRepository::someMethod() {
 *     // 1. 创建ConnectionGuard，自动获取连接
 *     ConnectionGuard guard(pool_);
 *     if (!guard.isValid()) {
 *         Logger::error("Failed to get database connection");
 *         return false;
 *     }
 *     
 *     // 2. 使用连接
 *     MYSQL_STMT* stmt = mysql_stmt_init(guard.get());
 *     // ... 执行SQL操作 ...
 *     
 *     // 3. 函数结束时，guard析构，自动归还连接
 *     return true;
 * }
 * @endcode
 */

#pragma once

#include "connection_pool.h"
#include <memory>

/**
 * @brief 数据库连接守卫类（RAII模式）
 * 
 * 职责：
 * - 构造时：从连接池获取一个可用连接
 * - 析构时：自动将连接归还到连接池
 * - 提供安全的连接访问接口
 * 
 * 特性：
 * - 禁止拷贝和移动（确保连接唯一性）
 * - 异常安全（无论如何退出函数，连接都会归还）
 * - 零开销抽象（编译器优化后无性能损失）
 */
class ConnectionGuard {
public:
    /**
     * @brief 构造函数，从连接池获取连接
     * @param pool 数据库连接池引用
     * 
     * 说明：
     * - 自动调用 pool.getConnection() 获取连接
     * - 如果连接池耗尽，会等待可用连接（带超时）
     * - 获取失败时，conn_ 为 nullptr
     */
    explicit ConnectionGuard(DatabaseConnectionPool& pool);
    
    /**
     * @brief 析构函数，自动归还连接到连接池
     * 
     * 说明：
     * - 如果 conn_ 不为空，调用 pool.returnConnection()
     * - 无论函数如何退出（正常返回、异常、break等），都会执行
     * - 这是RAII模式的核心：资源自动释放
     */
    ~ConnectionGuard();
    
    /**
     * @brief 获取原始MySQL连接指针
     * @return MYSQL* 原始连接指针，如果无效则返回nullptr
     * 
     * 使用场景：
     * - 创建预编译语句：mysql_stmt_init(guard.get())
     * - 执行查询：mysql_real_query(guard.get(), ...)
     */
    MYSQL* get() const;
    
    /**
     * @brief 检查连接是否有效
     * @return true 连接有效且可用，false 连接无效或获取失败
     * 
     * 使用场景：
     * - 在使用连接前检查：if (!guard.isValid()) return false;
     * - 判断是否需要重试
     */
    bool isValid() const;
    
    /**
     * @brief 获取MySQLConnection对象指针（用于特殊场景）
     * @return MySQLConnection* 连接对象指针，如果无效则返回nullptr
     * 
     * 使用场景：
     * - 需要调用MySQLConnection的成员方法
     * - 例如：guard.getConnection()->execute(query)
     */
    MySQLConnection* getConnection() const;
    
    // ========== 禁止拷贝和移动 ==========
    // 原因：确保每个连接只有一个所有者，防止重复归还
    
    /**
     * @brief 禁止拷贝构造
     */
    ConnectionGuard(const ConnectionGuard&) = delete;
    
    /**
     * @brief 禁止拷贝赋值
     */
    ConnectionGuard& operator=(const ConnectionGuard&) = delete;
    
    /**
     * @brief 禁止移动构造
     */
    ConnectionGuard(ConnectionGuard&&) = delete;
    
    /**
     * @brief 禁止移动赋值
     */
    ConnectionGuard& operator=(ConnectionGuard&&) = delete;

private:
    DatabaseConnectionPool& pool_;                  // 连接池引用
    std::unique_ptr<MySQLConnection> conn_;         // 持有的连接（智能指针）
};

