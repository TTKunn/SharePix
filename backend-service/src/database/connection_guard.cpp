/**
 * @file connection_guard.cpp
 * @brief RAII封装数据库连接的实现
 * @author Shared Parking Team
 * @date 2025-10-02
 */

#include "connection_guard.h"
#include "utils/logger.h"

// ============================================================================
// 构造函数：从连接池获取连接
// ============================================================================

ConnectionGuard::ConnectionGuard(DatabaseConnectionPool& pool)
    : pool_(pool), conn_(pool.getConnection()) {
    
    if (conn_) {
        // 连接获取成功
        Logger::debug("ConnectionGuard: Connection acquired from pool");
    } else {
        // 连接获取失败（连接池耗尽或超时）
        Logger::warning("ConnectionGuard: Failed to acquire connection from pool");
    }
}

// ============================================================================
// 析构函数：自动归还连接到连接池
// ============================================================================

ConnectionGuard::~ConnectionGuard() {
    if (conn_) {
        // 归还连接到连接池
        pool_.returnConnection(std::move(conn_));
        Logger::debug("ConnectionGuard: Connection returned to pool");
    }
    // 如果 conn_ 为空，说明从未获取到连接，无需归还
}

// ============================================================================
// 获取原始MySQL连接指针
// ============================================================================

MYSQL* ConnectionGuard::get() const {
    // 如果连接有效，返回原始MYSQL指针
    // 如果连接无效，返回nullptr
    return conn_ ? conn_->get() : nullptr;
}

// ============================================================================
// 检查连接是否有效
// ============================================================================

bool ConnectionGuard::isValid() const {
    // 检查两个条件：
    // 1. conn_ 不为空（成功获取到连接）
    // 2. conn_->isValid() 返回true（连接仍然有效）
    return conn_ && conn_->isValid();
}

// ============================================================================
// 获取MySQLConnection对象指针
// ============================================================================

MySQLConnection* ConnectionGuard::getConnection() const {
    // 返回智能指针持有的原始指针
    return conn_.get();
}

