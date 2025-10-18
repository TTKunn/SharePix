/**
 * @file image_repository.cpp
 * @brief 图片数据访问层实现（Post系统版本）
 * @author Knot Team
 * @date 2025-10-08
 */

#include "database/image_repository.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include "database/mysql_statement.h"
#include "utils/logger.h"
#include <mysql/mysql.h>
#include <cstring>
#include <memory>
#include <sstream>

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
        if (!stmt.isValid()) {
            return false;
        }
        
        // SQL 插入语句（新字段：post_id, display_order；移除：title, description, status）
        const char* query = "INSERT INTO images (image_id, post_id, display_order, user_id, file_url, thumbnail_url, file_size, width, height, mime_type) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        
        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }
        
        // 绑定参数
        MYSQL_BIND bind[10];
        memset(bind, 0, sizeof(bind));
        
        // image_id
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)image.getImageId().c_str();
        bind[0].buffer_length = image.getImageId().length();
        
        // post_id
        int postId = image.getPostId();
        bind[1].buffer_type = MYSQL_TYPE_LONG;
        bind[1].buffer = &postId;
        
        // display_order
        int displayOrder = image.getDisplayOrder();
        bind[2].buffer_type = MYSQL_TYPE_LONG;
        bind[2].buffer = &displayOrder;
        
        // user_id
        int userId = image.getUserId();
        bind[3].buffer_type = MYSQL_TYPE_LONG;
        bind[3].buffer = &userId;
        
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

// 从预编译语句构建 Image 对象（新字段结构）
Image ImageRepository::buildImageFromStatement(void* stmtPtr) {
    MYSQL_STMT* stmt = static_cast<MYSQL_STMT*>(stmtPtr);
    Image image;
    
    // 准备结果绑定（13个字段：id, image_id, post_id, display_order, user_id, file_url, thumbnail_url, file_size, width, height, mime_type, create_time, update_time）
    MYSQL_BIND result[13];
    memset(result, 0, sizeof(result));
    
    // 定义变量存储结果
    long long id;
    char imageId[37] = {0};
    int postId;
    int displayOrder;
    long long userId;
    char fileUrl[501] = {0};
    char thumbnailUrl[501] = {0};
    long long fileSize;
    int width, height;
    char mimeType[51] = {0};
    MYSQL_TIME createTime, updateTime;
    
    unsigned long imageId_length, fileUrl_length, thumbnailUrl_length, mimeType_length;
    
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
    
    // post_id
    result[idx].buffer_type = MYSQL_TYPE_LONG;
    result[idx].buffer = &postId;
    idx++;
    
    // display_order
    result[idx].buffer_type = MYSQL_TYPE_LONG;
    result[idx].buffer = &displayOrder;
    idx++;
    
    // user_id
    result[idx].buffer_type = MYSQL_TYPE_LONGLONG;
    result[idx].buffer = &userId;
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
        image.setPostId(postId);
        image.setDisplayOrder(displayOrder);
        image.setUserId(static_cast<int>(userId));
        image.setFileUrl(std::string(fileUrl, fileUrl_length));
        image.setThumbnailUrl(std::string(thumbnailUrl, thumbnailUrl_length));
        image.setFileSize(fileSize);
        image.setWidth(width);
        image.setHeight(height);
        image.setMimeType(std::string(mimeType, mimeType_length));
        
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
        if (!stmt.isValid()) {
            return std::nullopt;
        }

        const char* query = "SELECT i.id, i.image_id, i.post_id, i.display_order, i.user_id, "
                            "i.file_url, i.thumbnail_url, i.file_size, i.width, i.height, "
                            "i.mime_type, i.create_time, i.update_time, "
                            "COALESCE(u.user_id, '') AS user_logical_id "
                            "FROM images i "
                            "LEFT JOIN users u ON i.user_id = u.id "
                            "WHERE i.image_id = ?";

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

        // 获取user_logical_id字段
        MYSQL_BIND logicalIdBind[1];
        memset(logicalIdBind, 0, sizeof(logicalIdBind));
        char userLogicalId[128] = {0};
        unsigned long userLogicalIdLength;
        bool userLogicalIdIsNull;

        logicalIdBind[0].buffer_type = MYSQL_TYPE_STRING;
        logicalIdBind[0].buffer = userLogicalId;
        logicalIdBind[0].buffer_length = sizeof(userLogicalId);
        logicalIdBind[0].length = &userLogicalIdLength;
        logicalIdBind[0].is_null = &userLogicalIdIsNull;

        if (mysql_stmt_bind_result(stmt.get(), logicalIdBind) == 0 &&
            mysql_stmt_fetch_column(stmt.get(), logicalIdBind, 13, 0) == 0) {
            if (!userLogicalIdIsNull && userLogicalIdLength > 0) {
                image.setUserLogicalId(std::string(userLogicalId, userLogicalIdLength));
            }
        }

        if (image.getId() > 0) {
            return image;
        }

        return std::nullopt;

    } catch (const std::exception& e) {
        Logger::error("Exception in findByImageId: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 更新图片信息（只能更新display_order）
bool ImageRepository::updateImage(const Image& image) {
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

        const char* query = "UPDATE images SET display_order = ? WHERE image_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return false;
        }

        // 绑定参数
        MYSQL_BIND bind[2];
        memset(bind, 0, sizeof(bind));

        // display_order
        int displayOrder = image.getDisplayOrder();
        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &displayOrder;

        // image_id
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (char*)image.getImageId().c_str();
        bind[1].buffer_length = image.getImageId().length();

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
        if (!stmt.isValid()) {
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

// 根据帖子ID查找图片列表
std::vector<Image> ImageRepository::findByPostId(int postId) {
    std::vector<Image> images;

    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return images;
        }

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return images;
        }

        const char* query = "SELECT i.id, i.image_id, i.post_id, i.display_order, i.user_id, "
                            "i.file_url, i.thumbnail_url, i.file_size, i.width, i.height, "
                            "i.mime_type, i.create_time, i.update_time, "
                            "COALESCE(u.user_id, '') AS user_logical_id "
                            "FROM images i "
                            "LEFT JOIN users u ON i.user_id = u.id "
                            "WHERE i.post_id = ? "
                            "ORDER BY i.display_order";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &postId;

        if (mysql_stmt_bind_param(stmt.get(), bind) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        // 准备结果绑定（14个字段：id, image_id, post_id, display_order, user_id, file_url, thumbnail_url, file_size, width, height, mime_type, create_time, update_time, user_logical_id）
        MYSQL_BIND result[14];
        memset(result, 0, sizeof(result));

        // 定义变量存储结果
        long long id;
        char imageId[37] = {0};
        int postId_result;
        int displayOrder;
        long long userId;
        char fileUrl[501] = {0};
        char thumbnailUrl[501] = {0};
        long long fileSize;
        int width, height;
        char mimeType[51] = {0};
        MYSQL_TIME createTime, updateTime;
        char userLogicalId[128] = {0};

        unsigned long imageId_length, fileUrl_length, thumbnailUrl_length, mimeType_length, userLogicalIdLength;
        bool userLogicalIdIsNull;

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

        // post_id
        result[idx].buffer_type = MYSQL_TYPE_LONG;
        result[idx].buffer = &postId_result;
        idx++;

        // display_order
        result[idx].buffer_type = MYSQL_TYPE_LONG;
        result[idx].buffer = &displayOrder;
        idx++;

        // user_id
        result[idx].buffer_type = MYSQL_TYPE_LONGLONG;
        result[idx].buffer = &userId;
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

        // create_time
        result[idx].buffer_type = MYSQL_TYPE_TIMESTAMP;
        result[idx].buffer = &createTime;
        idx++;

        // update_time
        result[idx].buffer_type = MYSQL_TYPE_TIMESTAMP;
        result[idx].buffer = &updateTime;
        idx++;

        // user_logical_id (新增字段)
        result[idx].buffer_type = MYSQL_TYPE_STRING;
        result[idx].buffer = userLogicalId;
        result[idx].buffer_length = sizeof(userLogicalId);
        result[idx].length = &userLogicalIdLength;
        result[idx].is_null = &userLogicalIdIsNull;
        idx++;

        // 绑定结果
        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        // 稳定结果集
        mysql_stmt_store_result(stmt.get());

        // 获取所有结果
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Image image;
            image.setId(static_cast<int>(id));
            image.setImageId(std::string(imageId, imageId_length));
            image.setPostId(postId_result);
            image.setDisplayOrder(displayOrder);
            image.setUserId(static_cast<int>(userId));
            image.setFileUrl(std::string(fileUrl, fileUrl_length));
            image.setThumbnailUrl(std::string(thumbnailUrl, thumbnailUrl_length));
            image.setFileSize(fileSize);
            image.setWidth(width);
            image.setHeight(height);
            image.setMimeType(std::string(mimeType, mimeType_length));

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

            // 设置用户逻辑ID
            if (!userLogicalIdIsNull && userLogicalIdLength > 0) {
                image.setUserLogicalId(std::string(userLogicalId, userLogicalIdLength));
            }

            images.push_back(image);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in findByPostId: " + std::string(e.what()));
    }

    return images;
}

// 批量根据帖子ID查找图片
std::vector<Image> ImageRepository::findByPostIds(const std::vector<int>& postIds) {
    std::vector<Image> images;

    if (postIds.empty()) {
        return images;
    }

    try {
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return images;
        }

        // 构建IN子句
        std::ostringstream queryStream;
        queryStream << "SELECT i.id, i.image_id, i.post_id, i.display_order, i.user_id, "
                    << "i.file_url, i.thumbnail_url, i.file_size, i.width, i.height, "
                    << "i.mime_type, i.create_time, i.update_time, "
                    << "COALESCE(u.user_id, '') AS user_logical_id "
                    << "FROM images i "
                    << "LEFT JOIN users u ON i.user_id = u.id "
                    << "WHERE i.post_id IN (";
        for (size_t i = 0; i < postIds.size(); ++i) {
            if (i > 0) queryStream << ",";
            queryStream << "?";
        }
        queryStream << ") ORDER BY i.post_id, i.display_order";

        std::string queryStr = queryStream.str();
        const char* query = queryStr.c_str();

        MySQLStatement stmt(connGuard.get());
        if (!stmt.isValid()) {
            return images;
        }

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        // 绑定参数
        std::vector<MYSQL_BIND> bind(postIds.size());
        memset(bind.data(), 0, sizeof(MYSQL_BIND) * postIds.size());

        // 需要保持postIds的副本，因为bind需要指向稳定的内存地址
        std::vector<int> postIdsCopy = postIds;

        for (size_t i = 0; i < postIdsCopy.size(); ++i) {
            bind[i].buffer_type = MYSQL_TYPE_LONG;
            bind[i].buffer = &postIdsCopy[i];
        }

        if (mysql_stmt_bind_param(stmt.get(), bind.data()) != 0) {
            Logger::error("Failed to bind parameters: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        if (mysql_stmt_execute(stmt.get()) != 0) {
            Logger::error("Failed to execute statement: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        // 准备结果绑定（14个字段：id, image_id, post_id, display_order, user_id, file_url, thumbnail_url, file_size, width, height, mime_type, create_time, update_time, user_logical_id）
        MYSQL_BIND result[14];
        memset(result, 0, sizeof(result));

        // 定义变量存储结果
        long long id;
        char imageId[37] = {0};
        int postId_result;
        int displayOrder;
        long long userId;
        char fileUrl[501] = {0};
        char thumbnailUrl[501] = {0};
        long long fileSize;
        int width, height;
        char mimeType[51] = {0};
        MYSQL_TIME createTime, updateTime;
        char userLogicalId[128] = {0};

        unsigned long imageId_length, fileUrl_length, thumbnailUrl_length, mimeType_length, userLogicalIdLength;
        bool userLogicalIdIsNull;

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

        // post_id
        result[idx].buffer_type = MYSQL_TYPE_LONG;
        result[idx].buffer = &postId_result;
        idx++;

        // display_order
        result[idx].buffer_type = MYSQL_TYPE_LONG;
        result[idx].buffer = &displayOrder;
        idx++;

        // user_id
        result[idx].buffer_type = MYSQL_TYPE_LONGLONG;
        result[idx].buffer = &userId;
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

        // create_time
        result[idx].buffer_type = MYSQL_TYPE_TIMESTAMP;
        result[idx].buffer = &createTime;
        idx++;

        // update_time
        result[idx].buffer_type = MYSQL_TYPE_TIMESTAMP;
        result[idx].buffer = &updateTime;
        idx++;

        // user_logical_id (新增字段)
        result[idx].buffer_type = MYSQL_TYPE_STRING;
        result[idx].buffer = userLogicalId;
        result[idx].buffer_length = sizeof(userLogicalId);
        result[idx].length = &userLogicalIdLength;
        result[idx].is_null = &userLogicalIdIsNull;
        idx++;

        // 绑定结果
        if (mysql_stmt_bind_result(stmt.get(), result) != 0) {
            Logger::error("Failed to bind result: " + std::string(mysql_stmt_error(stmt.get())));
            return images;
        }

        // 稳定结果集
        mysql_stmt_store_result(stmt.get());

        // 获取所有结果
        while (mysql_stmt_fetch(stmt.get()) == 0) {
            Image image;
            image.setId(static_cast<int>(id));
            image.setImageId(std::string(imageId, imageId_length));
            image.setPostId(postId_result);
            image.setDisplayOrder(displayOrder);
            image.setUserId(static_cast<int>(userId));
            image.setFileUrl(std::string(fileUrl, fileUrl_length));
            image.setThumbnailUrl(std::string(thumbnailUrl, thumbnailUrl_length));
            image.setFileSize(fileSize);
            image.setWidth(width);
            image.setHeight(height);
            image.setMimeType(std::string(mimeType, mimeType_length));

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

            // 设置用户逻辑ID
            if (!userLogicalIdIsNull && userLogicalIdLength > 0) {
                image.setUserLogicalId(std::string(userLogicalId, userLogicalIdLength));
            }

            images.push_back(image);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in findByPostIds: " + std::string(e.what()));
    }

    return images;
}

// 删除帖子的所有图片
bool ImageRepository::deleteByPostId(int postId) {
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

        const char* query = "DELETE FROM images WHERE post_id = ?";

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

        Logger::info("Images deleted successfully for post_id: " + std::to_string(postId));
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in deleteByPostId: " + std::string(e.what()));
        return false;
    }
}

// 获取帖子的图片数量
int ImageRepository::getImageCountByPostId(int postId) {
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

        const char* query = "SELECT COUNT(*) FROM images WHERE post_id = ?";

        if (mysql_stmt_prepare(stmt.get(), query, strlen(query)) != 0) {
            Logger::error("Failed to prepare statement: " + std::string(mysql_stmt_error(stmt.get())));
            return 0;
        }

        // 绑定参数
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));

        bind[0].buffer_type = MYSQL_TYPE_LONG;
        bind[0].buffer = &postId;

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
        Logger::error("Exception in getImageCountByPostId: " + std::string(e.what()));
        return 0;
    }
}


