/**
 * @file avatar_processor.cpp
 * @brief 头像处理工具类实现
 * @author Knot Team
 * @date 2025-10-18
 */

#include "avatar_processor.h"
#include "logger.h"

// 不定义IMPLEMENTATION宏，因为image_processor.cpp已经定义了
#include "stb_image.h"
#include "stb_image_resize2.h"
#include "stb_image_write.h"

#include <ctime>
#include <cstdio>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

// 处理头像图片
AvatarProcessResult AvatarProcessor::processAvatar(
    const std::string& inputPath,
    const std::string& userId,
    const std::string& outputDir
) {
    AvatarProcessResult result;
    
    try {
        Logger::info("开始处理头像: userId=" + userId + ", inputPath=" + inputPath);
        
        // 1. 验证文件
        auto [valid, errorMsg] = validateAvatarFile(inputPath);
        if (!valid) {
            result.message = errorMsg;
            Logger::error("头像文件验证失败: " + errorMsg);
            return result;
        }
        
        // 2. 加载图片
        int width, height, channels;
        unsigned char* imageData = stbi_load(inputPath.c_str(), &width, &height, &channels, 0);
        
        if (!imageData) {
            result.message = "无法加载图片文件";
            Logger::error("stbi_load失败: " + inputPath);
            return result;
        }
        
        Logger::info("图片加载成功: " + std::to_string(width) + "x" + std::to_string(height) + ", channels=" + std::to_string(channels));
        
        // 3. 裁剪为正方形（如果需要）
        unsigned char* squareData = nullptr;
        int squareSize = 0;
        
        if (width != height) {
            squareData = cropToSquare(imageData, width, height, channels, squareSize);
            stbi_image_free(imageData);  // 释放原图数据
            
            if (!squareData) {
                result.message = "图片裁剪失败";
                Logger::error("cropToSquare失败");
                return result;
            }
            
            Logger::info("图片裁剪成功: " + std::to_string(squareSize) + "x" + std::to_string(squareSize));
        } else {
            squareData = imageData;
            squareSize = width;
        }
        
        // 4. 缩放到200x200
        unsigned char* resizedData = (unsigned char*)malloc(AVATAR_SIZE * AVATAR_SIZE * channels);
        if (!resizedData) {
            result.message = "内存分配失败";
            Logger::error("malloc失败");
            if (squareData != imageData) free(squareData);
            return result;
        }
        
        // 根据通道数选择pixel layout
        stbir_pixel_layout pixelLayout;
        if (channels == 1) {
            pixelLayout = STBIR_1CHANNEL;
        } else if (channels == 3) {
            pixelLayout = STBIR_RGB;
        } else if (channels == 4) {
            pixelLayout = STBIR_RGBA;
        } else {
            result.message = "不支持的图片通道数: " + std::to_string(channels);
            Logger::error(result.message);
            free(resizedData);
            if (squareData != imageData) free(squareData);
            return result;
        }
        
        unsigned char* resizeResult = stbir_resize_uint8_linear(
            squareData, squareSize, squareSize, 0,
            resizedData, AVATAR_SIZE, AVATAR_SIZE, 0,
            pixelLayout
        );
        
        // 释放正方形数据
        if (squareData != imageData) {
            free(squareData);
        }
        
        if (!resizeResult) {
            result.message = "图片缩放失败";
            Logger::error("stbir_resize_uint8_linear失败");
            free(resizedData);
            return result;
        }
        
        Logger::info("图片缩放成功: " + std::to_string(AVATAR_SIZE) + "x" + std::to_string(AVATAR_SIZE));
        
        // 5. 生成输出文件名
        std::time_t now = std::time(nullptr);
        std::string filename = userId + "_" + std::to_string(now) + ".jpg";
        std::string outputPath = outputDir + filename;
        
        // 6. 保存为JPEG
        int writeResult = stbi_write_jpg(
            outputPath.c_str(),
            AVATAR_SIZE,
            AVATAR_SIZE,
            channels,
            resizedData,
            JPEG_QUALITY
        );
        
        free(resizedData);  // 释放缩放后的数据
        
        if (!writeResult) {
            result.message = "保存图片失败";
            Logger::error("stbi_write_jpg失败: " + outputPath);
            return result;
        }
        
        // 7. 获取文件大小
        struct stat fileStat;
        if (stat(outputPath.c_str(), &fileStat) == 0) {
            result.fileSize = fileStat.st_size;
        }
        
        // 8. 填充结果
        result.success = true;
        result.message = "头像处理成功";
        result.avatarPath = "/uploads/avatars/" + filename;
        result.width = AVATAR_SIZE;
        result.height = AVATAR_SIZE;
        
        Logger::info("头像处理完成: " + result.avatarPath + ", fileSize=" + std::to_string(result.fileSize));
        
        return result;
        
    } catch (const std::exception& e) {
        result.message = "头像处理异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

// 删除旧头像文件
bool AvatarProcessor::deleteOldAvatar(const std::string& avatarUrl) {
    if (avatarUrl.empty() || avatarUrl.find("/uploads/avatars/") == std::string::npos) {
        return false;  // 不是头像文件路径，不处理
    }
    
    // 从URL中提取路径部分（可能包含域名或只是路径）
    std::string path = avatarUrl;
    size_t uploadsPos = path.find("/uploads/avatars/");
    if (uploadsPos != std::string::npos) {
        path = path.substr(uploadsPos);  // 提取 "/uploads/avatars/xxx.jpg"
    }
    
    // 构建完整文件路径：相对于backend-service目录，需要加 "../"
    std::string filePath = ".." + path;  // "../uploads/avatars/xxx.jpg"
    
    if (unlink(filePath.c_str()) == 0) {
        Logger::info("旧头像已删除: " + avatarUrl);
        return true;
    } else {
        Logger::warning("删除旧头像失败: " + avatarUrl + " (文件可能不存在)");
        return false;
    }
}

// 验证头像文件
std::pair<bool, std::string> AvatarProcessor::validateAvatarFile(
    const std::string& filePath,
    long long maxSize
) {
    // 1. 检查文件是否存在
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        return {false, "文件不存在"};
    }
    
    // 2. 检查文件大小
    if (fileStat.st_size > maxSize) {
        return {false, "文件大小超过限制（最大5MB）"};
    }
    
    if (fileStat.st_size == 0) {
        return {false, "文件为空"};
    }
    
    // 3. 检查文件格式（通过stb_image尝试加载）
    int width, height, channels;
    if (!stbi_info(filePath.c_str(), &width, &height, &channels)) {
        return {false, "不支持的文件格式，仅支持JPEG/PNG/GIF/WebP"};
    }
    
    // 4. 检查图片尺寸合法性
    if (width <= 0 || height <= 0) {
        return {false, "无效的图片尺寸"};
    }
    
    if (width > 10000 || height > 10000) {
        return {false, "图片尺寸过大（最大10000x10000）"};
    }
    
    return {true, ""};
}

// 裁剪图片为正方形（居中裁剪）
unsigned char* AvatarProcessor::cropToSquare(
    unsigned char* inputData,
    int width,
    int height,
    int channels,
    int& outSize
) {
    if (!inputData || width <= 0 || height <= 0 || channels <= 0) {
        return nullptr;
    }
    
    // 计算正方形边长（取较小值）
    int size = (width < height) ? width : height;
    outSize = size;
    
    // 计算裁剪起始位置（居中）
    int offsetX = (width - size) / 2;
    int offsetY = (height - size) / 2;
    
    // 分配输出内存
    unsigned char* outputData = (unsigned char*)malloc(size * size * channels);
    if (!outputData) {
        return nullptr;
    }
    
    // 逐行复制像素数据
    for (int y = 0; y < size; y++) {
        int srcY = offsetY + y;
        int srcOffset = (srcY * width + offsetX) * channels;
        int dstOffset = (y * size) * channels;
        
        memcpy(
            outputData + dstOffset,
            inputData + srcOffset,
            size * channels
        );
    }
    
    return outputData;
}

