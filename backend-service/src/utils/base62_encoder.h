/**
 * @file base62_encoder.h
 * @brief Base62编码工具类，用于生成短链接
 * @author Knot Development Team
 * @date 2025-10-16
 */

#pragma once

#include <string>
#include <stdexcept>
#include <cstdint>

/**
 * @brief Base62编码工具类
 * 
 * 使用字符集: 0-9A-Za-z (62个字符)
 * 用于将64位整数编码为短字符串，适合URL使用
 */
class Base62Encoder {
public:
    /**
     * @brief 将int64编码为Base62字符串
     * @param num 原始数字
     * @param minLength 最小长度（不足补0），默认8位
     * @return Base62编码字符串
     */
    static std::string encode(int64_t num, int minLength = 8);

    /**
     * @brief 将Base62字符串解码为int64
     * @param str Base62字符串
     * @return 解码后的数字
     * @throws std::invalid_argument 如果字符串包含非法字符
     */
    static int64_t decode(const std::string& str);

private:
    // Base62字符集: 0-9A-Za-z
    static const char BASE62_CHARS[];
    static const int BASE = 62;
    
    /**
     * @brief 获取字符对应的数值
     * @param c 字符
     * @return 对应的数值（0-61）
     * @throws std::invalid_argument 如果字符非法
     */
    static int charToValue(char c);
};





