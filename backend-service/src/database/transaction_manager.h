/**
 * @file transaction_manager.h
 * @brief 数据库事务管理器
 * @author Knot Team
 * @date 2025-10-13
 */

#pragma once

#include "connection_pool.h"
#include "connection_guard.h"
#include "utils/logger.h"
#include <mysql/mysql.h>
#include <functional>

/**
 * @brief 数据库事务管理器
 *
 * 提供RAII风格的事务管理，确保事务在异常时自动回滚
 */
class TransactionManager {
private:
    MYSQL* connection_;
    bool isActive_;
    bool isCommitted_;

public:
    /**
     * @brief 构造函数
     * @param connection 数据库连接
     */
    explicit TransactionManager(MYSQL* connection);

    /**
     * @brief 析构函数 - 自动回滚未提交的事务
     */
    ~TransactionManager();

    // 禁止拷贝
    TransactionManager(const TransactionManager&) = delete;
    TransactionManager& operator=(const TransactionManager&) = delete;

    /**
     * @brief 开始事务
     * @return 是否成功
     */
    bool begin();

    /**
     * @brief 提交事务
     * @return 是否成功
     */
    bool commit();

    /**
     * @brief 回滚事务
     * @return 是否成功
     */
    bool rollback();

    /**
     * @brief 检查事务是否活跃
     * @return 事务状态
     */
    bool isActive() const { return isActive_; }

    /**
     * @brief 获取数据库连接
     * @return 数据库连接
     */
    MYSQL* getConnection() const { return connection_; }

    /**
     * @brief 执行SQL语句
     * @param sql SQL语句
     * @return 是否成功
     */
    bool execute(const std::string& sql);
};

/**
 * @brief 事务辅助函数 - 在事务上下文中执行操作
 *
 * @param func 要执行的函数，接受MYSQL*参数
 * @return 是否成功
 */
bool executeInTransaction(std::function<bool(MYSQL*)> func);