/**
 * @file post_handler.h
 * @brief 帖子API处理器定义
 * @author Knot Team
 * @date 2025-10-08
 */

#pragma once

#include "api/base_handler.h"
#include "core/post_service.h"
#include "core/user_service.h"
#include "core/like_service.h"
#include "core/favorite_service.h"
#include <memory>

/**
 * @brief 帖子API处理器类
 * 
 * 负责处理帖子相关的HTTP请求
 */
class PostHandler : public BaseHandler {
public:
    /**
     * @brief 构造函数
     */
    PostHandler();
    
    /**
     * @brief 析构函数
     */
    ~PostHandler() = default;
    
    /**
     * @brief 注册所有路由
     * @param server HTTP服务器实例
     */
    void registerRoutes(httplib::Server& server);

private:
    std::unique_ptr<PostService> postService_;
    std::unique_ptr<UserService> userService_;           // 用户服务（批量查询用户信息）
    std::unique_ptr<LikeService> likeService_;           // 点赞服务（批量查询点赞状态）
    std::unique_ptr<FavoriteService> favoriteService_;   // 收藏服务（批量查询收藏状态）
    
    /**
     * @brief POST /api/v1/posts - 创建帖子
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleCreatePost(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief GET /api/v1/posts/:post_id - 获取帖子详情
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetPostDetail(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief PUT /api/v1/posts/:post_id - 更新帖子
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleUpdatePost(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief DELETE /api/v1/posts/:post_id - 删除帖子
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleDeletePost(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief GET /api/v1/posts - 获取Feed流（最新帖子列表）
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetRecentPosts(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief GET /api/v1/users/:user_id/posts - 获取用户帖子列表
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleGetUserPosts(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief POST /api/v1/posts/:post_id/images - 向帖子添加图片
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleAddImageToPost(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief DELETE /api/v1/posts/:post_id/images/:image_id - 删除帖子中的图片
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleRemoveImageFromPost(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief PUT /api/v1/posts/:post_id/images/order - 调整图片顺序
     * @param req HTTP请求
     * @param res HTTP响应
     */
    void handleReorderImages(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 将Post对象转换为JSON
     * @param post Post对象
     * @param includeImages 是否包含图片列表
     * @return JSON对象
     */
    Json::Value postToJson(const Post& post, bool includeImages = true);
    
    /**
     * @brief 将Image对象转换为JSON
     * @param image Image对象
     * @return JSON对象
     */
    Json::Value imageToJson(const Image& image);

    /**
     * @brief 保存上传的文件到临时目录
     * @param content 文件二进制内容
     * @param filename 原始文件名
     * @param contentType MIME类型
     * @return 保存后的文件路径，失败返回空字符串
     */
    std::string saveUploadedFile(
        const std::string& content,
        const std::string& filename,
        const std::string& contentType
    );
};

