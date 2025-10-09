/**
 * @file image_repository.h
 * @brief 图片数据访问层定义
 * @author Knot Team
 * @date 2025-10-07
 */

#pragma once

#include "models/image.h"
#include <optional>
#include <vector>
#include <string>

/**
 * @brief 图片数据访问类
 * 
 * 负责图片数据的CRUD操作
 */
class ImageRepository {
public:
    /**
     * @brief 构造函数
     */
    ImageRepository();
    
    /**
     * @brief 析构函数
     */
    ~ImageRepository() = default;
    
    /**
     * @brief 创建图片记录
     * @param image 图片对象
     * @return 成功返回true，失败返回false
     */
    bool createImage(const Image& image);
    
    /**
     * @brief 根据业务ID查找图片
     * @param imageId 业务逻辑ID（例：IMG_2025Q4_ABC123）
     * @return 如果找到返回Image对象，否则返回std::nullopt
     */
    std::optional<Image> findByImageId(const std::string& imageId);
    
    /**
     * @brief 更新图片信息
     * @param image 图片对象
     * @return 成功返回true，失败返回false
     */
    bool updateImage(const Image& image);
    
    /**
     * @brief 删除图片
     * @param imageId 业务逻辑ID
     * @return 成功返回true，失败返回false
     */
    bool deleteImage(const std::string& imageId);
    
    /**
     * @brief 根据帖子ID查找图片列表
     * @param postId 帖子物理ID
     * @return 图片列表（按display_order排序）
     */
    std::vector<Image> findByPostId(int postId);

    /**
     * @brief 批量根据帖子ID查找图片
     * @param postIds 帖子物理ID列表
     * @return 图片列表（按post_id和display_order排序）
     */
    std::vector<Image> findByPostIds(const std::vector<int>& postIds);

    /**
     * @brief 删除帖子的所有图片
     * @param postId 帖子物理ID
     * @return 成功返回true，失败返回false
     */
    bool deleteByPostId(int postId);

    /**
     * @brief 获取帖子的图片数量
     * @param postId 帖子物理ID
     * @return 图片数量
     */
    int getImageCountByPostId(int postId);

private:
    /**
     * @brief 从预编译语句构建Image对象
     * @param stmtPtr MYSQL_STMT指针
     * @return Image对象
     */
    Image buildImageFromStatement(void* stmtPtr);
};

