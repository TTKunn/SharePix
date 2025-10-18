/**
 * @file avatar_processor.h
 * @brief 头像处理工具类定义
 * @author Knot Team
 * @date 2025-10-18
 */

#pragma once

#include <string>
#include <utility>

/**
 * @brief 头像处理结果结构体
 */
struct AvatarProcessResult {
    bool success;              // 处理是否成功
    std::string message;       // 错误信息（如果失败）
    std::string avatarPath;    // 头像路径（相对路径，例：/uploads/avatars/USR_xxx_1234567890.jpg）
    int width;                 // 图片宽度（固定200）
    int height;                // 图片高度（固定200）
    long long fileSize;        // 文件大小（字节）
    
    AvatarProcessResult() : success(false), width(0), height(0), fileSize(0) {}
};

/**
 * @brief 头像处理工具类
 * 
 * 专门用于处理用户头像上传，包括裁剪、压缩、保存
 * 技术栈：stb_image, stb_image_resize, stb_image_write
 */
class AvatarProcessor {
public:
    /**
     * @brief 处理头像图片（裁剪为正方形 + 缩放 + 压缩）
     * 
     * 处理流程：
     * 1. 加载图片
     * 2. 验证格式和大小
     * 3. 如果不是正方形，居中裁剪为正方形
     * 4. 缩放到200x200
     * 5. 保存为JPEG（80%质量）
     * 
     * @param inputPath 输入图片路径（临时文件）
     * @param userId 用户逻辑ID（用于生成文件名，例：USR_2025Q4_abc123）
     * @param outputDir 输出目录（例：uploads/avatars/）
     * @return AvatarProcessResult 处理结果
     */
    static AvatarProcessResult processAvatar(
        const std::string& inputPath,
        const std::string& userId,
        const std::string& outputDir
    );
    
    /**
     * @brief 删除旧头像文件
     * 
     * @param avatarUrl 旧头像URL（相对路径，例：/uploads/avatars/USR_xxx_1234567890.jpg）
     * @return bool 成功返回true，失败或文件不存在返回false
     */
    static bool deleteOldAvatar(const std::string& avatarUrl);
    
    /**
     * @brief 验证头像文件格式和大小
     * 
     * @param filePath 文件路径
     * @param maxSize 最大文件大小（字节），默认5MB
     * @return std::pair<bool, std::string> {是否有效, 错误信息}
     */
    static std::pair<bool, std::string> validateAvatarFile(
        const std::string& filePath,
        long long maxSize = 5 * 1024 * 1024
    );

private:
    // 常量定义
    static constexpr int AVATAR_SIZE = 200;        // 头像尺寸（200x200）
    static constexpr int JPEG_QUALITY = 80;        // JPEG压缩质量（80%）
    static constexpr long long MAX_FILE_SIZE = 5 * 1024 * 1024;  // 最大文件大小（5MB）
    
    /**
     * @brief 裁剪图片为正方形（居中裁剪）
     * 
     * @param inputData 输入图片数据
     * @param width 原图宽度
     * @param height 原图高度
     * @param channels 通道数
     * @param outSize 输出正方形边长（返回值）
     * @return unsigned char* 裁剪后的图片数据（需要手动释放）
     */
    static unsigned char* cropToSquare(
        unsigned char* inputData,
        int width,
        int height,
        int channels,
        int& outSize
    );
};

