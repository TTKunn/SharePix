/**
 * @file comment_handler.h
 * @brief 评论API处理器定义
 * @author Knot Team
 * @date 2025-10-19
 */

#pragma once

#include "api/base_handler.h"
#include "core/comment_service.h"
#include "core/user_service.h"
#include <memory>

/**
 * @brief 评论API处理器类
 *
 * 负责处理评论相关的HTTP请求
 */
class CommentHandler : public BaseHandler {
public:
    /**
     * @brief 构造函数
     */
    CommentHandler();

    /**
     * @brief 析构函数
     */
    ~CommentHandler() = default;

    /**
     * @brief 注册所有路由
     * @param server HTTP服务器实例
     */
    void registerRoutes(httplib::Server& server);

private:
    std::unique_ptr<CommentService> commentService_;
    std::unique_ptr<UserService> userService_;  // 用于批量查询用户信息

    /**
     * @brief POST /api/v1/posts/:post_id/comments - 创建评论
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleCreateComment(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief GET /api/v1/posts/:post_id/comments - 获取评论列表
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetComments(const httplib::Request& req, httplib::Response& res);

    /**
     * @brief DELETE /api/v1/posts/:post_id/comments/:comment_id - 删除评论
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleDeleteComment(const httplib::Request& req, httplib::Response& res);
};
