/**
 * @file base64_decoder.h
 * @brief Base64编解码工具
 * @author Knot Team
 * @date 2025-10-14
 */

#pragma once

#include <string>
#include <vector>

/**
 * @brief Base64编解码工具类
 */
class Base64Decoder {
public:
    /**
     * @brief 检查字符串是否为Base64格式
     * @param str 待检查的字符串
     * @return 如果是Base64格式返回true
     */
    static bool isBase64(const std::string& str);

    /**
     * @brief 解码Base64字符串为二进制数据
     * @param encoded Base64编码的字符串
     * @return 解码后的二进制数据
     */
    static std::string decode(const std::string& encoded);

    /**
     * @brief 编码二进制数据为Base64字符串
     * @param data 二进制数据
     * @return Base64编码的字符串
     */
    static std::string encode(const std::string& data);

private:
    static const std::string base64_chars;
    
    static inline bool is_base64(unsigned char c) {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }
};

