/**
 * @file follow_handler.h
 * @brief 关注API处理器定义
 * @author Knot Team
 * @date 2025-10-16
 */

#pragma once

#include "api/base_handler.h"
#include "core/follow_service.h"
#include <memory>

/**
 * @brief 关注API处理器类
 *
 * 负责处理关注相关的HTTP请求
 */
class FollowHandler : public BaseHandler {
public:
    /**
     * @brief 构造函数
     */
    FollowHandler();

    /**
     * @brief 析构函数
     */
    ~FollowHandler() = default;

    /**
     * @brief 注册所有路由
     * @param server HTTP服务器实例
     */
    void registerRoutes(httplib::Server& server);

private:
    std::unique_ptr<FollowService> followService_;

    /**
     * @brief POST /api/v1/users/:user_id/follow - 关注用户
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleFollow(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief DELETE /api/v1/users/:user_id/follow - 取消关注
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleUnfollow(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/users/:user_id/follow/status - 检查关注关系
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleCheckFollowStatus(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/users/:user_id/following - 获取关注列表
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetFollowingList(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/users/:user_id/followers - 获取粉丝列表
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetFollowerList(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/users/:user_id/mutual-follows - 获取互关列表
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetMutualFollows(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/users/:user_id/stats - 获取用户统计信息
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetUserStats(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief POST /api/v1/users/follow/batch-status - 批量检查关注关系
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleBatchCheckFollowStatus(const httplib::Request& req, httplib::Response& res);
};



