/**
 * @file image_processor.cpp
 * @brief 图片处理工具类实现
 * @author Knot Team
 * @date 2025-10-07
 */

#include "utils/image_processor.h"
#include "utils/logger.h"
#include <sys/stat.h>
#include <algorithm>
#include <cstring>

// 定义STB宏
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

// 包含stb头文件
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

// 处理图片（压缩 + 生成缩略图）
ProcessResult ImageProcessor::processImage(const std::string& inputPath,
                                          const std::string& outputDir,
                                          const std::string& thumbnailDir,
                                          const std::string& filename) {
    ProcessResult result;
    
    try {
        // 1. 验证文件格式
        if (!validateFormat(inputPath)) {
            result.message = "不支持的图片格式";
            Logger::error("Invalid image format: " + inputPath);
            return result;
        }
        
        // 2. 验证文件大小
        long long fileSize = getFileSize(inputPath);
        if (fileSize > MAX_FILE_SIZE) {
            result.message = "文件大小超过5MB限制";
            Logger::error("File size exceeds limit: " + std::to_string(fileSize));
            return result;
        }
        
        // 3. 加载图片
        int width, height, channels;
        unsigned char* data = stbi_load(inputPath.c_str(), &width, &height, &channels, 0);
        
        if (!data) {
            result.message = "无法加载图片";
            Logger::error("Failed to load image: " + inputPath);
            return result;
        }
        
        // 4. 保存压缩后的原图
        std::string originalPath = outputDir + filename + ".jpg";
        int saveResult = stbi_write_jpg(originalPath.c_str(), width, height, channels, data, JPEG_QUALITY);
        
        if (!saveResult) {
            result.message = "无法保存压缩图片";
            Logger::error("Failed to save compressed image: " + originalPath);
            stbi_image_free(data);
            return result;
        }
        
        // 5. 生成缩略图
        std::string thumbnailPath = thumbnailDir + filename + "_thumb.jpg";
        if (!generateThumbnail(data, width, height, channels, thumbnailPath)) {
            result.message = "无法生成缩略图";
            Logger::error("Failed to generate thumbnail: " + thumbnailPath);
            stbi_image_free(data);
            return result;
        }
        
        // 6. 释放内存
        stbi_image_free(data);
        
        // 7. 获取压缩后的文件大小
        long long compressedSize = getFileSize(originalPath);
        
        // 8. 构建结果
        result.success = true;
        result.originalPath = originalPath;
        result.thumbnailPath = thumbnailPath;
        result.width = width;
        result.height = height;
        result.fileSize = compressedSize;
        
        Logger::info("Image processed successfully: " + filename);
        return result;
        
    } catch (const std::exception& e) {
        result.message = "处理图片时发生异常: " + std::string(e.what());
        Logger::error("Exception in processImage: " + std::string(e.what()));
        return result;
    }
}

// 生成缩略图
bool ImageProcessor::generateThumbnail(unsigned char* inputData,
                                       int width,
                                       int height,
                                       int channels,
                                       const std::string& outputPath) {
    try {
        // 计算缩略图尺寸（保持宽高比）
        int thumbWidth, thumbHeight;
        
        if (width > height) {
            thumbWidth = THUMBNAIL_SIZE;
            thumbHeight = (height * THUMBNAIL_SIZE) / width;
        } else {
            thumbHeight = THUMBNAIL_SIZE;
            thumbWidth = (width * THUMBNAIL_SIZE) / height;
        }
        
        // 分配缩略图内存
        unsigned char* thumbData = new unsigned char[thumbWidth * thumbHeight * channels];
        
        // 缩放图片
        stbir_resize_uint8_linear(inputData, width, height, 0,
                                  thumbData, thumbWidth, thumbHeight, 0,
                                  static_cast<stbir_pixel_layout>(channels));
        
        // 保存缩略图
        int saveResult = stbi_write_jpg(outputPath.c_str(), thumbWidth, thumbHeight, channels, thumbData, JPEG_QUALITY);
        
        // 释放内存
        delete[] thumbData;
        
        return saveResult != 0;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in generateThumbnail: " + std::string(e.what()));
        return false;
    }
}

// 验证图片格式
bool ImageProcessor::validateFormat(const std::string& filePath) {
    // 获取文件扩展名
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    
    std::string ext = filePath.substr(dotPos);
    
    // 转换为小写
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // 检查扩展名
    return (ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".webp");
}

// 获取图片尺寸
std::pair<int, int> ImageProcessor::getImageDimensions(const std::string& filePath) {
    int width, height, channels;
    
    // 只获取图片信息，不加载数据
    if (stbi_info(filePath.c_str(), &width, &height, &channels)) {
        return {width, height};
    }
    
    return {0, 0};
}

// 获取文件大小
long long ImageProcessor::getFileSize(const std::string& filePath) {
    struct stat stat_buf;
    int rc = stat(filePath.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : 0;
}

// 获取MIME类型
std::string ImageProcessor::getMimeType(const std::string& filePath) {
    // 获取文件扩展名
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string ext = filePath.substr(dotPos);
    
    // 转换为小写
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // 根据扩展名返回MIME类型
    if (ext == ".jpg" || ext == ".jpeg") {
        return "image/jpeg";
    } else if (ext == ".png") {
        return "image/png";
    } else if (ext == ".webp") {
        return "image/webp";
    } else {
        return "application/octet-stream";
    }
}

