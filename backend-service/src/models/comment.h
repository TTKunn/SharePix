/**
 * @file comment.h
 * @brief 评论模型定义
 * @author Knot Team
 * @date 2025-10-19
 */

#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

/**
 * @brief 评论模型类
 *
 * 一条Comment记录代表一个用户对一个帖子的评论
 * 评论内容长度限制：1-1000字符
 * 评论不允许编辑，只能删除
 */
class Comment {
public:
    /**
     * @brief 默认构造函数
     */
    Comment();

    /**
     * @brief 带参数的构造函数
     * @param id 物理ID（自增主键）
     * @param commentId 业务逻辑ID
     * @param postId 所属帖子ID
     * @param userId 评论用户ID
     * @param content 评论内容
     */
    Comment(int id, const std::string& commentId, int postId, int userId, const std::string& content);

    /**
     * @brief 析构函数
     */
    ~Comment() = default;

    // Getters
    int getId() const { return id_; }
    const std::string& getCommentId() const { return commentId_; }
    int getPostId() const { return postId_; }
    int getUserId() const { return userId_; }
    const std::string& getContent() const { return content_; }
    std::time_t getCreateTime() const { return createTime_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setCommentId(const std::string& commentId) { commentId_ = commentId; }
    void setPostId(int postId) { postId_ = postId; }
    void setUserId(int userId) { userId_ = userId; }
    void setContent(const std::string& content) { content_ = content; }
    void setCreateTime(std::time_t createTime) { createTime_ = createTime; }

    /**
     * @brief 将评论对象转换为JSON
     * @return JSON表示
     */
    Json::Value toJson() const;

    /**
     * @brief 从JSON创建评论对象
     * @param json JSON对象
     * @return Comment对象
     */
    static Comment fromJson(const Json::Value& json);

    /**
     * @brief 验证评论数据
     * @return 错误信息，空字符串表示验证通过
     */
    std::string validate() const;

private:
    int id_;                      // 物理ID（自增主键）
    std::string commentId_;       // 业务逻辑ID（例：CMT_2025Q4_ABC123）
    int postId_;                  // 所属帖子ID
    int userId_;                  // 评论用户ID
    std::string content_;         // 评论内容（最多1000字符）
    std::time_t createTime_;      // 创建时间
};
