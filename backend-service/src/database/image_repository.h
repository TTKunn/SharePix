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
     * @brief 获取最新图片列表（时间倒序）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return 图片列表
     */
    std::vector<Image> getRecentImages(int page, int pageSize);
    
    /**
     * @brief 根据用户ID查找图片列表
     * @param userId 用户ID
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return 图片列表
     */
    std::vector<Image> findByUserId(int userId, int page, int pageSize);
    
    /**
     * @brief 增加浏览数
     * @param imageId 业务逻辑ID
     * @return 成功返回true，失败返回false
     */
    bool incrementViewCount(const std::string& imageId);
    
    /**
     * @brief 获取图片总数
     * @return 图片总数
     */
    int getTotalCount();

private:
    /**
     * @brief 从预编译语句构建Image对象
     * @param stmtPtr MYSQL_STMT指针
     * @return Image对象
     */
    Image buildImageFromStatement(void* stmtPtr);
};

