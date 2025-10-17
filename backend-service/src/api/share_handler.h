/**
 * @file share_handler.h
 * @brief 分享链接API处理器
 * @author Knot Development Team
 * @date 2025-10-16
 */

#pragma once

#include "httplib.h"
#include <memory>
#include <json/json.h>

/**
 * @brief 分享链接API处理器
 * 
 * 提供分享链接相关的HTTP接口
 */
class ShareHandler {
public:
    /**
     * @brief 注册路由
     * @param server HTTP服务器实例
     */
    void registerRoutes(httplib::Server& server);

private:
    /**
     * @brief 创建帖子分享链接
     * 
     * POST /api/v1/posts/:post_id/share
     * 需要JWT认证
     * 
     * Request Body:
     * {
     *   "expire_days": 0  // 可选，0表示永不过期
     * }
     * 
     * Response:
     * {
     *   "success": true,
     *   "message": "分享链接创建成功",
     *   "data": {
     *     "short_code": "kHr7ZS9a",
     *     "short_url": "http://8.138.115.164:8080/s/kHr7ZS9a",
     *     "deep_links": {
     *       "ios": "https://knot.app/post/POST_2025Q4_ABC123",
     *       "android": "https://knot.app/post/POST_2025Q4_ABC123",
     *       "harmonyos_scheme": "knot://post/POST_2025Q4_ABC123",
     *       "harmonyos_app_link": "https://knot.app/post/POST_2025Q4_ABC123"
     *     },
     *     "is_reused": false
     *   }
     * }
     */
    void handleCreateShareLink(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 解析分享链接获取帖子信息
     * 
     * GET /api/v1/share/:code
     * 公开接口，无需认证
     * 
     * Response:
     * {
     *   "success": true,
     *   "message": "解析成功",
     *   "data": {
     *     "post": { ... },  // 完整的帖子信息
     *     "expired": false
     *   }
     * }
     */
    void handleResolveShareLink(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 构造统一的JSON响应
     * @param success 是否成功
     * @param message 消息
     * @param data 数据对象（可选）
     * @return JSON字符串
     */
    std::string buildResponse(bool success, const std::string& message, 
                              const Json::Value& data = Json::Value::null);
};

