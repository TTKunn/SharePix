/**
 * @file id_generator.h
 * @brief ID生成工具类
 * @author Knot Team
 * @date 2025-10-07
 */

#pragma once

#include <string>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>

/**
 * @brief ID生成工具类
 * 
 * 提供业务ID和文件名的生成功能
 */
class IdGenerator {
public:
    /**
     * @brief 生成图片业务ID
     * 
     * 格式：IMG_YYYYQX_XXXXXX
     * 例如：IMG_2025Q4_ABC123
     * 
     * @return 生成的图片业务ID
     */
    static std::string generateImageId();
    
    /**
     * @brief 生成唯一文件名
     * 
     * 使用UUID格式生成唯一文件名，避免文件名冲突
     * 
     * @param extension 文件扩展名（包含点号，如".jpg"）
     * @return 生成的文件名（UUID + 扩展名）
     */
    static std::string generateFileName(const std::string& extension);

private:
    /**
     * @brief 生成随机字符串
     * 
     * @param length 字符串长度
     * @return 随机字符串（大写字母和数字）
     */
    static std::string generateRandomString(int length);
    
    /**
     * @brief 生成UUID格式的字符串
     * 
     * @return UUID字符串（格式：xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx）
     */
    static std::string generateUUID();
};

