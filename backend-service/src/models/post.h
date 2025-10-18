/**
 * @file post.h
 * @brief 帖子模型定义
 * @author Knot Team
 * @date 2025-10-08
 */

#pragma once

#include <string>
#include <vector>
#include <ctime>
#include <json/json.h>
#include "image.h"

/**
 * @brief 帖子审核状态枚举
 */
enum class PostStatus {
    PENDING,     // 待审核
    APPROVED,    // 已通过
    REJECTED     // 已拒绝
};

/**
 * @brief 帖子模型类
 * 
 * 一个帖子可以包含1-9张图片，以及标题、配文等信息
 */
class Post {
public:
    /**
     * @brief 默认构造函数
     */
    Post();
    
    /**
     * @brief 带参数的构造函数
     * @param id 物理ID（自增主键）
     * @param postId 业务逻辑ID（例：POST_2025Q4_ABC123）
     * @param userId 发布用户ID
     * @param title 帖子标题
     */
    Post(int id, const std::string& postId, int userId, const std::string& title);
    
    /**
     * @brief 析构函数
     */
    ~Post() = default;
    
    // Getters
    int getId() const { return id_; }
    const std::string& getPostId() const { return postId_; }
    int getUserId() const { return userId_; }
    const std::string& getUserLogicalId() const { return userLogicalId_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getDescription() const { return description_; }
    int getImageCount() const { return imageCount_; }
    int getLikeCount() const { return likeCount_; }
    int getFavoriteCount() const { return favoriteCount_; }
    int getViewCount() const { return viewCount_; }
    PostStatus getStatus() const { return status_; }
    std::time_t getCreateTime() const { return createTime_; }
    std::time_t getUpdateTime() const { return updateTime_; }
    const std::vector<Image>& getImages() const { return images_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setPostId(const std::string& postId) { postId_ = postId; }
    void setUserId(int userId) { userId_ = userId; }
    void setUserLogicalId(const std::string& userLogicalId) { userLogicalId_ = userLogicalId; }
    void setUsername(const std::string& username) { username_ = username; }
    void setTitle(const std::string& title) { title_ = title; }
    void setDescription(const std::string& description) { description_ = description; }
    void setImageCount(int imageCount) { imageCount_ = imageCount; }
    void setLikeCount(int likeCount) { likeCount_ = likeCount; }
    void setFavoriteCount(int favoriteCount) { favoriteCount_ = favoriteCount; }
    void setViewCount(int viewCount) { viewCount_ = viewCount; }
    void setStatus(PostStatus status) { status_ = status; }
    void setCreateTime(std::time_t createTime) { createTime_ = createTime; }
    void setUpdateTime(std::time_t updateTime) { updateTime_ = updateTime; }
    void setImages(const std::vector<Image>& images) { images_ = images; }
    
    /**
     * @brief 添加图片到帖子
     * @param image 图片对象
     */
    void addImage(const Image& image);
    
    /**
     * @brief 清空所有图片
     */
    void clearImages();
    
    /**
     * @brief 将帖子对象转换为JSON
     * @param includeImages 是否包含图片列表（默认true）
     * @return JSON表示
     */
    Json::Value toJson(bool includeImages = true) const;
    
    /**
     * @brief 从JSON创建帖子对象
     * @param j JSON对象
     * @return Post对象
     */
    static Post fromJson(const Json::Value& j);
    
    /**
     * @brief 验证帖子数据
     * @return 验证错误信息（如果有效则为空字符串）
     */
    std::string validate() const;
    
    /**
     * @brief 检查帖子是否已通过审核
     * @return 如果已通过则返回true，否则返回false
     */
    bool isApproved() const { return status_ == PostStatus::APPROVED; }
    
    /**
     * @brief 获取封面图片（第一张图片）
     * @return 封面图片的缩略图URL，如果没有图片返回空字符串
     */
    std::string getCoverImageUrl() const;
    
    /**
     * @brief 将PostStatus转换为字符串
     * @param status 帖子状态
     * @return 字符串表示
     */
    static std::string statusToString(PostStatus status);
    
    /**
     * @brief 将字符串转换为PostStatus
     * @param statusStr 状态字符串
     * @return PostStatus枚举值
     */
    static PostStatus stringToStatus(const std::string& statusStr);

private:
    int id_;                      // 物理ID（自增主键）
    std::string postId_;          // 业务逻辑ID（例：POST_2025Q4_ABC123）
    int userId_;                  // 发布用户物理ID（内部使用）
    std::string userLogicalId_;   // 发布用户逻辑ID（返回前端）
    std::string username_;        // 发布用户昵称（v2.5.1新增）
    std::string title_;           // 帖子标题
    std::string description_;     // 帖子配文
    int imageCount_;              // 图片数量
    int likeCount_;               // 点赞数
    int favoriteCount_;           // 收藏数
    int viewCount_;               // 浏览数
    PostStatus status_;           // 审核状态
    std::time_t createTime_;      // 创建时间
    std::time_t updateTime_;      // 更新时间
    std::vector<Image> images_;   // 关联的图片列表
};

