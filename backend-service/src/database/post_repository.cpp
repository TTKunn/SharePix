/**
 * @file post_repository.cpp
 * @brief 帖子数据访问层实现
 * @author Knot Team
 * @date 2025-10-08
 */

#include "database/post_repository.h"
#include "database/image_repository.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include "database/mysql_statement.h"
#include "utils/logger.h"
#include <mysql/mysql.h>
#include <cstring>
#include <memory>
#include <sstream>
#include <map>

// 构造函数
PostRepository::PostRepository() {
    Logger::info("PostRepository initialized");
}

// 创建帖子记录（不包含图片）
bool PostRepository::createPost(Post& post) {
    try {
        // 使用ConnectionGuard自动管理连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return false;
        }
        
        // SQL 插入语句
        const char* query = "INSERT INTO posts (post_id, user_id, title, description, image_count, status) VALUES (?, ?, ?, ?, ?, ?)";
        
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        // 绑定参数
        MYSQL_BIND bind[6];
        memset(bind, 0, sizeof(bind));
        
        // post_id
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)post.getPostId().c_str();
        bind[0].buffer_length = post.getPostId().length();
        
        // user_id
        int userId = post.getUserId();
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &userId;
        
        // title
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)post.getTitle().c_str();
        bind[2].buffer_length = post.getTitle().length();
        
        // description (可为空)
        std::string description = post.getDescription();
        bool description_is_null = description.empty();
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = (char*)description.c_str();
        bind[3].buffer_length = description.length();
        bind[3].is_null = &description_is_null;
        
        // image_count
        int imageCount = post.getImageCount();
        bind[4].buffer_type = MYSQL_TYPE_LONG;
        bind[4].buffer = &imageCount;
        
        // status
        std::string status = Post::statusToString(post.getStatus());
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = (char*)status.c_str();
        bind[5].buffer_length = status.length();
        
        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        // 获取自增ID
        post.setId(static_cast<int>(mysql_stmt_insert_id(stmt.get())));
        
        Logger::info("Post created successfully: " + post.getPostId());
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in createPost: " + std::string(e.what()));
        return false;
    }
}

// 从预编译语句构建Post对象
Post PostRepository::buildPostFromStatement(void* stmtPtr) {
    MYSQL_STMT* stmt = static_cast<MYSQL_STMT*>(stmtPtr);
    Post post;
    
    // 准备结果绑定（12个字段：id, post_id, user_id, title, description, image_count, like_count, favorite_count, view_count, status, create_time, update_time）
    MYSQL_BIND result[12];
    memset(result, 0, sizeof(result));
    
    // 定义变量存储结果
    long long id;
    char postId[37] = {0};
    long long userId;
    char title[256] = {0};
    char description[1024] = {0};
    int imageCount, likeCount, favoriteCount, viewCount;
    char status[20] = {0};
    MYSQL_TIME createTime, updateTime;
    
    unsigned long postId_length, title_length, description_length, status_length;
    bool description_is_null;
    
    // 绑定结果（按照 SELECT * 的顺序）
    int idx = 0;
    
    // id
    result[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    result[idx].buffer = &id;
    idx++;
    
    // post_id
    result[idx].buffer_type = MYSQL_TYPE_STRING;
    result[idx].buffer = postId;
    result[idx].buffer_length = sizeof(postId);
    result[idx].length = &postId_length;
    idx++;
    
    // user_id
    result[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    result[idx].buffer = &userId;
    idx++;
    
    // title
    result[idx].buffer_type = MYSQL_TYPE_STRING;
    result[idx].buffer = title;
    result[idx].buffer_length = sizeof(title);
    result[idx].length = &title_length;
    idx++;
    
    // description
    result[idx].buffer_type = MYSQL_TYPE_STRING;
    result[idx].buffer = description;
    result[idx].buffer_length = sizeof(description);
    result[idx].length = &description_length;
    result[idx].is_null = &description_is_null;
    idx++;
    
    // image_count
    result[idx].buffer_type = MYSQL_TYPE_LONG;
    result[idx].buffer = &imageCount;
    idx++;
    
    // like_count
    result[idx].buffer_type = MYSQL_TYPE_LONG;
    result[idx].buffer = &likeCount;
    idx++;
    
    // favorite_count
    result[idx].buffer_type = MYSQL_TYPE_LONG;
    result[idx].buffer = &favoriteCount;
    idx++;
    
    // view_count
    result[idx].buffer_type = MYSQL_TYPE_LONG;
    result[idx].buffer = &viewCount;
    idx++;
    
    // status
    result[idx].buffer_type = MYSQL_TYPE_STRING;
    result[idx].buffer = status;
    result[idx].buffer_length = sizeof(status);
    result[idx].length = &status_length;
    idx++;
    
    // create_time
    result[idx].buffer_type = MYSQL_TYPE_TIMESTAMP;
    result[idx].buffer = &createTime;
    idx++;
    
    // update_time
    result[idx].buffer_type = MYSQL_TYPE_TIMESTAMP;
    result[idx].buffer = &updateTime;
    idx++;
    
    // 绑定结果
    if (mysql_stmt_bind_result(stmt, result) != 0) {
        Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt)));
        return post;
    }
    
    // 获取数据
    if (mysql_stmt_fetch(stmt) == 0) {
        post.setId(static_cast<int>(id));
        post.setPostId(std::string(postId, postId_length));
        post.setUserId(static_cast<int>(userId));
        post.setTitle(std::string(title, title_length));
        
        if (!description_is_null) {
            post.setDescription(std::string(description, description_length));
        }
        
        post.setImageCount(imageCount);
        post.setLikeCount(likeCount);
        post.setFavoriteCount(favoriteCount);
        post.setViewCount(viewCount);
        post.setStatus(Post::stringToStatus(std::string(status, status_length)));
        
        // 转换时间
        struct tm tm_create = {0};
        tm_create.tm_year = createTime.year - 1900;
        tm_create.tm_mon = createTime.month - 1;
        tm_create.tm_mday = createTime.day;
        tm_create.tm_hour = createTime.hour;
        tm_create.tm_min = createTime.minute;
        tm_create.tm_sec = createTime.second;
        post.setCreateTime(mktime(&tm_create));
        
        struct tm tm_update = {0};
        tm_update.tm_year = updateTime.year - 1900;
        tm_update.tm_mon = updateTime.month - 1;
        tm_update.tm_mday = updateTime.day;
        tm_update.tm_hour = updateTime.hour;
        tm_update.tm_min = updateTime.minute;
        tm_update.tm_sec = updateTime.second;
        post.setUpdateTime(mktime(&tm_update));
    }
    
    return post;
}

// 根据业务ID查找帖子（不包含图片）
std::optional<Post> PostRepository::findByPostId(const std::string& postId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return std::nullopt;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return std::nullopt;
        }

        const char* query = "SELECT * FROM posts WHERE post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)postId.c_str();
        bind[0].buffer_length = postId.length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        Post post = buildPostFromStatement(stmt.get());

        if (post.getId() > 0) {
            return post;
        }

        return std::nullopt;

    } catch (const std::exception& e) {
        Logger::error("Exception in findByPostId: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 根据业务ID查找帖子（包含图片，使用JOIN查询）
std::optional<Post> PostRepository::findByPostIdWithImages(const std::string& postId) {
    try {
        // 先查询Post
        auto postOpt = findByPostId(postId);
        if (!postOpt.has_value()) {
            return std::nullopt;
        }

        Post post = postOpt.value();

        // 再查询Images
        ImageRepository imageRepo;
        std::vector<Image> images = imageRepo.findByPostId(post.getId());

        // 添加图片到Post
        for (const auto& image : images) {
            post.addImage(image);
        }

        return post;

    } catch (const std::exception& e) {
        Logger::error("Exception in findByPostIdWithImages: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 更新帖子信息（标题、配文）
bool PostRepository::updatePost(const Post& post) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return false;
        }

        const char* query = "UPDATE posts SET title = ?, description = ? WHERE post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        // title
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)post.getTitle().c_str();
        bind[0].buffer_length = post.getTitle().length();

        // description
        std::string description = post.getDescription();
        bool description_is_null = description.empty();
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)description.c_str();
        bind[1].buffer_length = description.length();
        bind[1].is_null = &description_is_null;

        // post_id
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)post.getPostId().c_str();
        bind[2].buffer_length = post.getPostId().length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::info("Post updated successfully: " + post.getPostId());
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in updatePost: " + std::string(e.what()));
        return false;
    }
}

// 删除帖子（级联删除图片）
bool PostRepository::deletePost(const std::string& postId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return false;
        }

        const char* query = "DELETE FROM posts WHERE post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)postId.c_str();
        bind[0].buffer_length = postId.length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::info("Post deleted successfully: " + postId);
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in deletePost: " + std::string(e.what()));
        return false;
    }
}

// 获取最新帖子列表（不包含图片）
std::vector<Post> PostRepository::getRecentPosts(int page, int pageSize) {
    std::vector<Post> posts;

    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return posts;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return posts;
        }

        const char* query = "SELECT * FROM posts WHERE status = 'APPROVED' ORDER BY create_time DESC LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        // 计算OFFSET
        int offset = (page - 1) * pageSize;

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &pageSize;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        // 获取所有结果
        while (true) {
            Post post = buildPostFromStatement(stmt.get());
            if (post.getId() > 0) {
                posts.push_back(post);
                // 移动到下一行
                if (mysql_stmt_fetch(stmt.get()) != 0) {
                    break;
                }
            } else {
                break;
            }
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in getRecentPosts: " + std::string(e.what()));
    }

    return posts;
}

// 批量加载帖子的图片
void PostRepository::loadImagesForPosts(std::vector<Post>& posts) {
    if (posts.empty()) {
        return;
    }

    try {
        // 收集所有post_id
        std::vector<int> postIds;
        for (const auto& post : posts) {
            postIds.push_back(post.getId());
        }

        // 批量查询所有图片
        ImageRepository imageRepo;
        std::vector<Image> allImages = imageRepo.findByPostIds(postIds);

        // 按post_id分组
        std::map<int, std::vector<Image>> imagesByPostId;
        for (const auto& image : allImages) {
            imagesByPostId[image.getPostId()].push_back(image);
        }

        // 将图片添加到对应的Post
        for (auto& post : posts) {
            if (imagesByPostId.find(post.getId()) != imagesByPostId.end()) {
                for (const auto& image : imagesByPostId[post.getId()]) {
                    post.addImage(image);
                }
            }
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in loadImagesForPosts: " + std::string(e.what()));
    }
}

// 获取最新帖子列表（包含图片，使用批量查询优化）
std::vector<Post> PostRepository::getRecentPostsWithImages(int page, int pageSize) {
    // 先查询Posts
    std::vector<Post> posts = getRecentPosts(page, pageSize);

    // 批量加载图片
    loadImagesForPosts(posts);

    return posts;
}

// 根据用户ID查找帖子列表（不包含图片）
std::vector<Post> PostRepository::findByUserId(int userId, int page, int pageSize) {
    std::vector<Post> posts;

    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return posts;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return posts;
        }

        const char* query = "SELECT * FROM posts WHERE user_id = ? ORDER BY create_time DESC LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        // 计算OFFSET
        int offset = (page - 1) * pageSize;

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &userId;

        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &pageSize;

        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &offset;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return posts;
        }

        // 获取所有结果
        while (true) {
            Post post = buildPostFromStatement(stmt.get());
            if (post.getId() > 0) {
                posts.push_back(post);
                // 移动到下一行
                if (mysql_stmt_fetch(stmt.get()) != 0) {
                    break;
                }
            } else {
                break;
            }
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in findByUserId: " + std::string(e.what()));
    }

    return posts;
}

// 根据用户ID查找帖子列表（包含图片）
std::vector<Post> PostRepository::findByUserIdWithImages(int userId, int page, int pageSize) {
    // 先查询Posts
    std::vector<Post> posts = findByUserId(userId, page, pageSize);

    // 批量加载图片
    loadImagesForPosts(posts);

    return posts;
}

// 增加浏览数
bool PostRepository::incrementViewCount(const std::string& postId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return false;
        }

        const char* query = "UPDATE posts SET view_count = view_count + 1 WHERE post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)postId.c_str();
        bind[0].buffer_length = postId.length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in incrementViewCount: " + std::string(e.what()));
        return false;
    }
}

// 更新图片数量
bool PostRepository::updateImageCount(const std::string& postId, int imageCount) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return false;
        }

        const char* query = "UPDATE posts SET image_count = ? WHERE post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &imageCount;

        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)postId.c_str();
        bind[1].buffer_length = postId.length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in updateImageCount: " + std::string(e.what()));
        return false;
    }
}

// 获取帖子总数
int PostRepository::getTotalCount() {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return 0;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return 0;
        }

        const char* query = "SELECT COUNT(*) FROM posts WHERE status = 'APPROVED'";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定结果
        MYSQL_BIND result[1];
        memset(result, 0, sizeof(result));

        long long count = 0;
        result[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result[0].buffer = &count;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            return static_cast<int>(count);
        }

        return 0;

    } catch (const std::exception& e) {
        Logger::error("Exception in getTotalCount: " + std::string(e.what()));
        return 0;
    }
}

// 获取用户的帖子总数
int PostRepository::getUserPostCount(int userId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return 0;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return 0;
        }

        const char* query = "SELECT COUNT(*) FROM posts WHERE user_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &userId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定结果
        MYSQL_BIND result[1];
        memset(result, 0, sizeof(result));

        long long count = 0;
        result[0].buffer_type = MYSQL_TYPE_LONGLONG;
        result[0].buffer = &count;

        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        if (mysql_stmt_fetch(stmt.get()) == 0) {
            return static_cast<int>(count);
        }

        return 0;

    } catch (const std::exception& e) {
        Logger::error("Exception in getUserPostCount: " + std::string(e.what()));
        return 0;
    }
}

// 增加点赞数（原子操作）
bool PostRepository::incrementLikeCount(MYSQL* conn, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // 使用原子操作更新计数
        const char* query = "UPDATE posts SET like_count = like_count + 1 WHERE id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::debug("Incremented like count for post id=" + std::to_string(postId));
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in incrementLikeCount: " + std::string(e.what()));
        return false;
    }
}

// 减少点赞数（原子操作）
bool PostRepository::decrementLikeCount(MYSQL* conn, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // 使用原子操作更新计数，确保不会小于0
        const char* query = "UPDATE posts SET like_count = like_count - 1 WHERE id = ? AND like_count > 0";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::debug("Decremented like count for post id=" + std::to_string(postId));
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in decrementLikeCount: " + std::string(e.what()));
        return false;
    }
}

// 增加收藏数（原子操作）
bool PostRepository::incrementFavoriteCount(MYSQL* conn, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // 使用原子操作更新计数
        const char* query = "UPDATE posts SET favorite_count = favorite_count + 1 WHERE id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::debug("Incremented favorite count for post id=" + std::to_string(postId));
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in incrementFavoriteCount: " + std::string(e.what()));
        return false;
    }
}

// 减少收藏数（原子操作）
bool PostRepository::decrementFavoriteCount(MYSQL* conn, int postId) {
    try {
        if (!conn) {
            Logger::error("Database connection is null");
            return false;
        }

        MySQLStatement stmt(conn);
        if (!stmt.isValid()) {
            return false;
        }

        // 使用原子操作更新计数，确保不会小于0
        const char* query = "UPDATE posts SET favorite_count = favorite_count - 1 WHERE id = ? AND favorite_count > 0";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::debug("Decremented favorite count for post id=" + std::to_string(postId));
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in decrementFavoriteCount: " + std::string(e.what()));
        return false;
    }
}

