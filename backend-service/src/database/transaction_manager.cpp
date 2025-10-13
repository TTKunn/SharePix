/**
 * @file transaction_manager.cpp
 * @brief 数据库事务管理器实现
 * @author Knot Team
 * @date 2025-10-13
 */

#include "database/transaction_manager.h"
#include <stdexcept>

TransactionManager::TransactionManager(MYSQL* connection)
    : connection_(connection), isActive_(false), isCommitted_(false) {
    if (!connection_) {
        throw std::invalid_argument("Database connection cannot be null");
    }
}

TransactionManager::~TransactionManager() {
    if (isActive_ && !isCommitted_) {
        Logger::warning("Transaction was not committed, rolling back automatically");
        rollback();
    }
}

bool TransactionManager::begin() {
    if (isActive_) {
        Logger::warning("Transaction is already active");
        return false;
    }

    if (mysql_query(connection_, "START TRANSACTION") != 0) {
        Logger::error("Failed to start transaction: " + std::string(mysql_error(connection_)));
        return false;
    }

    isActive_ = true;
    isCommitted_ = false;
    Logger::debug("Transaction started");
    return true;
}

bool TransactionManager::commit() {
    if (!isActive_) {
        Logger::warning("No active transaction to commit");
        return false;
    }

    if (isCommitted_) {
        Logger::warning("Transaction has already been committed");
        return false;
    }

    if (mysql_query(connection_, "COMMIT") != 0) {
        Logger::error("Failed to commit transaction: " + std::string(mysql_error(connection_)));
        return false;
    }

    isCommitted_ = true;
    isActive_ = false;
    Logger::debug("Transaction committed successfully");
    return true;
}

bool TransactionManager::rollback() {
    if (!isActive_) {
        Logger::warning("No active transaction to rollback");
        return false;
    }

    if (mysql_query(connection_, "ROLLBACK") != 0) {
        Logger::error("Failed to rollback transaction: " + std::string(mysql_error(connection_)));
        return false;
    }

    isActive_ = false;
    isCommitted_ = false;
    Logger::debug("Transaction rolled back");
    return true;
}

bool TransactionManager::execute(const std::string& sql) {
    if (!isActive_) {
        Logger::error("Cannot execute SQL outside of transaction context");
        return false;
    }

    if (mysql_query(connection_, sql.c_str()) != 0) {
        Logger::error("Failed to execute SQL: " + sql +
                     ", Error: " + std::string(mysql_error(connection_)));
        return false;
    }

    Logger::debug("SQL executed successfully in transaction: " + sql);
    return true;
}

bool executeInTransaction(std::function<bool(MYSQL*)> func) {
    try {
        // 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection for transaction");
            return false;
        }

        MYSQL* conn = connGuard.get();
        TransactionManager transaction(conn);

        // 开始事务
        if (!transaction.begin()) {
            Logger::error("Failed to begin transaction");
            return false;
        }

        // 执行业务逻辑
        if (!func(conn)) {
            Logger::warning("Business logic failed, rolling back transaction");
            transaction.rollback();
            return false;
        }

        // 提交事务
        if (!transaction.commit()) {
            Logger::error("Failed to commit transaction");
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in transaction: " + std::string(e.what()));
        return false;
    }
}