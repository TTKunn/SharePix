/**
 * @file post_service.h
 * @brief 帖子服务 - 业务逻辑层
 * @author Knot Team
 * @date 2025-10-12
 */

#pragma once

#include "models/post.h"
#include <optional>
#include <string>
#include <vector>
#include <memory>

// 前向声明
class PostRepository;
class ImageService;
class TagRepository;
class ImageRepository;

// ============================================================================
// Result结构体定义
// ============================================================================

/**
 * @brief 帖子创建结果结构体
 */
struct PostCreateResult {
    bool success;
    std::string message;
    Post post;

    PostCreateResult() : success(false), message("") {}
};

/**
 * @brief 帖子查询结果结构体
 */
struct PostQueryResult {
    bool success;
    std::string message;
    std::vector<Post> posts;
    int total;
    int page;
    int pageSize;

    PostQueryResult() : success(false), message(""), total(0), page(0), pageSize(0) {}
};

/**
 * @brief 帖子服务类
 *
 * 负责帖子相关的业务逻辑：
 * - 创建帖子（1-9张图片）
 * - 查询帖子
 * - 更新帖子
 * - 删除帖子
 * - Feed流推荐
 */
class PostService {
public:
    /**
     * @brief 构造函数
     */
    PostService();

    /**
     * @brief 析构函数
     */
    ~PostService();

    /**
     * @brief 创建帖子
     *
     * 业务流程：
     * 1. 验证帖子信息（标题、配文）
     * 2. 验证图片数量（1-9张）
     * 3. 生成帖子ID（POST_2025Q4_XXXXXX）
     * 4. 开启事务
     * 5. 创建帖子记录
     * 6. 上传并处理图片
     * 7. 关联标签（如果有）
     * 8. 提交事务
     *
     * @param userId 创建用户ID
     * @param title 帖子标题
     * @param description 帖子描述
     * @param imagePaths 上传的图片临时文件路径列表（1-9个）
     * @param tags 标签列表
     * @return PostCreateResult 创建结果
     */
    PostCreateResult createPost(
        int userId,
        const std::string& title,
        const std::string& description,
        const std::vector<std::string>& imagePaths,
        const std::vector<std::string>& tags
    );

    /**
     * @brief 获取帖子详情
     *
     * @param postId 帖子业务ID（POST_XXXXXX）
     * @param includeImages 是否包含图片列表
     * @return 成功返回Post对象，失败返回nullopt
     */
    std::optional<Post> getPostDetail(const std::string& postId, bool includeImages = true);

    /**
     * @brief 更新帖子信息
     *
     * 只允许修改标题和配文，不允许修改图片
     *
     * @param postId 帖子业务ID
     * @param userId 操作用户ID（用于权限验证）
     * @param title 新标题
     * @param description 新描述
     * @return true 更新成功，false 更新失败
     */
    bool updatePost(const std::string& postId, int userId, const std::string& title, const std::string& description);

    /**
     * @brief 删除帖子
     *
     * 级联删除帖子的所有图片和标签关联
     *
     * @param postId 帖子业务ID
     * @param userId 操作用户ID（用于权限验证）
     * @return true 删除成功，false 删除失败
     */
    bool deletePost(const std::string& postId, int userId);

    /**
     * @brief 获取Feed流（最新帖子列表）
     *
     * 按创建时间倒序排列，支持分页
     *
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @param includeImages 是否包含图片列表
     * @return PostQueryResult 查询结果
     */
    PostQueryResult getRecentPosts(int page, int pageSize, bool includeImages = true);

    /**
     * @brief 获取用户的帖子列表
     *
     * @param userId 用户ID（物理ID）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @param includeImages 是否包含图片列表
     * @return PostQueryResult 查询结果
     */
    PostQueryResult getUserPosts(int userId, int page, int pageSize, bool includeImages = true);

    /**
     * @brief 向帖子添加图片
     *
     * @param postId 帖子业务ID
     * @param userId 操作用户ID（用于权限验证）
     * @param imagePath 图片临时文件路径
     * @return true 添加成功，false 添加失败
     */
    bool addImageToPost(const std::string& postId, int userId, const std::string& imagePath);

    /**
     * @brief 从帖子删除图片
     *
     * @param postId 帖子业务ID
     * @param imageId 图片业务ID
     * @param userId 操作用户ID（用于权限验证）
     * @return true 删除成功，false 删除失败
     */
    bool removeImageFromPost(const std::string& postId, const std::string& imageId, int userId);

    /**
     * @brief 调整图片顺序
     *
     * @param postId 帖子业务ID
     * @param userId 操作用户ID（用于权限验证）
     * @param imageIds 图片业务ID列表（新的顺序）
     * @return true 调整成功，false 调整失败
     */
    bool reorderImages(const std::string& postId, int userId, const std::vector<std::string>& imageIds);

    /**
     * @brief 增加浏览数
     *
     * @param postId 帖子业务ID
     * @return true 增加成功，false 增加失败
     */
    bool incrementViewCount(const std::string& postId);

    /**
     * @brief 更新帖子的图片数量
     *
     * @param postId 帖子业务ID
     * @param newCount 新的图片数量
     * @return true 更新成功，false 更新失败
     */
    bool updateImageCount(const std::string& postId, int newCount);

    /**
     * @brief 重新计算并更新帖子的图片数量
     *
     * 根据images表中的实际记录数重新计算图片数量并更新posts表
     *
     * @param postId 帖子业务ID
     * @return true 更新成功，false 更新失败
     */
    bool recalculateImageCount(const std::string& postId);

private:
    std::unique_ptr<PostRepository> postRepo_;
    std::unique_ptr<ImageService> imageService_;
    std::unique_ptr<TagRepository> tagRepo_;
    std::unique_ptr<ImageRepository> imageRepo_;

    /**
     * @brief 生成唯一帖子ID
     *
     * 格式：POST_2025Q4_XXXXXX（6位随机字母数字）
     *
     * @return 帖子业务ID
     */
    std::string generatePostId();

    /**
     * @brief 验证帖子信息
     *
     * 检查：
     * - 标题：1-255字符
     * - 配文：最多5000字符（可选）
     * - 用户ID：有效
     *
     * @param post 帖子对象
     * @return true 验证通过，false 验证失败
     */
    bool validatePost(const Post& post);

    /**
     * @brief 验证图片数量
     *
     * @param imageCount 图片数量
     * @return true 验证通过（1-9张），false 验证失败
     */
    bool validateImageCount(int imageCount);

    /**
     * @brief 检查用户是否拥有帖子
     *
     * @param postId 帖子业务ID
     * @param userId 用户ID
     * @return true 拥有，false 不拥有
     */
    bool checkOwnership(const std::string& postId, int userId);
};
