/**
 * @file favorite.h
 * @brief 收藏模型定义
 * @author Knot Team
 * @date 2025-10-10
 */

#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

/**
 * @brief 收藏模型类
 *
 * 一条Favorite记录代表一个用户对一个帖子的收藏行为
 * 通过唯一约束(user_id, post_id)保证幂等性
 */
class Favorite {
public:
    /**
     * @brief 默认构造函数
     */
    Favorite();

    /**
     * @brief 带参数的构造函数
     * @param id 物理ID（自增主键）
     * @param userId 收藏用户ID
     * @param postId 被收藏帖子ID
     */
    Favorite(int id, int userId, int postId);

    /**
     * @brief 析构函数
     */
    ~Favorite() = default;

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
     * @brief 将收藏对象转换为JSON
     * @return JSON表示
     */
    Json::Value toJson() const;

    /**
     * @brief 从JSON创建收藏对象
     * @param json JSON对象
     * @return Favorite对象
     */
    static Favorite fromJson(const Json::Value& json);

private:
    int id_;                      // 物理ID（自增主键）
    int userId_;                  // 收藏用户ID
    int postId_;                  // 被收藏帖子ID
    std::time_t createTime_;      // 收藏时间
};
