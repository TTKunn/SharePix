/**
 * @file transaction_guard.h
 * @brief MySQL事务RAII封装
 * @author Knot Team
 * @date 2025-10-08
 */

#pragma once

#include <mysql/mysql.h>
#include "utils/logger.h"

/**
 * @brief MySQL事务RAII封装类
 * 
 * 自动管理事务的生命周期：
 * - 构造时开启事务（BEGIN）
 * - 析构时根据状态提交（COMMIT）或回滚（ROLLBACK）
 * - 支持手动提交和回滚
 * 
 * 使用示例：
 * @code
 * ConnectionGuard connGuard(pool);
 * TransactionGuard txn(connGuard.get());
 * 
 * // 执行多个SQL操作
 * if (!operation1()) {
 *     return false;  // 自动回滚
 * }
 * if (!operation2()) {
 *     return false;  // 自动回滚
 * }
 * 
 * txn.commit();  // 手动提交
 * @endcode
 */
class TransactionGuard {
public:
    /**
     * @brief 构造函数，开启事务
     * @param conn MySQL连接对象
     */
    explicit TransactionGuard(MYSQL* conn) 
        : conn_(conn), committed_(false), rolled_back_(false) {
        if (conn_) {
            if (mysql_query(conn_, "START TRANSACTION") != 0) {
                Logger::error("Failed to start transaction: " + 
                            std::string(mysql_error(conn_)));
                conn_ = nullptr;  // 标记为无效
            } else {
                Logger::debug("Transaction started");
            }
        }
    }
    
    /**
     * @brief 析构函数，自动提交或回滚
     * 
     * 如果未手动提交或回滚，则自动回滚
     */
    ~TransactionGuard() {
        if (conn_ && !committed_ && !rolled_back_) {
            rollback();
        }
    }
    
    /**
     * @brief 提交事务
     * @return 成功返回true，失败返回false
     */
    bool commit() {
        if (!conn_ || committed_ || rolled_back_) {
            return false;
        }
        
        if (mysql_query(conn_, "COMMIT") != 0) {
            Logger::error("Failed to commit transaction: " + 
                        std::string(mysql_error(conn_)));
            rollback();  // 提交失败，尝试回滚
            return false;
        }
        
        committed_ = true;
        Logger::debug("Transaction committed");
        return true;
    }
    
    /**
     * @brief 回滚事务
     * @return 成功返回true，失败返回false
     */
    bool rollback() {
        if (!conn_ || rolled_back_) {
            return false;
        }
        
        if (mysql_query(conn_, "ROLLBACK") != 0) {
            Logger::error("Failed to rollback transaction: " + 
                        std::string(mysql_error(conn_)));
            return false;
        }
        
        rolled_back_ = true;
        Logger::debug("Transaction rolled back");
        return true;
    }
    
    /**
     * @brief 检查事务是否有效
     * @return 有效返回true，否则返回false
     */
    bool isValid() const {
        return conn_ != nullptr;
    }
    
    /**
     * @brief 检查事务是否已提交
     * @return 已提交返回true，否则返回false
     */
    bool isCommitted() const {
        return committed_;
    }
    
    /**
     * @brief 检查事务是否已回滚
     * @return 已回滚返回true，否则返回false
     */
    bool isRolledBack() const {
        return rolled_back_;
    }
    
    // 禁止拷贝
    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;
    
    // 禁止移动
    TransactionGuard(TransactionGuard&&) = delete;
    TransactionGuard& operator=(TransactionGuard&&) = delete;
    
private:
    MYSQL* conn_;           ///< MySQL连接对象
    bool committed_;        ///< 是否已提交
    bool rolled_back_;      ///< 是否已回滚
};

