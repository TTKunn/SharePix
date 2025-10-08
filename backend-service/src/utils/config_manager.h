/**
 * @file config_manager.h
 * @brief Configuration management for the authentication service
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#pragma once

#include <string>
#include <memory>
#include <json/json.h>

/**
 * @brief Singleton class for managing application configuration
 */
class ConfigManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to ConfigManager instance
     */
    static ConfigManager& getInstance();
    
    /**
     * @brief Load configuration from JSON file
     * @param configPath Path to configuration file
     * @return true if successful, false otherwise
     */
    bool loadConfig(const std::string& configPath);
    
    /**
     * @brief Get configuration value by key path
     * @param keyPath Dot-separated key path (e.g., "server.port")
     * @param defaultValue Default value if key not found
     * @return Configuration value
     */
    template<typename T>
    T get(const std::string& keyPath, const T& defaultValue = T{}) const;
    
    /**
     * @brief Check if configuration key exists
     * @param keyPath Dot-separated key path
     * @return true if key exists, false otherwise
     */
    bool has(const std::string& keyPath) const;
    
    /**
     * @brief Get raw JSON configuration
     * @return Reference to JSON configuration
     */
    const Json::Value& getConfig() const { return config_; }
    
    // Prevent copying
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;

    Json::Value config_;
    std::string configPath_;

    /**
     * @brief Parse key path and navigate JSON
     * @param keyPath Dot-separated key path
     * @return Pointer to JSON value or nullptr if not found
     */
    const Json::Value* findValue(const std::string& keyPath) const;
};
