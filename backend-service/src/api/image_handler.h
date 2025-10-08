/**
 * @file image_handler.h
 * @brief 图片API处理器
 * @author Knot Team
 * @date 2025-10-07
 */

#pragma once

#include "httplib.h"
#include <json/json.h>
#include <memory>

// 前向声明
class ImageService;

/**
 * @brief 图片API处理器类
 * 
 * 处理所有图片相关的HTTP请求，包括上传、查询、删除等
 */
class ImageHandler {
public:
    /**
     * @brief 构造函数
     */
    ImageHandler();
    
    /**
     * @brief 析构函数
     */
    ~ImageHandler();
    
    /**
     * @brief 注册路由到HTTP服务器
     * 
     * @param server HTTP服务器实例
     */
    void registerRoutes(httplib::Server& server);
    
private:
    /**
     * @brief 处理图片上传请求
     * 
     * POST /api/v1/images
     */
    void handleUpload(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理获取最新图片列表请求
     * 
     * GET /api/v1/images?page=1&page_size=20
     */
    void handleGetRecent(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理获取图片详情请求
     * 
     * GET /api/v1/images/:id
     */
    void handleGetById(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理更新图文配文请求
     * 
     * PUT /api/v1/images/:id
     */
    void handleUpdate(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理删除图片请求
     * 
     * DELETE /api/v1/images/:id
     */
    void handleDelete(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理获取用户图片列表请求
     * 
     * GET /api/v1/users/:id/images?page=1&page_size=20
     */
    void handleGetUserImages(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 从JWT令牌中获取用户ID
     * 
     * @param token JWT令牌
     * @return int 用户ID（失败返回0）
     */
    int getUserIdFromToken(const std::string& token);
    
    /**
     * @brief 从请求中提取JWT令牌
     * 
     * @param req HTTP请求对象
     * @return std::string JWT令牌（如果存在）
     */
    std::string extractToken(const httplib::Request& req);
    
    /**
     * @brief 解析JSON请求体
     * 
     * @param body 请求体字符串
     * @param jsonOut 输出的JSON对象
     * @return true 解析成功，false 解析失败
     */
    bool parseJsonBody(const std::string& body, Json::Value& jsonOut);
    
    /**
     * @brief 发送JSON响应
     * 
     * @param res HTTP响应对象
     * @param statusCode HTTP状态码
     * @param success 是否成功
     * @param message 消息内容
     * @param data 数据对象（可选）
     */
    void sendJsonResponse(httplib::Response& res, 
                         int statusCode,
                         bool success, 
                         const std::string& message,
                         const Json::Value& data = Json::Value::null);
    
    std::unique_ptr<ImageService> imageService_;
};

