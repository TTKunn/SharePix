/**
 * @file base64_decoder.cpp
 * @brief Base64编解码工具实现
 * @author Knot Team
 * @date 2025-10-14
 */

#include "base64_decoder.h"
#include <algorithm>
#include <stdexcept>

const std::string Base64Decoder::base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// 检查字符串是否为Base64格式
bool Base64Decoder::isBase64(const std::string& str) {
    if (str.empty()) {
        return false;
    }

    // 首先检查是否为Data URI格式
    if (str.length() > 5 && str.substr(0, 5) == "data:") {
        size_t comma_pos = str.find(',');
        if (comma_pos != std::string::npos && comma_pos < 100) {
            // 检查是否包含base64标识
            std::string header = str.substr(0, comma_pos);
            if (header.find("base64") != std::string::npos) {
                return true;
            }
        }
    }

    // Base64字符串长度必须是4的倍数（或末尾有=填充）
    size_t len = str.length();
    if (len < 4) {
        return false;
    }

    // 检查是否包含非Base64字符（忽略末尾的=）
    size_t checkLen = len;
    while (checkLen > 0 && str[checkLen - 1] == '=') {
        checkLen--;
    }

    for (size_t i = 0; i < checkLen; i++) {
        unsigned char c = str[i];
        if (!is_base64(c)) {
            return false;
        }
    }

    // 长度是4的倍数且全是Base64字符，判定为Base64
    return (len % 4 == 0 || (len >= 100 && checkLen > len - 3));
}

// 解码Base64字符串
std::string Base64Decoder::decode(const std::string& encoded_string) {
    // 使用const引用避免复制，找到实际Base64数据的起始位置
    const std::string* encoded_ptr = &encoded_string;
    size_t start_pos = 0;
    
    // 移除data URI前缀（如果存在）
    size_t comma_pos = encoded_string.find(',');
    if (comma_pos != std::string::npos && comma_pos < 100) {
        // 可能是 data:image/xxx;base64,xxxxx 格式
        if (encoded_string.substr(0, 5) == "data:") {
            start_pos = comma_pos + 1;
        }
    }

    // 计算实际编码数据长度
    size_t encoded_len = encoded_string.size() - start_pos;
    size_t in_len = encoded_len;
    size_t i = 0;
    size_t in_ = start_pos;
    unsigned char char_array_4[4], char_array_3[3];
    
    // 🔧 关键修复1: 预分配内存，避免频繁重新分配
    // Base64解码后大小约为原始的3/4
    std::string ret;
    ret.reserve((encoded_len * 3) / 4 + 4);  // +4是为了安全

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            // 🔧 关键修复2: 使用append而不是+=，效率更高
            ret.append(reinterpret_cast<const char*>(char_array_3), 3);
            i = 0;
        }
    }

    if (i) {
        size_t j = 0;
        for (j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (j = 0; j < 4; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        // 🔧 关键修复3: 同样使用append
        ret.append(reinterpret_cast<const char*>(char_array_3), i - 1);
    }

    return ret;
}

// 编码二进制数据为Base64
std::string Base64Decoder::encode(const std::string& data) {
    size_t in_len = data.size();
    size_t i = 0;
    size_t j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    std::string ret;

    const unsigned char* bytes_to_encode = reinterpret_cast<const unsigned char*>(data.c_str());

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; i < 4; i++) {
                ret += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++) {
            ret += base64_chars[char_array_4[j]];
        }

        while (i++ < 3) {
            ret += '=';
        }
    }

    return ret;
}

