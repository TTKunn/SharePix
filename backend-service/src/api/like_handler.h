/**
 * @file like_handler.h
 * @brief 点赞API处理器定义
 * @author Knot Team
 * @date 2025-10-10
 */

#pragma once

#include "api/base_handler.h"
#include "core/like_service.h"
#include <memory>

/**
 * @brief 点赞API处理器类
 *
 * 负责处理点赞相关的HTTP请求
 */
class LikeHandler : public BaseHandler {
public:
    /**
     * @brief 构造函数
     */
    LikeHandler();

    /**
     * @brief 析构函数
     */
    ~LikeHandler() = default;

    /**
     * @brief 注册所有路由
     * @param server HTTP服务器实例
     */
    void registerRoutes(httplib::Server& server);

private:
    std::unique_ptr<LikeService> likeService_;

    /**
     * @brief POST /api/v1/posts/:post_id/like - 点赞帖子
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleLike(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief DELETE /api/v1/posts/:post_id/like - 取消点赞
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleUnlike(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/posts/:post_id/like/status - 查询点赞状态
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetLikeStatus(const httplib::Request& req, httplib::Response& res);
};
