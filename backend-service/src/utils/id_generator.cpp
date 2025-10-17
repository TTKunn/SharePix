/**
 * @file id_generator.cpp
 * @brief ID生成工具类实现
 * @author Knot Team
 * @date 2025-10-07
 */

#include "id_generator.h"
#include "base62_encoder.h"
#include "logger.h"
#include <mutex>

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

// 生成分享链接短码
std::string IdGenerator::generateShareCode() {
    // 生成雪花ID
    int64_t snowflakeId = generateSnowflakeId();
    
    // Base62编码为8位短码
    std::string shortCode = Base62Encoder::encode(snowflakeId, 8);
    
    Logger::debug("Generated share code: " + shortCode + " from snowflake ID: " + std::to_string(snowflakeId));
    
    return shortCode;
}

// 生成雪花ID（简化版单机实现）
int64_t IdGenerator::generateSnowflakeId() {
    // 雪花ID常量
    static const int64_t EPOCH = 1609459200000LL; // 2021-01-01 00:00:00 UTC
    static const int64_t MACHINE_ID_BITS = 10;
    static const int64_t SEQUENCE_BITS = 12;
    static const int64_t MAX_SEQUENCE = (1LL << SEQUENCE_BITS) - 1;  // 4095
    static const int64_t MACHINE_ID = 0;  // 单机版使用固定机器ID
    
    // 静态变量保存状态
    static int64_t lastTimestamp = -1;
    static int64_t sequence = 0;
    static std::mutex mutex;
    
    std::lock_guard<std::mutex> lock(mutex);
    
    // 获取当前时间戳（毫秒）
    auto now = std::chrono::system_clock::now();
    int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
    
    // 时钟回拨检测
    if (timestamp < lastTimestamp) {
        Logger::error("Clock moved backwards! Refusing to generate ID");
        throw std::runtime_error("Clock moved backwards, refusing to generate snowflake ID");
    }
    
    // 同一毫秒内，序列号自增
    if (timestamp == lastTimestamp) {
        sequence = (sequence + 1) & MAX_SEQUENCE;
        
        // 序列号溢出，等待下一毫秒
        if (sequence == 0) {
            // 忙等待到下一毫秒
            while (timestamp <= lastTimestamp) {
                now = std::chrono::system_clock::now();
                timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()
                ).count();
            }
        }
    } else {
        // 新的毫秒，序列号重置
        sequence = 0;
    }
    
    lastTimestamp = timestamp;
    
    // 组装64位ID
    // [0 - 1位符号] [41位时间戳] [10位机器ID] [12位序列号]
    int64_t id = ((timestamp - EPOCH) << (MACHINE_ID_BITS + SEQUENCE_BITS))
               | (MACHINE_ID << SEQUENCE_BITS)
               | sequence;
    
    return id;
}

