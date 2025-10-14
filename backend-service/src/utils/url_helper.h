/**
 * @file url_helper.h
 * @brief URL辅助工具
 * @author Knot Team
 * @date 2025-10-14
 */

#pragma once

#include <string>

/**
 * @brief URL辅助工具类
 * 
 * 用于统一处理图片、文件等资源路径的URL前缀添加
 */
class UrlHelper {
public:
    /**
     * @brief 为路径添加服务器URL前缀
     * 
     * 自动从配置文件读取server.base_url,如果未配置则使用默认值 http://8.138.115.164:8080
     * 
     * @param path 相对路径 (如 /uploads/images/xxx.jpg)
     * @return 完整URL (如 http://8.138.115.164:8080/uploads/images/xxx.jpg)
     * 
     * 特殊情况处理:
     * - 如果path为空,返回空字符串
     * - 如果path已经是完整URL(以http://或https://开头),直接返回原值
     * - 如果配置中base_url为空字符串,返回相对路径(用于开发环境)
     */
    static std::string toFullUrl(const std::string& path);
    
    /**
     * @brief 判断路径是否需要添加前缀
     * @param path 路径字符串
     * @return true表示需要添加前缀, false表示不需要
     */
    static bool needsPrefix(const std::string& path);
};

