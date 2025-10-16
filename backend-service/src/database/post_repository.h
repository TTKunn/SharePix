/**
 * @file post_repository.h
 * @brief 帖子数据访问层定义
 * @author Knot Team
 * @date 2025-10-08
 */

#pragma once

#include "models/post.h"
#include <mysql/mysql.h>
#include <optional>
#include <vector>
#include <string>

/**
 * @brief 帖子数据访问类
 * 
 * 负责帖子数据的CRUD操作，包括关联的图片
 */
class PostRepository {
public:
    /**
     * @brief 构造函数
     */
    PostRepository();
    
    /**
     * @brief 析构函数
     */
    ~PostRepository() = default;
    
    /**
     * @brief 创建帖子记录（不包含图片）
     * @param post 帖子对象
     * @return 成功返回true，失败返回false
     */
    bool createPost(Post& post);
    
    /**
     * @brief 根据业务ID查找帖子（不包含图片）
     * @param postId 业务逻辑ID（例：POST_2025Q4_ABC123）
     * @return 如果找到返回Post对象，否则返回std::nullopt
     */
    std::optional<Post> findByPostId(const std::string& postId);
    
    /**
     * @brief 根据业务ID查找帖子（包含图片，使用JOIN查询）
     * @param postId 业务逻辑ID
     * @return 如果找到返回Post对象（包含images），否则返回std::nullopt
     */
    std::optional<Post> findByPostIdWithImages(const std::string& postId);
    
    /**
     * @brief 更新帖子信息（标题、配文）
     * @param post 帖子对象
     * @return 成功返回true，失败返回false
     */
    bool updatePost(const Post& post);
    
    /**
     * @brief 删除帖子（级联删除图片）
     * @param postId 业务逻辑ID
     * @return 成功返回true，失败返回false
     */
    bool deletePost(const std::string& postId);
    
    /**
     * @brief 获取最新帖子列表（不包含图片）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return 帖子列表
     */
    std::vector<Post> getRecentPosts(int page, int pageSize);
    
    /**
     * @brief 获取最新帖子列表（包含图片，使用批量查询优化）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return 帖子列表（包含images）
     */
    std::vector<Post> getRecentPostsWithImages(int page, int pageSize);
    
    /**
     * @brief 获取最新帖子列表（包含图片，使用LEFT JOIN优化，推荐使用）
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return 帖子列表（包含images）
     * @note 使用LEFT JOIN + 一次查询获取所有数据，性能更优
     */
    std::vector<Post> getRecentPostsWithImagesOptimized(int page, int pageSize);
    
    /**
     * @brief 根据用户ID查找帖子列表（不包含图片）
     * @param userId 用户ID
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return 帖子列表
     */
    std::vector<Post> findByUserId(int userId, int page, int pageSize);
    
    /**
     * @brief 根据用户ID查找帖子列表（包含图片）
     * @param userId 用户ID
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return 帖子列表（包含images）
     */
    std::vector<Post> findByUserIdWithImages(int userId, int page, int pageSize);
    
    /**
     * @brief 增加浏览数
     * @param postId 业务逻辑ID
     * @return 成功返回true，失败返回false
     */
    bool incrementViewCount(const std::string& postId);
    
    /**
     * @brief 更新图片数量
     * @param postId 业务逻辑ID
     * @param imageCount 新的图片数量
     * @return 成功返回true，失败返回false
     */
    bool updateImageCount(const std::string& postId, int imageCount);
    
    /**
     * @brief 获取帖子总数
     * @return 帖子总数
     */
    int getTotalCount();
    
    /**
     * @brief 获取用户的帖子总数
     * @param userId 用户ID
     * @return 帖子总数
     */
    int getUserPostCount(int userId);

    /**
     * @brief 增加点赞数（原子操作）
     * @param conn MySQL连接
     * @param postId 帖子物理ID
     * @return 成功返回true，失败返回false
     */
    bool incrementLikeCount(MYSQL* conn, int postId);

    /**
     * @brief 减少点赞数（原子操作）
     * @param conn MySQL连接
     * @param postId 帖子物理ID
     * @return 成功返回true，失败返回false
     */
    bool decrementLikeCount(MYSQL* conn, int postId);

    /**
     * @brief 增加收藏数（原子操作）
     * @param conn MySQL连接
     * @param postId 帖子物理ID
     * @return 成功返回true，失败返回false
     */
    bool incrementFavoriteCount(MYSQL* conn, int postId);

    /**
     * @brief 减少收藏数（原子操作）
     * @param conn MySQL连接
     * @param postId 帖子物理ID
     * @return 成功返回true，失败返回false
     */
    bool decrementFavoriteCount(MYSQL* conn, int postId);

private:
    /**
     * @brief 从预编译语句构建Post对象
     * @param stmtPtr MYSQL_STMT指针
     * @return Post对象
     */
    Post buildPostFromStatement(void* stmtPtr);
    
    /**
     * @brief 批量加载帖子的图片
     * @param posts 帖子列表
     */
    void loadImagesForPosts(std::vector<Post>& posts);
};

