/**
 * @file base_handler.h
 * @brief API处理器基类定义
 * @author Knot Team
 * @date 2025-10-08
 */

#pragma once

#include <string>
#include <json/json.h>
#include "httplib.h"

/**
 * @brief API处理器基类
 * 
 * 提供通用的JWT验证、JSON解析、响应发送等功能
 * 所有Handler应继承此类
 */
class BaseHandler {
public:
    /**
     * @brief 构造函数
     */
    BaseHandler() = default;
    
    /**
     * @brief 析构函数
     */
    virtual ~BaseHandler() = default;

protected:
    /**
     * @brief 从请求中提取JWT令牌
     * @param req HTTP请求对象
     * @return JWT令牌字符串，如果不存在返回空字符串
     */
    std::string extractToken(const httplib::Request& req);
    
    /**
     * @brief 从JWT令牌中获取用户ID
     * @param token JWT令牌
     * @return 用户ID，如果令牌无效返回0
     */
    int getUserIdFromToken(const std::string& token);
    
    /**
     * @brief 验证JWT令牌并获取用户ID（组合方法）
     * @param req HTTP请求对象
     * @param userId 输出参数，用户ID
     * @return 成功返回true，失败返回false
     */
    bool authenticateRequest(const httplib::Request& req, int& userId);
    
    /**
     * @brief 解析JSON请求体
     * @param body 请求体字符串
     * @param jsonOut 输出参数，解析后的JSON对象
     * @return 成功返回true，失败返回false
     */
    bool parseJsonBody(const std::string& body, Json::Value& jsonOut);
    
    /**
     * @brief 发送JSON响应
     * @param res HTTP响应对象
     * @param statusCode HTTP状态码
     * @param success 操作是否成功
     * @param message 响应消息
     * @param data 响应数据（可选）
     */
    void sendJsonResponse(httplib::Response& res,
                         int statusCode,
                         bool success,
                         const std::string& message,
                         const Json::Value& data = Json::Value::null);
    
    /**
     * @brief 发送错误响应（快捷方法）
     * @param res HTTP响应对象
     * @param statusCode HTTP状态码
     * @param message 错误消息
     */
    void sendErrorResponse(httplib::Response& res,
                          int statusCode,
                          const std::string& message);
    
    /**
     * @brief 发送成功响应（快捷方法）
     * @param res HTTP响应对象
     * @param message 成功消息
     * @param data 响应数据（可选）
     */
    void sendSuccessResponse(httplib::Response& res,
                            const std::string& message,
                            const Json::Value& data = Json::Value::null);
    
    /**
     * @brief 验证分页参数
     * @param page 页码
     * @param pageSize 每页数量
     * @param maxPageSize 最大每页数量（默认100）
     * @return 成功返回true，失败返回false
     */
    bool validatePagination(int page, int pageSize, int maxPageSize = 100);
    
    /**
     * @brief 从查询参数中获取整数值
     * @param req HTTP请求对象
     * @param key 参数名
     * @param defaultValue 默认值
     * @return 参数值
     */
    int getQueryParamInt(const httplib::Request& req, 
                        const std::string& key, 
                        int defaultValue);
    
    /**
     * @brief 从查询参数中获取字符串值
     * @param req HTTP请求对象
     * @param key 参数名
     * @param defaultValue 默认值
     * @return 参数值
     */
    std::string getQueryParamString(const httplib::Request& req,
                                   const std::string& key,
                                   const std::string& defaultValue = "");
};

