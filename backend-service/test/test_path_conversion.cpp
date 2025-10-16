/**
 * 测试文件: test_path_conversion.cpp
 * 测试目的: 验证物理路径到HTTP路径的转换逻辑
 * 创建时间: 2025-10-15
 * 测试完成后删除
 */

#include <iostream>
#include <string>
#include <cassert>

// 路径转换函数（与image_service.cpp中的逻辑相同）
std::string convertToHttpPath(const std::string& physicalPath) {
    // 查找 "images/" 或 "thumbnails/" 的位置
    size_t imagesPos = physicalPath.find("images/");
    size_t thumbnailsPos = physicalPath.find("thumbnails/");
    
    if (imagesPos != std::string::npos) {
        // 提取 "images/filename.jpg" 部分，并添加 "/uploads/" 前缀
        return "/uploads/" + physicalPath.substr(imagesPos);
    } else if (thumbnailsPos != std::string::npos) {
        // 提取 "thumbnails/filename.jpg" 部分，并添加 "/uploads/" 前缀
        return "/uploads/" + physicalPath.substr(thumbnailsPos);
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
}

void testPathConversion() {
    std::cout << "=== 路径转换测试 ===" << std::endl;
    
    // 测试用例1：相对路径（新配置）
    std::string test1 = "../uploads/images/IMG_2025Q4_ABC123.jpg";
    std::string expected1 = "/uploads/images/IMG_2025Q4_ABC123.jpg";
    std::string result1 = convertToHttpPath(test1);
    std::cout << "测试1 - 相对路径:" << std::endl;
    std::cout << "  输入: " << test1 << std::endl;
    std::cout << "  输出: " << result1 << std::endl;
    std::cout << "  期望: " << expected1 << std::endl;
    std::cout << "  结果: " << (result1 == expected1 ? "✓ 通过" : "✗ 失败") << std::endl;
    assert(result1 == expected1);
    std::cout << std::endl;
    
    // 测试用例2：相对路径（缩略图）
    std::string test2 = "../uploads/thumbnails/IMG_2025Q4_ABC123_thumb.jpg";
    std::string expected2 = "/uploads/thumbnails/IMG_2025Q4_ABC123_thumb.jpg";
    std::string result2 = convertToHttpPath(test2);
    std::cout << "测试2 - 相对路径（缩略图）:" << std::endl;
    std::cout << "  输入: " << test2 << std::endl;
    std::cout << "  输出: " << result2 << std::endl;
    std::cout << "  期望: " << expected2 << std::endl;
    std::cout << "  结果: " << (result2 == expected2 ? "✓ 通过" : "✗ 失败") << std::endl;
    assert(result2 == expected2);
    std::cout << std::endl;
    
    // 测试用例3：旧格式（当前目录相对路径）
    std::string test3 = "uploads/images/IMG_2025Q4_ABC123.jpg";
    std::string expected3 = "/uploads/images/IMG_2025Q4_ABC123.jpg";
    std::string result3 = convertToHttpPath(test3);
    std::cout << "测试3 - 旧格式（相对路径）:" << std::endl;
    std::cout << "  输入: " << test3 << std::endl;
    std::cout << "  输出: " << result3 << std::endl;
    std::cout << "  期望: " << expected3 << std::endl;
    std::cout << "  结果: " << (result3 == expected3 ? "✓ 通过" : "✗ 失败") << std::endl;
    assert(result3 == expected3);
    std::cout << std::endl;
    
    // 测试用例4：已经是HTTP路径
    std::string test4 = "/uploads/images/IMG_2025Q4_ABC123.jpg";
    std::string expected4 = "/uploads/images/IMG_2025Q4_ABC123.jpg";
    std::string result4 = convertToHttpPath(test4);
    std::cout << "测试4 - 已经是HTTP路径:" << std::endl;
    std::cout << "  输入: " << test4 << std::endl;
    std::cout << "  输出: " << result4 << std::endl;
    std::cout << "  期望: " << expected4 << std::endl;
    std::cout << "  结果: " << (result4 == expected4 ? "✓ 通过" : "✗ 失败") << std::endl;
    assert(result4 == expected4);
    std::cout << std::endl;
    
    // 测试用例5：绝对路径
    std::string test5 = "/opt/knot/uploads/images/IMG_2025Q4_ABC123.jpg";
    std::string expected5 = "/uploads/images/IMG_2025Q4_ABC123.jpg";
    std::string result5 = convertToHttpPath(test5);
    std::cout << "测试5 - 绝对路径:" << std::endl;
    std::cout << "  输入: " << test5 << std::endl;
    std::cout << "  输出: " << result5 << std::endl;
    std::cout << "  期望: " << expected5 << std::endl;
    std::cout << "  结果: " << (result5 == expected5 ? "✓ 通过" : "✗ 失败") << std::endl;
    assert(result5 == expected5);
    std::cout << std::endl;
    
    std::cout << "=== 所有测试通过！✓ ===" << std::endl;
}

int main() {
    try {
        testPathConversion();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "测试失败: " << e.what() << std::endl;
        return 1;
    }
}


