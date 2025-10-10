/**
 * @file like.h
 * @brief 点赞模型定义
 * @author Knot Team
 * @date 2025-10-10
 */

#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

/**
 * @brief 点赞模型类
 *
 * 一条Like记录代表一个用户对一个帖子的点赞行为
 * 通过唯一约束(user_id, post_id)保证幂等性
 */
class Like {
public:
    /**
     * @brief 默认构造函数
     */
    Like();

    /**
     * @brief 带参数的构造函数
     * @param id 物理ID（自增主键）
     * @param userId 点赞用户ID
     * @param postId 被点赞帖子ID
     */
    Like(int id, int userId, int postId);

    /**
     * @brief 析构函数
     */
    ~Like() = default;

    // Getters
    int getId() const { return id_; }
    int getUserId() const { return userId_; }
    int getPostId() const { return postId_; }
    std::time_t getCreateTime() const { return createTime_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setUserId(int userId) { userId_ = userId; }
    void setPostId(int postId) { postId_ = postId; }
    void setCreateTime(std::time_t createTime) { createTime_ = createTime; }

    /**
     * @brief 将点赞对象转换为JSON
     * @return JSON表示
     */
    Json::Value toJson() const;

    /**
     * @brief 从JSON创建点赞对象
     * @param json JSON对象
     * @return Like对象
     */
    static Like fromJson(const Json::Value& json);

private:
    int id_;                      // 物理ID（自增主键）
    int userId_;                  // 点赞用户ID
    int postId_;                  // 被点赞帖子ID
    std::time_t createTime_;      // 点赞时间
};
