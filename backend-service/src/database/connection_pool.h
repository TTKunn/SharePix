/**
 * @file connection_pool.h
 * @brief MySQL database connection pool
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#pragma once

#include <mysql/mysql.h>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <json/json.h>

/**
 * @brief RAII wrapper for MySQL connection
 */
class MySQLConnection {
public:
    /**
     * @brief Constructor
     * @param conn MySQL connection handle
     */
    explicit MySQLConnection(MYSQL* conn);
    
    /**
     * @brief Destructor
     */
    ~MySQLConnection();
    
    /**
     * @brief Get raw MySQL connection handle
     * @return MySQL connection handle
     */
    MYSQL* get() const { return connection_; }
    
    /**
     * @brief Check if connection is valid
     * @return true if valid, false otherwise
     */
    bool isValid() const;
    
    /**
     * @brief Execute query
     * @param query SQL query string
     * @return true if successful, false otherwise
     */
    bool execute(const std::string& query);
    
    /**
     * @brief Execute query and get result
     * @param query SQL query string
     * @return MySQL result set (caller must free)
     */
    MYSQL_RES* executeQuery(const std::string& query);

private:
    MYSQL* connection_;
};

/**
 * @brief Singleton database connection pool
 */
class DatabaseConnectionPool {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to connection pool instance
     */
    static DatabaseConnectionPool& getInstance();
    
    /**
     * @brief Initialize connection pool
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * @brief Get connection from pool
     * @return Unique pointer to MySQL connection
     */
    std::unique_ptr<MySQLConnection> getConnection();
    
    /**
     * @brief Return connection to pool
     * @param conn Connection to return
     */
    void returnConnection(std::unique_ptr<MySQLConnection> conn);
    
    /**
     * @brief Get pool statistics
     * @return JSON object with pool stats
     */
    Json::Value getStats() const;
    
    // Prevent copying
    DatabaseConnectionPool(const DatabaseConnectionPool&) = delete;
    DatabaseConnectionPool& operator=(const DatabaseConnectionPool&) = delete;

private:
    DatabaseConnectionPool() = default;
    ~DatabaseConnectionPool();
    
    std::queue<std::unique_ptr<MySQLConnection>> connections_;
    mutable std::mutex poolMutex_;
    std::condition_variable poolCondition_;
    
    std::string host_;
    int port_;
    std::string database_;
    std::string username_;
    std::string password_;
    int poolSize_;
    int connectionTimeout_;
    
    bool initialized_;
    
    /**
     * @brief Create new MySQL connection
     * @return Unique pointer to MySQL connection
     */
    std::unique_ptr<MySQLConnection> createConnection();
    
    /**
     * @brief Test database connection
     * @return true if successful, false otherwise
     */
    bool testConnection();
};
