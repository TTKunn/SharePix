/**
 * @file id_generator.cpp
 * @brief ID生成工具类实现
 * @author Knot Team
 * @date 2025-10-07
 */

#include "id_generator.h"

// 生成图片业务ID
std::string IdGenerator::generateImageId() {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time_t);
    
    // 计算季度（1-4）
    int quarter = (tm->tm_mon / 3) + 1;
    
    // 生成6位随机字符
    std::string random = generateRandomString(6);
    
    // 构建业务ID：IMG_YYYYQX_XXXXXX
    std::ostringstream oss;
    oss << "IMG_" << (1900 + tm->tm_year) << "Q" << quarter << "_" << random;
    
    return oss.str();
}

// 生成唯一文件名
std::string IdGenerator::generateFileName(const std::string& extension) {
    // 生成UUID
    std::string uuid = generateUUID();
    
    // 拼接扩展名
    return uuid + extension;
}

// 生成随机字符串（大写字母和数字）
std::string IdGenerator::generateRandomString(int length) {
    static const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static const int charsetSize = sizeof(charset) - 1;
    
    // 使用随机设备作为种子
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charsetSize - 1);
    
    std::string result;
    result.reserve(length);
    
    for (int i = 0; i < length; i++) {
        result += charset[dis(gen)];
    }
    
    return result;
}

// 生成UUID格式的字符串
std::string IdGenerator::generateUUID() {
    // 使用随机设备生成UUID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);
    
    std::ostringstream oss;
    oss << std::hex;
    
    // UUID格式：xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx
    // 其中4表示版本4（随机UUID），y的值为8、9、a或b
    
    for (int i = 0; i < 8; i++) {
        oss << dis(gen);
    }
    oss << "-";
    
    for (int i = 0; i < 4; i++) {
        oss << dis(gen);
    }
    oss << "-";
    
    // 版本4：第13位固定为4
    oss << "4";
    for (int i = 0; i < 3; i++) {
        oss << dis(gen);
    }
    oss << "-";
    
    // 变体：第17位为8、9、a或b
    oss << dis2(gen);
    for (int i = 0; i < 3; i++) {
        oss << dis(gen);
    }
    oss << "-";
    
    for (int i = 0; i < 12; i++) {
        oss << dis(gen);
    }
    
    return oss.str();
}

