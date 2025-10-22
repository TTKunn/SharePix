#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

/**
 * @file share.h
 * @brief 分享记录模型类
 * @version v2.10.0
 * @date 2025-10-22
 *
 * 设计说明：
 * - 仅存储ID字段，不存储冗余的帖子和用户信息
 * - 帖子和用户详细信息通过Service层的批量查询获取
 * - 保证数据始终最新，无需维护冗余字段同步
 */

class Share {
public:
    /**
     * @brief 默认构造函数
     */
    Share();

    /**
     * @brief 参数化构造函数
     * @param id 物理ID
     * @param shareId 业务ID
     * @param postId 帖子ID
     * @param senderId 分享者ID
     * @param receiverId 接收者ID
     */
    Share(int id, const std::string& shareId, int postId, int senderId, int receiverId);

    /**
     * @brief 析构函数
     */
    ~Share() = default;

    // Getters
    int getId() const { return id_; }
    const std::string& getShareId() const { return shareId_; }
    int getPostId() const { return postId_; }
    int getSenderId() const { return senderId_; }
    int getReceiverId() const { return receiverId_; }
    const std::string& getShareMessage() const { return shareMessage_; }
    std::time_t getCreateTime() const { return createTime_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setShareId(const std::string& shareId) { shareId_ = shareId; }
    void setPostId(int postId) { postId_ = postId; }
    void setSenderId(int senderId) { senderId_ = senderId; }
    void setReceiverId(int receiverId) { receiverId_ = receiverId; }
    void setShareMessage(const std::string& message) { shareMessage_ = message; }
    void setCreateTime(std::time_t createTime) { createTime_ = createTime; }

    /**
     * @brief 转换为JSON对象（仅包含基础字段）
     * @return JSON对象
     * @note 完整的响应（包含帖子和用户信息）在Service层组装
     */
    Json::Value toJson() const;

    /**
     * @brief 从JSON对象创建Share实例
     * @param json JSON对象
     * @return Share实例
     */
    static Share fromJson(const Json::Value& json);

    /**
     * @brief 验证分享数据的有效性
     * @return 错误消息，如果验证通过则返回空字符串
     */
    std::string validate() const;

private:
    int id_;                     // 物理ID
    std::string shareId_;        // 业务ID（格式：SHR_2025Q4_ABC123）
    int postId_;                 // 帖子物理ID（关联posts表）
    int senderId_;               // 分享者物理ID（关联users表）
    int receiverId_;             // 接收者物理ID（关联users表）
    std::string shareMessage_;   // 分享附言（可选，最多500字符）
    std::time_t createTime_;     // 创建时间

    // v2.10.0设计：移除冗余字段
    // 不再存储：postTitle_, postCoverImage_, postDescription_,
    //           senderUsername_, senderAvatar_
    // 原因：通过PostService::batchGetPosts()和UserService::batchGetUsers()
    //       在Service层动态获取，保证数据一致性
};
