/**
 * @file image_processor.h
 * @brief 图片处理工具类定义
 * @author Knot Team
 * @date 2025-10-07
 */

#pragma once

#include <string>
#include <utility>

/**
 * @brief 图片处理结果结构体
 */
struct ProcessResult {
    bool success;                  // 处理是否成功
    std::string message;           // 错误信息（如果失败）
    std::string originalPath;      // 压缩后的原图路径
    std::string thumbnailPath;     // 缩略图路径
    int width;                     // 图片宽度
    int height;                    // 图片高度
    long long fileSize;            // 文件大小（字节）
    
    ProcessResult() : success(false), width(0), height(0), fileSize(0) {}
};

/**
 * @brief 图片处理工具类
 * 
 * 使用stb_image库实现图片压缩、缩略图生成、格式验证等功能
 */
class ImageProcessor {
public:
    /**
     * @brief 处理图片（压缩 + 生成缩略图）
     * 
     * @param inputPath 输入图片路径
     * @param outputDir 输出目录（例：uploads/images/）
     * @param thumbnailDir 缩略图目录（例：uploads/thumbnails/）
     * @param filename 输出文件名（不含扩展名）
     * @return ProcessResult 处理结果
     */
    static ProcessResult processImage(const std::string& inputPath,
                                      const std::string& outputDir,
                                      const std::string& thumbnailDir,
                                      const std::string& filename);
    
    /**
     * @brief 验证图片格式
     * 
     * @param filePath 文件路径
     * @return true 如果格式有效，false 否则
     */
    static bool validateFormat(const std::string& filePath);
    
    /**
     * @brief 获取图片尺寸
     * 
     * @param filePath 文件路径
     * @return std::pair<int, int> 宽度和高度（如果失败返回{0, 0}）
     */
    static std::pair<int, int> getImageDimensions(const std::string& filePath);
    
    /**
     * @brief 获取文件大小
     * 
     * @param filePath 文件路径
     * @return long long 文件大小（字节），失败返回0
     */
    static long long getFileSize(const std::string& filePath);
    
    /**
     * @brief 获取MIME类型
     * 
     * @param filePath 文件路径
     * @return std::string MIME类型（例：image/jpeg）
     */
    static std::string getMimeType(const std::string& filePath);

private:
    // 常量定义
    static constexpr int THUMBNAIL_SIZE = 300;      // 缩略图尺寸（300x300）
    static constexpr int JPEG_QUALITY = 80;         // JPEG压缩质量（80%）
    static constexpr long long MAX_FILE_SIZE = 5 * 1024 * 1024;  // 最大文件大小（5MB）
    
    /**
     * @brief 生成缩略图
     * 
     * @param inputData 输入图片数据
     * @param width 原图宽度
     * @param height 原图高度
     * @param channels 通道数
     * @param outputPath 输出路径
     * @return bool 成功返回true，失败返回false
     */
    static bool generateThumbnail(unsigned char* inputData,
                                  int width,
                                  int height,
                                  int channels,
                                  const std::string& outputPath);
};

