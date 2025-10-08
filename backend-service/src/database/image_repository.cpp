/**
 * @file image_repository.cpp
 * @brief 图片数据访问层实现
 * @author Knot Team
 * @date 2025-10-07
 */

#include "database/image_repository.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include "utils/logger.h"
#include <mysql/mysql.h>
#include <cstring>
#include <memory>

// RAII 封装 MYSQL_STMT
class MySQLStatement {
public:
    explicit MySQLStatement(MYSQL* conn) {
        stmt_ = mysql_stmt_init(conn);
        if (!stmt_) {
            Logger::error("Failed to initialize MySQL statement");
        }
    }
    
    ~MySQLStatement() {
        if (stmt_) {
            mysql_stmt_close(stmt_);
        }
    }
    
    MYSQL_STMT* get() { return stmt_; }
    
    // 禁止拷贝
    MySQLStatement(const MySQLStatement&) = delete;
    MySQLStatement& operator=(const MySQLStatement&) = delete;
    
private:
    MYSQL_STMT* stmt_;
};

// 构造函数
ImageRepository::ImageRepository() {
    Logger::info("ImageRepository initialized");
}

// 创建图片记录
bool ImageRepository::createImage(const Image& image) {
    try {
        // 使用ConnectionGuard自动管理连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }
        
        // SQL 插入语句
        const char* query = "INSERT INTO images (image_id, user_id, title, description, file_url, thumbnail_url, file_size, width, height, mime_type, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        // 绑定参数
        MYSQL_BIND bind[11];
        memset(bind, 0, sizeof(bind));
        
        // image_id
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)image.getImageId().c_str();
        bind[0].buffer_length = image.getImageId().length();
        
        // user_id
        int userId = image.getUserId();
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &userId;
        
        // title
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)image.getTitle().c_str();
        bind[2].buffer_length = image.getTitle().length();
        
        // description (可为空)
        std::string description = image.getDescription();
        bool description_is_null = description.empty();
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = (char*)description.c_str();
        bind[3].buffer_length = description.length();
        bind[3].is_null = &description_is_null;
        
        // file_url
        bind[4].buffer_type = MYSQL_TYPE_STRING;
        bind[4].buffer = (char*)image.getFileUrl().c_str();
        bind[4].buffer_length = image.getFileUrl().length();
        
        // thumbnail_url
        bind[5].buffer_type = MYSQL_TYPE_STRING;
        bind[5].buffer = (char*)image.getThumbnailUrl().c_str();
        bind[5].buffer_length = image.getThumbnailUrl().length();
        
        // file_size
        long long fileSize = image.getFileSize();
        bind[6].buffer_type = MYSQL_TYPE_LONGLONG;
        bind[6].buffer = &fileSize;
        
        // width
        int width = image.getWidth();
        bind[7].buffer_type = MYSQL_TYPE_LONG;
        bind[7].buffer = &width;
        
        // height
        int height = image.getHeight();
        bind[8].buffer_type = MYSQL_TYPE_LONG;
        bind[8].buffer = &height;
        
        // mime_type
        bind[9].buffer_type = MYSQL_TYPE_STRING;
        bind[9].buffer = (char*)image.getMimeType().c_str();
        bind[9].buffer_length = image.getMimeType().length();
        
        // status
        std::string status = Image::statusToString(image.getStatus());
        bind[10].buffer_type = MYSQL_TYPE_STRING;
        bind[10].buffer = (char*)status.c_str();
        bind[10].buffer_length = status.length();
        
        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        Logger::info("Image created successfully: " + image.getImageId());
        return true;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in createImage: " + std::string(e.what()));
        return false;
    }
}

// 从预编译语句构建 Image 对象
Image ImageRepository::buildImageFromStatement(void* stmtPtr) {
    MYSQL_STMT* stmt = static_cast<MYSQL_STMT*>(stmtPtr);
    Image image;
    
    // 准备结果绑定
    MYSQL_BIND result[17];
    memset(result, 0, sizeof(result));
    
    // 定义变量存储结果
    long long id;
    char imageId[37] = {0};
    long long userId;
    char title[256] = {0};
    char description[1024] = {0};
    char fileUrl[501] = {0};
    char thumbnailUrl[501] = {0};
    long long fileSize;
    int width, height;
    char mimeType[51] = {0};
    int likeCount, favoriteCount, viewCount;
    char status[20] = {0};
    MYSQL_TIME createTime, updateTime;
    
    unsigned long imageId_length, title_length, description_length;
    unsigned long fileUrl_length, thumbnailUrl_length, mimeType_length, status_length;
    
    bool description_is_null;
    
    // 绑定结果（按照 SELECT * 的顺序）
    int idx = 0;
    
    // id
    result[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    result[idx].buffer = &id;
    idx++;
    
    // image_id
    result[idx].buffer_type = MYSQL_TYPE_STRING;
    result[idx].buffer = imageId;
    result[idx].buffer_length = sizeof(imageId);
    result[idx].length = &imageId_length;
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
    
    // file_url
    result[idx].buffer_type = MYSQL_TYPE_STRING;
    result[idx].buffer = fileUrl;
    result[idx].buffer_length = sizeof(fileUrl);
    result[idx].length = &fileUrl_length;
    idx++;
    
    // thumbnail_url
    result[idx].buffer_type = MYSQL_TYPE_STRING;
    result[idx].buffer = thumbnailUrl;
    result[idx].buffer_length = sizeof(thumbnailUrl);
    result[idx].length = &thumbnailUrl_length;
    idx++;
    
    // file_size
    result[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    result[idx].buffer = &fileSize;
    idx++;
    
    // width
    result[idx].buffer_type = MYSQL_TYPE_LONG;
    result[idx].buffer = &width;
    idx++;
    
    // height
    result[idx].buffer_type = MYSQL_TYPE_LONG;
    result[idx].buffer = &height;
    idx++;
    
    // mime_type
    result[idx].buffer_type = MYSQL_TYPE_STRING;
    result[idx].buffer = mimeType;
    result[idx].buffer_length = sizeof(mimeType);
    result[idx].length = &mimeType_length;
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
        return image;
    }
    
    // 获取数据
    if (mysql_stmt_fetch(stmt) == 0) {
        image.setId(static_cast<int>(id));
        image.setImageId(std::string(imageId, imageId_length));
        image.setUserId(static_cast<int>(userId));
        image.setTitle(std::string(title, title_length));
        
        if (!description_is_null) {
            image.setDescription(std::string(description, description_length));
        }
        
        image.setFileUrl(std::string(fileUrl, fileUrl_length));
        image.setThumbnailUrl(std::string(thumbnailUrl, thumbnailUrl_length));
        image.setFileSize(fileSize);
        image.setWidth(width);
        image.setHeight(height);
        image.setMimeType(std::string(mimeType, mimeType_length));
        image.setLikeCount(likeCount);
        image.setFavoriteCount(favoriteCount);
        image.setViewCount(viewCount);
        image.setStatus(Image::stringToStatus(std::string(status, status_length)));
        
        // 转换时间
        struct tm tm_create = {0};
        tm_create.tm_year = createTime.year - 1900;
        tm_create.tm_mon = createTime.month - 1;
        tm_create.tm_mday = createTime.day;
        tm_create.tm_hour = createTime.hour;
        tm_create.tm_min = createTime.minute;
        tm_create.tm_sec = createTime.second;
        image.setCreateTime(mktime(&tm_create));
        
        struct tm tm_update = {0};
        tm_update.tm_year = updateTime.year - 1900;
        tm_update.tm_mon = updateTime.month - 1;
        tm_update.tm_mday = updateTime.day;
        tm_update.tm_hour = updateTime.hour;
        tm_update.tm_min = updateTime.minute;
        tm_update.tm_sec = updateTime.second;
        image.setUpdateTime(mktime(&tm_update));
    }
    
    return image;
}

// 根据业务ID查找图片
std::optional<Image> ImageRepository::findByImageId(const std::string& imageId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return std::nullopt;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return std::nullopt;
        }

        const char* query = "SELECT * FROM images WHERE image_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)imageId.c_str();
        bind[0].buffer_length = imageId.length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return std::nullopt;
        }

        Image image = buildImageFromStatement(stmt.get());

        if (image.getId() > 0) {
            return image;
        }

        return std::nullopt;

    } catch (const std::exception& e) {
        Logger::error("Exception in findByImageId: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 更新图片信息
bool ImageRepository::updateImage(const Image& image) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }

        const char* query = "UPDATE images SET title = ?, description = ? WHERE image_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[3];
        memset(bind, 0, sizeof(bind));

        // title
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)image.getTitle().c_str();
        bind[0].buffer_length = image.getTitle().length();

        // description
        std::string description = image.getDescription();
        bool description_is_null = description.empty();
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)description.c_str();
        bind[1].buffer_length = description.length();
        bind[1].is_null = &description_is_null;

        // image_id
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)image.getImageId().c_str();
        bind[2].buffer_length = image.getImageId().length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::info("Image updated successfully: " + image.getImageId());
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in updateImage: " + std::string(e.what()));
        return false;
    }
}

// 删除图片
bool ImageRepository::deleteImage(const std::string& imageId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }

        const char* query = "DELETE FROM images WHERE image_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)imageId.c_str();
        bind[0].buffer_length = imageId.length();

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        Logger::info("Image deleted successfully: " + imageId);
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in deleteImage: " + std::string(e.what()));
        return false;
    }
}

// 获取最新图片列表（时间倒序）
std::vector<Image> ImageRepository::getRecentImages(int page, int pageSize) {
    std::vector<Image> images;

    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return images;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return images;
        }

        const char* query = "SELECT * FROM images WHERE status = 'APPROVED' ORDER BY create_time DESC LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
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
            return images;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        // 获取所有结果
        while (true) {
            Image image = buildImageFromStatement(stmt.get());
            if (image.getId() > 0) {
                images.push_back(image);
                // 移动到下一行
                if (mysql_stmt_fetch(stmt.get()) != 0) {
                    break;
                }
            } else {
                break;
            }
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in getRecentImages: " + std::string(e.what()));
    }

    return images;
}

// 根据用户ID查找图片列表
std::vector<Image> ImageRepository::findByUserId(int userId, int page, int pageSize) {
    std::vector<Image> images;

    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return images;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return images;
        }

        const char* query = "SELECT * FROM images WHERE user_id = ? ORDER BY create_time DESC LIMIT ? OFFSET ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
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
            return images;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        // 获取所有结果
        while (true) {
            Image image = buildImageFromStatement(stmt.get());
            if (image.getId() > 0) {
                images.push_back(image);
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

    return images;
}

// 增加浏览数
bool ImageRepository::incrementViewCount(const std::string& imageId) {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return false;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return false;
        }

        const char* query = "UPDATE images SET view_count = view_count + 1 WHERE image_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)imageId.c_str();
        bind[0].buffer_length = imageId.length();

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

// 获取图片总数
int ImageRepository::getTotalCount() {
    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return 0;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.get()) {
            return 0;
        }

        const char* query = "SELECT COUNT(*) FROM images WHERE status = 'APPROVED'";

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

