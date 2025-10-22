/**
 * @file share_handler.h
 * @brief 分享API处理器
 * @author Claude Code Assistant
 * @date 2025-10-22
 * @version v2.10.0
 */

#pragma once

#include <httplib.h>
#include <memory>
#include <json/json.h>

// 前向声明
class ShareService;
class JWTManager;

/**
 * @brief 分享API处理器类
 *
 * 负责处理分享相关的HTTP请求：
 * - POST /api/v1/shares/posts - 创建分享记录
 * - GET /api/v1/shares/received - 获取收到的分享列表
 * - GET /api/v1/shares/sent - 获取发出的分享列表
 * - DELETE /api/v1/shares/:id - 删除分享记录
 */
class ShareHandler {
public:
    /**
     * @brief 构造函数
     */
    ShareHandler();

    /**
     * @brief 析构函数
     */
    ~ShareHandler();

    /**
     * @brief 注册路由
     * @param server HTTP服务器对象
     */
    void registerRoutes(httplib::Server& server);

private:
    std::unique_ptr<ShareService> shareService_;
    std::unique_ptr<JWTManager> jwtManager_;

    /**
     * @brief POST /api/v1/shares/posts - 创建分享记录
     * @param req HTTP请求
     * @param res HTTP响应
     *
     * 请求体（JSON）：
     * {
     *   "post_id": 123,           // 帖子物理ID
     *   "receiver_id": 456,       // 接收者物理ID
     *   "share_message": "推荐给你"  // 分享附言（可选）
     * }
     *
     * 响应（201 Created）：
     * {
     *   "success": true,
     *   "message": "分享成功",
     *   "data": {
     *     "share_id": "SHR_2025Q4_ABC123",
     *     "create_time": 1697952000
     *   },
     *   "timestamp": 1697952000
     * }
     */
    void handleCreateShare(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/shares/received - 获取收到的分享列表
     * @param req HTTP请求
     * @param res HTTP响应
     *
     * 请求参数：
     * - page: 页码（默认1）
     * - page_size: 每页数量（默认20，最大50）
     *
     * 响应（200 OK）：
     * {
     *   "success": true,
     *   "message": "查询成功",
     *   "data": {
     *     "shares": [{
     *       "share_id": "SHR_2025Q4_ABC123",
     *       "share_message": "推荐给你",
     *       "create_time": 1697952000,
     *       "post": {
     *         "id": 123,
     *         "post_id": "POST_2025Q4_DEF456",
     *         "title": "美食探店",
     *         "description": "今天去了...",
     *         "cover_image": "/uploads/...",
     *         "like_count": 100,
     *         "favorite_count": 50
     *       },
     *       "sender": {
     *         "id": 789,
     *         "user_id": "USR_2025Q4_GHI789",
     *         "username": "张三",
     *         "avatar_url": "/uploads/...",
     *         "bio": "热爱美食"
     *       }
     *     }],
     *     "total": 15,
     *     "page": 1,
     *     "page_size": 20,
     *     "has_more": false
     *   },
     *   "timestamp": 1697952000
     * }
     */
    void handleGetReceivedShares(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/shares/sent - 获取发出的分享列表
     * @param req HTTP请求
     * @param res HTTP响应
     *
     * 请求参数和响应格式与handleGetReceivedShares相同
     */
    void handleGetSentShares(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief DELETE /api/v1/shares/:id - 删除分享记录
     * @param req HTTP请求
     * @param res HTTP响应
     *
     * 路径参数：
     * - id: 分享物理ID
     *
     * 响应（200 OK）：
     * {
     *   "success": true,
     *   "message": "删除成功",
     *   "timestamp": 1697952000
     * }
     */
    void handleDeleteShare(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief 从JWT令牌中提取用户ID
     * @param req HTTP请求
     * @return 用户ID（失败返回0）
     */
    int extractUserIdFromToken(const httplib::Request& req);

    /**
     * @brief 构建JSON响应
     * @param success 是否成功
     * @param message 消息
     * @param data 数据对象（可选）
     * @return JSON字符串
     */
    std::string buildJsonResponse(bool success, const std::string& message, const Json::Value& data = Json::Value::null);

    /**
     * @brief 发送错误响应
     * @param res HTTP响应对象
     * @param statusCode HTTP状态码
     * @param message 错误消息
     */
    void sendErrorResponse(httplib::Response& res, int statusCode, const std::string& message);
};
