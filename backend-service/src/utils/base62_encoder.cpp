/**
 * @file base62_encoder.cpp
 * @brief Base62编码工具类实现
 * @author Knot Development Team
 * @date 2025-10-16
 */

#include "utils/base62_encoder.h"
#include "utils/logger.h"

// Base62字符集定义: 0-9(0-9), A-Z(10-35), a-z(36-61)
const char Base62Encoder::BASE62_CHARS[] = 
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

std::string Base62Encoder::encode(int64_t num, int minLength) {
    // 处理0的特殊情况
    if (num == 0) {
        return std::string(minLength, '0');
    }
    
    // 处理负数
    if (num < 0) {
        Logger::error("Base62Encoder: Cannot encode negative number: " + std::to_string(num));
        throw std::invalid_argument("Cannot encode negative number");
    }
    
    std::string result;
    
    // 转换为Base62
    while (num > 0) {
        int remainder = num % BASE;
        result = BASE62_CHARS[remainder] + result;
        num /= BASE;
    }
    
    // 填充到最小长度
    while (static_cast<int>(result.length()) < minLength) {
        result = '0' + result;
    }
    
    return result;
}

int64_t Base62Encoder::decode(const std::string& str) {
    if (str.empty()) {
        Logger::error("Base62Encoder: Cannot decode empty string");
        throw std::invalid_argument("Cannot decode empty string");
    }
    
    int64_t result = 0;
    
    for (char c : str) {
        int value = charToValue(c);
        
        // 检查溢出
        if (result > (INT64_MAX / BASE)) {
            Logger::error("Base62Encoder: Decode overflow for string: " + str);
            throw std::overflow_error("Decode result too large");
        }
        
        result = result * BASE + value;
    }
    
    return result;
}

int Base62Encoder::charToValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';  // 0-9
    } else if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;  // 10-35
    } else if (c >= 'a' && c <= 'z') {
        return c - 'a' + 36;  // 36-61
    } else {
        Logger::error("Base62Encoder: Invalid character: " + std::string(1, c));
        throw std::invalid_argument("Invalid Base62 character: " + std::string(1, c));
    }
}





