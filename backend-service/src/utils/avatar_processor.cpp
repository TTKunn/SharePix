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

        // 2. 加载图片（使用RAII智能指针自动管理内存）
        int width, height, channels;
        StbiPtr imageData(stbi_load(inputPath.c_str(), &width, &height, &channels, 0));

        if (!imageData) {
            result.message = "无法加载图片文件";
            Logger::error("stbi_load失败: " + inputPath);
            return result;
        }

        Logger::info("图片加载成功: " + std::to_string(width) + "x" + std::to_string(height) + ", channels=" + std::to_string(channels));

        // 3. 裁剪为正方形（如果需要）
        // 使用裸指针指向当前使用的数据（可能是imageData或cropToSquare的结果）
        unsigned char* squareDataPtr = nullptr;
        MallocPtr croppedData;  // 仅在裁剪时持有malloc分配的内存
        int squareSize = 0;

        if (width != height) {
            // 需要裁剪，cropToSquare返回malloc分配的内存
            unsigned char* rawCroppedData = cropToSquare(imageData.get(), width, height, channels, squareSize);

            if (!rawCroppedData) {
                result.message = "图片裁剪失败";
                Logger::error("cropToSquare失败");
                return result;  // imageData自动释放
            }

            // 将malloc分配的内存交给智能指针管理
            croppedData.reset(rawCroppedData);
            squareDataPtr = croppedData.get();

            // 释放原图数据（不再需要）
            imageData.reset();

            Logger::info("图片裁剪成功: " + std::to_string(squareSize) + "x" + std::to_string(squareSize));
        } else {
            // 已经是正方形，直接使用imageData
            squareDataPtr = imageData.get();
            squareSize = width;
            Logger::info("图片已是正方形，跳过裁剪");
        }

        // 4. 缩放到200x200（使用RAII智能指针）
        MallocPtr resizedData((unsigned char*)malloc(AVATAR_SIZE * AVATAR_SIZE * channels));
        if (!resizedData) {
            result.message = "内存分配失败";
            Logger::error("malloc失败");
            return result;  // 所有智能指针自动释放
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
            return result;  // 所有智能指针自动释放
        }

        unsigned char* resizeResult = stbir_resize_uint8_linear(
            squareDataPtr, squareSize, squareSize, 0,
            resizedData.get(), AVATAR_SIZE, AVATAR_SIZE, 0,
            pixelLayout
        );

        if (!resizeResult) {
            result.message = "图片缩放失败";
            Logger::error("stbir_resize_uint8_linear失败");
            return result;  // 所有智能指针自动释放
        }

        Logger::info("图片缩放成功: " + std::to_string(AVATAR_SIZE) + "x" + std::to_string(AVATAR_SIZE));

        // 5. 生成输出文件名
        std::time_t now = std::time(nullptr);
        std::string filename = userId + "_" + std::to_string(now) + ".jpg";

        // 确保输出目录以斜杠结尾
        std::string normalizedOutputDir = outputDir;
        if (!normalizedOutputDir.empty() && normalizedOutputDir.back() != '/') {
            normalizedOutputDir += "/";
        }

        std::string outputPath = normalizedOutputDir + filename;

        // 6. 保存为JPEG
        int writeResult = stbi_write_jpg(
            outputPath.c_str(),
            AVATAR_SIZE,
            AVATAR_SIZE,
            channels,
            resizedData.get(),
            JPEG_QUALITY
        );

        if (!writeResult) {
            result.message = "保存图片失败";
            Logger::error("stbi_write_jpg失败: " + outputPath);
            return result;  // 所有智能指针自动释放
        }

        // 7. 获取文件大小
        struct stat fileStat;
        if (stat(outputPath.c_str(), &fileStat) == 0) {
            result.fileSize = fileStat.st_size;
        }

        // 8. 填充结果
        result.success = true;
        result.message = "头像处理成功";

        // 将物理路径转换为HTTP访问路径
        // 物理路径: "../uploads/avatars/USR_xxx.jpg" 或 "uploads/avatars/USR_xxx.jpg"
        // HTTP路径: "/uploads/avatars/USR_xxx.jpg"
        auto convertToHttpPath = [](const std::string& physicalPath) -> std::string {
            // 查找 "avatars/" 的位置
            size_t avatarsPos = physicalPath.find("avatars/");

            if (avatarsPos != std::string::npos) {
                // 提取 "avatars/filename.jpg" 部分，并添加 "/uploads/" 前缀
                return "/uploads/" + physicalPath.substr(avatarsPos);
            }

            // 如果没有找到，检查是否已经是标准HTTP路径（以 /uploads/ 开头）
            if (physicalPath.find("/uploads/") == 0) {
                return physicalPath;
            }

            // 兜底：确保以斜杠开头
            if (!physicalPath.empty() && physicalPath[0] != '/') {
                return "/" + physicalPath;
            }

            return physicalPath;
        };

        result.avatarPath = convertToHttpPath(outputPath);
        result.width = AVATAR_SIZE;
        result.height = AVATAR_SIZE;

        Logger::info("头像处理完成: " + result.avatarPath + ", fileSize=" + std::to_string(result.fileSize));

        return result;
        // 函数结束时，所有智能指针自动释放内存

    } catch (const std::exception& e) {
        result.message = "头像处理异常: " + std::string(e.what());
        Logger::error(result.message);
        // 异常情况下，所有智能指针也会自动释放
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

