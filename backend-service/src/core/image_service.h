/**
 * @file image_service.h
 * @brief 图片服务 - 业务逻辑层
 * @author Knot Team
 * @date 2025-10-12
 */

#pragma once

#include "models/image.h"
#include <optional>
#include <string>
#include <vector>
#include <map>
#include <memory>

// 前向声明
class ImageRepository;

// ============================================================================
// Result结构体定义
// ============================================================================

/**
 * @brief 图片上传结果结构体
 */
struct ImageUploadResult {
    bool success;
    std::string message;
    Image image;

    ImageUploadResult() : success(false), message("") {}
};

/**
 * @brief 图片查询结果结构体
 */
struct ImageQueryResult {
    bool success;
    std::string message;
    std::vector<Image> images;
    int total;
    int page;
    int pageSize;

    ImageQueryResult() : success(false), message(""), total(0), page(0), pageSize(0) {}
};

/**
 * @brief 图片服务类
 *
 * 负责图片相关的业务逻辑：
 * - 图片上传和处理
 * - 图片查询
 * - 图片删除
 * - 图片批量操作
 */
class ImageService {
public:
    /**
     * @brief 构造函数
     */
    ImageService();

    /**
     * @brief 析构函数
     */
    ~ImageService();

    /**
     * @brief 上传并处理图片
     *
     * 业务流程：
     * 1. 验证图片格式
     * 2. 图片压缩（质量85%）
     * 3. 生成缩略图（300x300）
     * 4. 生成图片ID（IMG_2025Q4_XXXXXX）
     * 5. 保存图片记录到数据库
     *
     * @param userId 上传用户ID
     * @param tempPath 上传的临时文件路径
     * @param title 图片标题
     * @param description 图片描述
     * @param tags 标签列表
     * @return ImageUploadResult 上传结果
     */
    ImageUploadResult uploadImage(
        int userId,
        const std::string& tempPath,
        const std::string& title,
        const std::string& description,
        const std::vector<std::string>& tags
    );

    /**
     * @brief 获取最新图片列表（分页）
     *
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return ImageQueryResult 查询结果
     */
    ImageQueryResult getRecentImages(int page, int pageSize);

    /**
     * @brief 根据图片ID获取图片详情
     *
     * @param imageId 图片业务ID（IMG_XXXXXX）
     * @return 成功返回Image对象，失败返回nullopt
     */
    std::optional<Image> getImageDetail(const std::string& imageId);

    /**
     * @brief 更新图片标题和描述
     *
     * @param imageId 图片业务ID
     * @param userId 操作用户ID（用于权限验证）
     * @param title 新标题
     * @param description 新描述
     * @return true 更新成功，false 更新失败
     */
    bool updateImageText(const std::string& imageId, int userId, const std::string& title, const std::string& description);

    /**
     * @brief 获取用户上传的图片列表（分页）
     *
     * @param userId 用户ID
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return ImageQueryResult 查询结果
     */
    ImageQueryResult getUserImages(int userId, int page, int pageSize);

    /**
     * @brief 删除图片
     *
     * @param imageId 图片业务ID
     * @param userId 操作用户ID（用于权限验证）
     * @return true 删除成功，false 删除失败
     */
    bool deleteImage(const std::string& imageId, int userId);

    /**
     * @brief 获取帖子的所有图片
     *
     * @param postId 帖子物理ID
     * @return 图片列表（按display_order排序）
     */
    std::vector<Image> getImagesByPostId(int postId);

    /**
     * @brief 批量获取多个帖子的图片
     *
     * 用于Feed流等场景，避免N+1查询问题
     *
     * @param postIds 帖子物理ID列表
     * @return Map<postId, 图片列表>
     */
    std::map<int, std::vector<Image>> getImagesByPostIds(const std::vector<int>& postIds);

    /**
     * @brief 删除帖子的所有图片
     *
     * @param postId 帖子物理ID
     * @return true 删除成功，false 删除失败
     */
    bool deleteImagesByPostId(int postId);

    /**
     * @brief 更新图片显示顺序
     *
     * @param imageId 图片业务ID
     * @param newOrder 新的显示顺序
     * @return true 更新成功，false 更新失败
     */
    bool updateDisplayOrder(const std::string& imageId, int newOrder);

private:
    std::unique_ptr<ImageRepository> imageRepo_;

    /**
     * @brief 生成唯一图片ID
     *
     * 格式：IMG_2025Q4_XXXXXX（6位随机字母数字）
     *
     * @return 图片业务ID
     */
    std::string generateImageId();
};
