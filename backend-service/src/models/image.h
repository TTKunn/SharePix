/**
 * @file image.h
 * @brief 图片模型定义
 * @author Knot Team
 * @date 2025-10-07
 */

#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

/**
 * @brief 图片模型类
 *
 * 一条Image记录代表帖子中的一张图片
 * 图片属于某个帖子（Post），通过post_id关联
 */
class Image {
public:
    /**
     * @brief 默认构造函数
     */
    Image();

    /**
     * @brief 带参数的构造函数
     * @param id 物理ID（自增主键）
     * @param imageId 业务逻辑ID（例：IMG_2025Q4_ABC123）
     * @param postId 所属帖子ID
     * @param userId 上传用户ID
     */
    Image(int id, const std::string& imageId, int postId, int userId);
    
    /**
     * @brief 析构函数
     */
    ~Image() = default;
    
    // Getters
    int getId() const { return id_; }
    const std::string& getImageId() const { return imageId_; }
    int getPostId() const { return postId_; }
    int getDisplayOrder() const { return displayOrder_; }
    int getUserId() const { return userId_; }
    const std::string& getUserLogicalId() const { return userLogicalId_; }
    const std::string& getFileUrl() const { return fileUrl_; }
    const std::string& getThumbnailUrl() const { return thumbnailUrl_; }
    long long getFileSize() const { return fileSize_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    const std::string& getMimeType() const { return mimeType_; }
    std::time_t getCreateTime() const { return createTime_; }
    std::time_t getUpdateTime() const { return updateTime_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setImageId(const std::string& imageId) { imageId_ = imageId; }
    void setPostId(int postId) { postId_ = postId; }
    void setDisplayOrder(int displayOrder) { displayOrder_ = displayOrder; }
    void setUserId(int userId) { userId_ = userId; }
    void setUserLogicalId(const std::string& userLogicalId) { userLogicalId_ = userLogicalId; }
    void setFileUrl(const std::string& fileUrl) { fileUrl_ = fileUrl; }
    void setThumbnailUrl(const std::string& thumbnailUrl) { thumbnailUrl_ = thumbnailUrl; }
    void setFileSize(long long fileSize) { fileSize_ = fileSize; }
    void setWidth(int width) { width_ = width; }
    void setHeight(int height) { height_ = height; }
    void setMimeType(const std::string& mimeType) { mimeType_ = mimeType; }
    void setCreateTime(std::time_t createTime) { createTime_ = createTime; }
    void setUpdateTime(std::time_t updateTime) { updateTime_ = updateTime; }
    
    /**
     * @brief 将图片对象转换为JSON
     * @return JSON表示
     */
    Json::Value toJson() const;
    
    /**
     * @brief 从JSON创建图片对象
     * @param j JSON对象
     * @return Image对象
     */
    static Image fromJson(const Json::Value& j);
    
    /**
     * @brief 验证图片数据
     * @return 验证错误信息（如果有效则为空字符串）
     */
    std::string validate() const;

private:
    int id_;                      // 物理ID（自增主键）
    std::string imageId_;         // 业务逻辑ID（例：IMG_2025Q4_ABC123）
    int postId_;                  // 所属帖子ID
    int displayOrder_;            // 显示顺序（0-8）
    int userId_;                  // 上传用户物理ID（内部使用）
    std::string userLogicalId_;   // 上传用户逻辑ID（返回前端）
    std::string fileUrl_;         // 原图URL
    std::string thumbnailUrl_;    // 缩略图URL
    long long fileSize_;          // 文件大小（字节）
    int width_;                   // 图片宽度
    int height_;                  // 图片高度
    std::string mimeType_;        // MIME类型（image/jpeg, image/png, image/webp）
    std::time_t createTime_;      // 创建时间
    std::time_t updateTime_;      // 更新时间
};

