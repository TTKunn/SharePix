/**
 * @file connection_pool.cpp
 * @brief MySQL database connection pool implementation
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#include "database/connection_pool.h"
#include "utils/config_manager.h"
#include "utils/logger.h"
#include <json/json.h>
#include <stdexcept>

// ============================================================================
// MySQLConnection Implementation
// ============================================================================

MySQLConnection::MySQLConnection(MYSQL* conn) : connection_(conn) {
}

MySQLConnection::~MySQLConnection() {
    if (connection_) {
        mysql_close(connection_);
        connection_ = nullptr;
    }
}

bool MySQLConnection::isValid() const {
    if (!connection_) {
        return false;
    }
    
    // Ping to check if connection is alive
    return mysql_ping(connection_) == 0;
}

bool MySQLConnection::execute(const std::string& query) {
    if (!connection_) {
        return false;
    }
    
    int result = mysql_real_query(connection_, query.c_str(), query.length());
    if (result != 0) {
        Logger::error("MySQL query failed: " + std::string(mysql_error(connection_)));
        return false;
    }
    
    return true;
}

MYSQL_RES* MySQLConnection::executeQuery(const std::string& query) {
    if (!execute(query)) {
        return nullptr;
    }
    
    MYSQL_RES* result = mysql_store_result(connection_);
    if (!result && mysql_field_count(connection_) > 0) {
        Logger::error("MySQL store result failed: " + std::string(mysql_error(connection_)));
        return nullptr;
    }
    
    return result;
}

// ============================================================================
// DatabaseConnectionPool Implementation
// ============================================================================

DatabaseConnectionPool& DatabaseConnectionPool::getInstance() {
    static DatabaseConnectionPool instance;
    return instance;
}

DatabaseConnectionPool::~DatabaseConnectionPool() {
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    // Close all connections
    while (!connections_.empty()) {
        connections_.pop();
    }
    
    mysql_library_end();
}

bool DatabaseConnectionPool::initialize() {
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    if (initialized_) {
        Logger::warning("Database connection pool already initialized");
        return true;
    }
    
    // Initialize MySQL library
    if (mysql_library_init(0, nullptr, nullptr) != 0) {
        Logger::error("Failed to initialize MySQL library");
        return false;
    }
    
    // Load configuration
    auto& config = ConfigManager::getInstance();
    host_ = config.get<std::string>("database.host", "localhost");
    port_ = config.get<int>("database.port", 3306);
    database_ = config.get<std::string>("database.database", "shared_parking");
    username_ = config.get<std::string>("database.username", "root");
    password_ = config.get<std::string>("database.password", "");
    poolSize_ = config.get<int>("database.pool_size", 10);
    connectionTimeout_ = config.get<int>("database.connection_timeout", 30);
    
    Logger::info("Initializing database connection pool...");
    Logger::info("Host: " + host_ + ":" + std::to_string(port_));
    Logger::info("Database: " + database_);
    Logger::info("Pool size: " + std::to_string(poolSize_));
    
    // Test connection first
    if (!testConnection()) {
        Logger::error("Failed to connect to database");
        return false;
    }
    
    // Create initial connections
    for (int i = 0; i < poolSize_; ++i) {
        auto conn = createConnection();
        if (conn) {
            connections_.push(std::move(conn));
        } else {
            Logger::error("Failed to create connection " + std::to_string(i + 1));
            return false;
        }
    }
    
    initialized_ = true;
    Logger::info("Database connection pool initialized successfully");
    
    return true;
}

std::unique_ptr<MySQLConnection> DatabaseConnectionPool::getConnection() {
    std::unique_lock<std::mutex> lock(poolMutex_);
    
    if (!initialized_) {
        Logger::error("Connection pool not initialized");
        return nullptr;
    }
    
    // Wait for available connection (with timeout)
    if (connections_.empty()) {
        Logger::warning("Connection pool exhausted, waiting for available connection...");
        
        auto timeout = std::chrono::seconds(connectionTimeout_);
        if (!poolCondition_.wait_for(lock, timeout, [this] { return !connections_.empty(); })) {
            Logger::error("Connection pool timeout");
            return nullptr;
        }
    }
    
    // Get connection from pool
    auto conn = std::move(connections_.front());
    connections_.pop();
    
    // Validate connection
    if (!conn->isValid()) {
        Logger::warning("Invalid connection detected, creating new one");
        conn = createConnection();
    }
    
    return conn;
}

void DatabaseConnectionPool::returnConnection(std::unique_ptr<MySQLConnection> conn) {
    if (!conn) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    // Validate connection before returning
    if (conn->isValid()) {
        connections_.push(std::move(conn));
        poolCondition_.notify_one();
    } else {
        Logger::warning("Discarding invalid connection");
        // Connection will be destroyed automatically
    }
}

Json::Value DatabaseConnectionPool::getStats() const {
    std::lock_guard<std::mutex> lock(poolMutex_);

    Json::Value stats;
    stats["pool_size"] = poolSize_;
    stats["available_connections"] = static_cast<int>(connections_.size());
    stats["active_connections"] = poolSize_ - static_cast<int>(connections_.size());
    stats["initialized"] = initialized_;

    return stats;
}

std::unique_ptr<MySQLConnection> DatabaseConnectionPool::createConnection() {
    MYSQL* mysql = mysql_init(nullptr);
    if (!mysql) {
        Logger::error("Failed to initialize MySQL connection");
        return nullptr;
    }
    
    // Set connection timeout
    unsigned int timeout = connectionTimeout_;
    mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    
    // Set charset
    mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");
    
    // Enable auto-reconnect
    bool reconnect = true;
    mysql_options(mysql, MYSQL_OPT_RECONNECT, &reconnect);
    
    // Connect to database
    if (!mysql_real_connect(mysql, host_.c_str(), username_.c_str(), 
                           password_.c_str(), database_.c_str(), 
                           port_, nullptr, 0)) {
        Logger::error("Failed to connect to MySQL: " + std::string(mysql_error(mysql)));
        mysql_close(mysql);
        return nullptr;
    }
    
    return std::make_unique<MySQLConnection>(mysql);
}

bool DatabaseConnectionPool::testConnection() {
    auto conn = createConnection();
    if (!conn) {
        return false;
    }
    
    // Test simple query
    bool result = conn->execute("SELECT 1");
    
    return result;
}

