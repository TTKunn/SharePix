/**
 * @file url_helper.cpp
 * @brief URL辅助工具实现
 * @author Knot Team
 * @date 2025-10-14
 */

#include "utils/url_helper.h"
#include "utils/config_manager.h"

// 默认的服务器URL (如果配置文件中未设置server.base_url,则使用此默认值)
const std::string DEFAULT_BASE_URL = "http://43.142.157.145:8080";

// 为路径添加服务器URL前缀
std::string UrlHelper::toFullUrl(const std::string& path) {
    // 如果路径为空,直接返回空字符串
    if (path.empty()) {
        return "";
    }
    
    // 如果已经是完整URL(以http://或https://开头),直接返回原值
    if (path.find("http://") == 0 || path.find("https://") == 0) {
        return path;
    }
    
    // 从配置中获取base_url,如果未配置则使用默认值
    std::string baseUrl = ConfigManager::getInstance()
        .get<std::string>("server.base_url", DEFAULT_BASE_URL);
    
    // 如果base_url被显式设置为空字符串,则返回相对路径(开发环境使用)
    if (baseUrl.empty()) {
        return path;
    }
    
    // 确保base_url不以/结尾
    if (baseUrl.back() == '/') {
        baseUrl.pop_back();
    }
    
    // 确保path以/开头
    if (path.front() != '/') {
        return baseUrl + "/" + path;
    }
    
    return baseUrl + path;
}

// 判断路径是否需要添加前缀
bool UrlHelper::needsPrefix(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    
    // 已经是完整URL,不需要前缀
    if (path.find("http://") == 0 || path.find("https://") == 0) {
        return false;
    }
    
    return true;
}

