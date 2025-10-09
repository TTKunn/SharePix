/**
 * @file mysql_statement.h
 * @brief MySQL预编译语句RAII封装
 * @author Knot Team
 * @date 2025-10-08
 */

#pragma once

#include <mysql/mysql.h>
#include "utils/logger.h"

/**
 * @brief MySQL预编译语句RAII封装类
 * 
 * 自动管理MYSQL_STMT的生命周期，确保资源正确释放
 * 使用RAII模式，在析构时自动调用mysql_stmt_close
 */
class MySQLStatement {
public:
    /**
     * @brief 构造函数
     * @param conn MySQL连接对象
     */
    explicit MySQLStatement(MYSQL* conn) {
        stmt_ = mysql_stmt_init(conn);
        if (!stmt_) {
            Logger::error("Failed to initialize MySQL statement");
        }
    }
    
    /**
     * @brief 析构函数
     * 
     * 自动关闭预编译语句，释放资源
     */
    ~MySQLStatement() {
        if (stmt_) {
            mysql_stmt_close(stmt_);
        }
    }
    
    /**
     * @brief 获取MYSQL_STMT指针
     * @return MYSQL_STMT指针
     */
    MYSQL_STMT* get() { 
        return stmt_; 
    }
    
    /**
     * @brief 检查语句是否有效
     * @return 有效返回true，否则返回false
     */
    bool isValid() const {
        return stmt_ != nullptr;
    }
    
    /**
     * @brief 获取错误信息
     * @return 错误信息字符串
     */
    std::string getError() const {
        if (stmt_) {
            return std::string(mysql_stmt_error(stmt_));
        }
        return "Statement not initialized";
    }
    
    // 禁止拷贝
    MySQLStatement(const MySQLStatement&) = delete;
    MySQLStatement& operator=(const MySQLStatement&) = delete;
    
    // 禁止移动（简化实现）
    MySQLStatement(MySQLStatement&&) = delete;
    MySQLStatement& operator=(MySQLStatement&&) = delete;
    
private:
    MYSQL_STMT* stmt_;  ///< MySQL预编译语句指针
};

