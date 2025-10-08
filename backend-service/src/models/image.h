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
 * @brief 图片审核状态枚举
 */
enum class ImageStatus {
    PENDING,     // 待审核
    APPROVED,    // 已通过
    REJECTED     // 已拒绝
};

/**
 * @brief 图片模型类
 * 
 * 一条Image记录代表一篇完整的图文推文（图片为主 + 配文）
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
     * @param userId 上传用户ID
     * @param title 推文标题
     */
    Image(int id, const std::string& imageId, int userId, const std::string& title);
    
    /**
     * @brief 析构函数
     */
    ~Image() = default;
    
    // Getters
    int getId() const { return id_; }
    const std::string& getImageId() const { return imageId_; }
    int getUserId() const { return userId_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getDescription() const { return description_; }
    const std::string& getFileUrl() const { return fileUrl_; }
    const std::string& getThumbnailUrl() const { return thumbnailUrl_; }
    long long getFileSize() const { return fileSize_; }
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    const std::string& getMimeType() const { return mimeType_; }
    int getLikeCount() const { return likeCount_; }
    int getFavoriteCount() const { return favoriteCount_; }
    int getViewCount() const { return viewCount_; }
    ImageStatus getStatus() const { return status_; }
    std::time_t getCreateTime() const { return createTime_; }
    std::time_t getUpdateTime() const { return updateTime_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setImageId(const std::string& imageId) { imageId_ = imageId; }
    void setUserId(int userId) { userId_ = userId; }
    void setTitle(const std::string& title) { title_ = title; }
    void setDescription(const std::string& description) { description_ = description; }
    void setFileUrl(const std::string& fileUrl) { fileUrl_ = fileUrl; }
    void setThumbnailUrl(const std::string& thumbnailUrl) { thumbnailUrl_ = thumbnailUrl; }
    void setFileSize(long long fileSize) { fileSize_ = fileSize; }
    void setWidth(int width) { width_ = width; }
    void setHeight(int height) { height_ = height; }
    void setMimeType(const std::string& mimeType) { mimeType_ = mimeType; }
    void setLikeCount(int likeCount) { likeCount_ = likeCount; }
    void setFavoriteCount(int favoriteCount) { favoriteCount_ = favoriteCount; }
    void setViewCount(int viewCount) { viewCount_ = viewCount; }
    void setStatus(ImageStatus status) { status_ = status; }
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
    
    /**
     * @brief 检查图片是否已通过审核
     * @return 如果已通过则返回true，否则返回false
     */
    bool isApproved() const { return status_ == ImageStatus::APPROVED; }
    
    /**
     * @brief 将ImageStatus转换为字符串
     * @param status 图片状态
     * @return 字符串表示
     */
    static std::string statusToString(ImageStatus status);
    
    /**
     * @brief 将字符串转换为ImageStatus
     * @param statusStr 状态字符串
     * @return ImageStatus枚举值
     */
    static ImageStatus stringToStatus(const std::string& statusStr);

private:
    int id_;                      // 物理ID（自增主键）
    std::string imageId_;         // 业务逻辑ID（例：IMG_2025Q4_ABC123）
    int userId_;                  // 上传用户ID
    std::string title_;           // 推文标题
    std::string description_;     // 推文配文
    std::string fileUrl_;         // 原图URL
    std::string thumbnailUrl_;    // 缩略图URL
    long long fileSize_;          // 文件大小（字节）
    int width_;                   // 图片宽度
    int height_;                  // 图片高度
    std::string mimeType_;        // MIME类型（image/jpeg, image/png, image/webp）
    int likeCount_;               // 点赞数
    int favoriteCount_;           // 收藏数
    int viewCount_;               // 浏览数
    ImageStatus status_;          // 审核状态
    std::time_t createTime_;      // 创建时间
    std::time_t updateTime_;      // 更新时间
};

