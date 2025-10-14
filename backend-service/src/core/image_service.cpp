/**
 * @file image_service.cpp
 * @brief 图片服务实现
 * @author Knot Team
 * @date 2025-10-12
 */

#include "core/image_service.h"
#include "database/image_repository.h"
#include "utils/image_processor.h"
#include "utils/logger.h"
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

// 构造函数
ImageService::ImageService() {
    imageRepo_ = std::make_unique<ImageRepository>();
    Logger::info("ImageService initialized");
}

// 析构函数
ImageService::~ImageService() {
    Logger::info("ImageService destroyed");
}

// 上传并处理图片
ImageUploadResult ImageService::uploadImage(
    int userId,
    const std::string& tempPath,
    const std::string& title,
    const std::string& description,
    const std::vector<std::string>& tags
) {
    ImageUploadResult result;

    Logger::info("ImageService::uploadImage - userId=" + std::to_string(userId) +
                 ", tempPath=" + tempPath);

    // 1. 验证图片格式
    if (!ImageProcessor::validateFormat(tempPath)) {
        result.success = false;
        result.message = "不支持的图片格式，仅支持JPEG、PNG、WebP";
        Logger::error("Invalid image format: " + tempPath);
        return result;
    }

    // 2. 生成图片ID
    std::string imageId = generateImageId();

    // 3. 处理图片（压缩 + 生成缩略图）
    ProcessResult processResult = ImageProcessor::processImage(
        tempPath,
        "uploads/images/",
        "uploads/thumbnails/",
        imageId
    );

    if (!processResult.success) {
        result.success = false;
        result.message = "图片处理失败: " + processResult.message;
        Logger::error("Image processing failed: " + processResult.message);
        return result;
    }

    // 4. 创建Image对象（不保存到数据库，由调用方负责设置postId并保存）
    Image image;
    image.setImageId(imageId);
    image.setUserId(userId);
    
    // 确保URL以斜杠开头（标准化路径格式）
    std::string fileUrl = processResult.originalPath;
    std::string thumbnailUrl = processResult.thumbnailPath;
    if (!fileUrl.empty() && fileUrl[0] != '/') {
        fileUrl = "/" + fileUrl;
    }
    if (!thumbnailUrl.empty() && thumbnailUrl[0] != '/') {
        thumbnailUrl = "/" + thumbnailUrl;
    }
    
    image.setFileUrl(fileUrl);
    image.setThumbnailUrl(thumbnailUrl);
    image.setFileSize(processResult.fileSize);
    image.setWidth(processResult.width);
    image.setHeight(processResult.height);
    image.setMimeType(ImageProcessor::getMimeType(processResult.originalPath));
    image.setCreateTime(std::time(nullptr));
    image.setUpdateTime(std::time(nullptr));

    // 注意：postId和displayOrder需要由调用方设置后再保存到数据库

    // 5. 返回成功结果（图片已处理但未保存到数据库）
    result.success = true;
    result.message = "图片处理成功";
    result.image = image;

    Logger::info("Image processed successfully: " + imageId);
    return result;
}

// 获取最新图片列表
ImageQueryResult ImageService::getRecentImages(int page, int pageSize) {
    ImageQueryResult result;

    try {
        Logger::info("Getting recent images: page=" + std::to_string(page) +
                     ", pageSize=" + std::to_string(pageSize));

        // 参数验证
        if (page < 1) {
            result.message = "页码必须大于等于1";
            Logger::warning(result.message);
            return result;
        }

        if (pageSize <= 0 || pageSize > 100) {
            result.message = "每页数量必须在1-100之间";
            Logger::warning(result.message);
            return result;
        }

        // 注意：当前ImageRepository没有分页查询方法，暂时返回空结果
        // 实际使用时需要扩展Repository层的方法
        result.success = true;
        result.message = "查询成功（功能待完善）";
        result.images = std::vector<Image>();
        result.total = 0;
        result.page = page;
        result.pageSize = pageSize;

        Logger::warning("Recent images functionality needs repository implementation");
        return result;

    } catch (const std::exception& e) {
        result.message = "查询最新图片异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

// 获取图片详情
std::optional<Image> ImageService::getImageDetail(const std::string& imageId) {
    try {
        Logger::info("Getting image detail: " + imageId);
        return imageRepo_->findByImageId(imageId);
    } catch (const std::exception& e) {
        Logger::error("Exception in getImageDetail: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 更新图片标题和描述
bool ImageService::updateImageText(const std::string& imageId, int userId, const std::string& title, const std::string& description) {
    try {
        Logger::info("Updating image text: imageId=" + imageId + ", userId=" + std::to_string(userId));

        // 1. 查询图片记录
        auto imageOpt = imageRepo_->findByImageId(imageId);
        if (!imageOpt.has_value()) {
            Logger::error("Image not found: " + imageId);
            return false;
        }

        Image image = imageOpt.value();

        // 2. 验证权限（只有上传者可以修改）
        if (image.getUserId() != userId) {
            Logger::error("Permission denied: user " + std::to_string(userId) +
                         " cannot update image " + imageId);
            return false;
        }

        // 3. 注意：Image模型目前没有title和description字段
        // 实际使用时需要扩展Image模型或使用其他方式存储图片描述
        image.setUpdateTime(std::time(nullptr));

        // 4. 保存到数据库（仅更新时间戳）
        if (!imageRepo_->updateImage(image)) {
            Logger::error("Failed to update image: " + imageId);
            return false;
        }

        Logger::warning("Image text update functionality needs model extension: " + imageId);
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in updateImageText: " + std::string(e.what()));
        return false;
    }
}

// 获取用户上传的图片列表
ImageQueryResult ImageService::getUserImages(int userId, int page, int pageSize) {
    ImageQueryResult result;

    try {
        Logger::info("Getting user images: userId=" + std::to_string(userId) +
                     ", page=" + std::to_string(page) +
                     ", pageSize=" + std::to_string(pageSize));

        // 参数验证
        if (page < 1) {
            result.message = "页码必须大于等于1";
            Logger::warning(result.message);
            return result;
        }

        if (pageSize <= 0 || pageSize > 100) {
            result.message = "每页数量必须在1-100之间";
            Logger::warning(result.message);
            return result;
        }

        // 注意：当前ImageRepository没有按用户查询方法，暂时返回空结果
        // 实际使用时需要扩展Repository层的方法
        result.success = true;
        result.message = "查询成功（功能待完善）";
        result.images = std::vector<Image>();
        result.total = 0;
        result.page = page;
        result.pageSize = pageSize;

        Logger::warning("User images functionality needs repository implementation");
        return result;

    } catch (const std::exception& e) {
        result.message = "查询用户图片异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

// 删除图片
bool ImageService::deleteImage(const std::string& imageId, int userId) {
    Logger::info("ImageService::deleteImage - imageId=" + imageId +
                 ", userId=" + std::to_string(userId));

    // 1. 查询图片记录
    auto imageOpt = imageRepo_->findByImageId(imageId);
    if (!imageOpt.has_value()) {
        Logger::error("Image not found: " + imageId);
        return false;
    }

    Image image = imageOpt.value();

    // 2. 验证权限（只有上传者可以删除）
    if (image.getUserId() != userId) {
        Logger::error("Permission denied: user " + std::to_string(userId) +
                     " cannot delete image " + imageId);
        return false;
    }

    // 3. 从数据库删除记录
    if (!imageRepo_->deleteImage(imageId)) {
        Logger::error("Failed to delete image record: " + imageId);
        return false;
    }

    // 4. 删除物理文件（原图和缩略图）
    // 注意：这里简化处理，实际生产环境应该考虑异步删除或软删除
    try {
        if (!image.getFileUrl().empty()) {
            std::remove(image.getFileUrl().c_str());
        }
        if (!image.getThumbnailUrl().empty()) {
            std::remove(image.getThumbnailUrl().c_str());
        }
    } catch (const std::exception& e) {
        Logger::warning("Failed to delete image files: " + std::string(e.what()));
        // 即使文件删除失败，数据库记录已删除，返回成功
    }

    Logger::info("Image deleted successfully: " + imageId);
    return true;
}

// 获取帖子的所有图片
std::vector<Image> ImageService::getImagesByPostId(int postId) {
    try {
        Logger::info("Getting images for post: " + std::to_string(postId));
        return imageRepo_->findByPostId(postId);
    } catch (const std::exception& e) {
        Logger::error("Exception in getImagesByPostId: " + std::string(e.what()));
        return std::vector<Image>();
    }
}

// 批量获取多个帖子的图片
std::map<int, std::vector<Image>> ImageService::getImagesByPostIds(const std::vector<int>& postIds) {
    try {
        Logger::info("Getting images for " + std::to_string(postIds.size()) + " posts");

        // Repository返回的是vector，需要转换为map
        std::vector<Image> allImages = imageRepo_->findByPostIds(postIds);
        std::map<int, std::vector<Image>> resultMap;

        // 按postId分组
        for (const auto& image : allImages) {
            int postId = image.getPostId();
            resultMap[postId].push_back(image);
        }

        return resultMap;
    } catch (const std::exception& e) {
        Logger::error("Exception in getImagesByPostIds: " + std::string(e.what()));
        return std::map<int, std::vector<Image>>();
    }
}

// 删除帖子的所有图片
bool ImageService::deleteImagesByPostId(int postId) {
    try {
        Logger::info("Deleting all images for post: " + std::to_string(postId));

        // 1. 获取帖子的所有图片
        std::vector<Image> images = imageRepo_->findByPostId(postId);

        // 2. 删除每张图片
        bool allSuccess = true;
        for (const auto& image : images) {
            if (!deleteImage(image.getImageId(), image.getUserId())) {
                Logger::warning("Failed to delete image: " + image.getImageId());
                allSuccess = false;
            }
        }

        Logger::info("Images deletion completed for post: " + std::to_string(postId));
        return allSuccess;

    } catch (const std::exception& e) {
        Logger::error("Exception in deleteImagesByPostId: " + std::string(e.what()));
        return false;
    }
}

// 更新图片显示顺序
bool ImageService::updateDisplayOrder(const std::string& imageId, int newOrder) {
    Logger::info("ImageService::updateDisplayOrder - imageId=" + imageId +
                 ", newOrder=" + std::to_string(newOrder));

    // 1. 验证顺序值（0-8）
    if (newOrder < 0 || newOrder > 8) {
        Logger::error("Invalid display order: " + std::to_string(newOrder));
        return false;
    }

    // 2. 查询图片记录
    auto imageOpt = imageRepo_->findByImageId(imageId);
    if (!imageOpt.has_value()) {
        Logger::error("Image not found: " + imageId);
        return false;
    }

    Image image = imageOpt.value();

    // 3. 更新显示顺序
    image.setDisplayOrder(newOrder);
    image.setUpdateTime(std::time(nullptr));

    if (!imageRepo_->updateImage(image)) {
        Logger::error("Failed to update display order for image: " + imageId);
        return false;
    }

    Logger::info("Display order updated successfully for image: " + imageId);
    return true;
}

// 生成唯一图片ID
std::string ImageService::generateImageId() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);

    int year = tm->tm_year + 1900;
    int quarter = (tm->tm_mon / 3) + 1;

    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

    std::string randomPart;
    for (int i = 0; i < 6; ++i) {
        randomPart += alphanum[dis(gen)];
    }

    std::ostringstream oss;
    oss << "IMG_" << year << "Q" << quarter << "_" << randomPart;

    return oss.str();
}
