/**
 * @file config_manager.cpp
 * @brief Configuration management implementation
 * @author Shared Parking Team
 * @date 2024-01-01
 */

#include "utils/config_manager.h"
#include <fstream>
#include <sstream>
#include <vector>

// Singleton instance
ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

// Load configuration from JSON file
bool ConfigManager::loadConfig(const std::string& configPath) {
    try {
        std::ifstream configFile(configPath);
        if (!configFile.is_open()) {
            return false;
        }

        Json::CharReaderBuilder builder;
        builder["collectComments"] = false;
        std::string errs;
        
        if (!Json::parseFromStream(builder, configFile, &config_, &errs)) {
            return false;
        }

        configPath_ = configPath;
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// Check if configuration key exists
bool ConfigManager::has(const std::string& keyPath) const {
    return findValue(keyPath) != nullptr;
}

// Find value by key path
const Json::Value* ConfigManager::findValue(const std::string& keyPath) const {
    if (keyPath.empty()) {
        return &config_;
    }

    // Split key path by '.'
    std::vector<std::string> keys;
    std::stringstream ss(keyPath);
    std::string key;
    
    while (std::getline(ss, key, '.')) {
        if (!key.empty()) {
            keys.push_back(key);
        }
    }

    // Navigate through JSON tree
    const Json::Value* current = &config_;
    for (const auto& k : keys) {
        if (!current->isObject() || !current->isMember(k)) {
            return nullptr;
        }
        current = &(*current)[k];
    }

    return current;
}

// Template specializations for common types
template<>
std::string ConfigManager::get<std::string>(const std::string& keyPath, 
                                            const std::string& defaultValue) const {
    const Json::Value* value = findValue(keyPath);
    if (value && value->isString()) {
        return value->asString();
    }
    return defaultValue;
}

template<>
int ConfigManager::get<int>(const std::string& keyPath, const int& defaultValue) const {
    const Json::Value* value = findValue(keyPath);
    if (value && value->isInt()) {
        return value->asInt();
    }
    return defaultValue;
}

template<>
bool ConfigManager::get<bool>(const std::string& keyPath, const bool& defaultValue) const {
    const Json::Value* value = findValue(keyPath);
    if (value && value->isBool()) {
        return value->asBool();
    }
    return defaultValue;
}

template<>
double ConfigManager::get<double>(const std::string& keyPath, const double& defaultValue) const {
    const Json::Value* value = findValue(keyPath);
    if (value && value->isDouble()) {
        return value->asDouble();
    }
    return defaultValue;
}

