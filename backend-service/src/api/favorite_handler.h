/**
 * @file favorite_handler.h
 * @brief 收藏API处理器定义
 * @author Knot Team
 * @date 2025-10-10
 */

#pragma once

#include "api/base_handler.h"
#include "core/favorite_service.h"
#include <memory>

/**
 * @brief 收藏API处理器类
 *
 * 负责处理收藏相关的HTTP请求
 */
class FavoriteHandler : public BaseHandler {
public:
    /**
     * @brief 构造函数
     */
    FavoriteHandler();

    /**
     * @brief 析构函数
     */
    ~FavoriteHandler() = default;

    /**
     * @brief 注册所有路由
     * @param server HTTP服务器实例
     */
    void registerRoutes(httplib::Server& server);

private:
    std::unique_ptr<FavoriteService> favoriteService_;

    /**
     * @brief POST /api/v1/posts/:post_id/favorite - 收藏帖子
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleFavorite(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief DELETE /api/v1/posts/:post_id/favorite - 取消收藏
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleUnfavorite(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/posts/:post_id/favorite/status - 查询收藏状态
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetFavoriteStatus(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/users/favorites - 获取用户收藏列表
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetUserFavorites(const httplib::Request& req, httplib::Response& res);
};
