/**
 * 测试文件: test_user_repository.cpp
 * 测试目的: 测试UserRepository::findById方法的类型转换问题
 * 创建时间: 2025-10-22
 * 测试完成后删除
 */

#include "database/user_repository.h"
#include "database/connection_pool.h"
#include "utils/config_manager.h"
#include "utils/logger.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " <config.json路径>" << std::endl;
        return 1;
    }

    try {
        // 1. 初始化配置
        ConfigManager::getInstance().loadConfig(argv[1]);
        Logger::init("logs/test.log", "debug");

        // 2. 初始化数据库连接池
        auto& pool = DatabaseConnectionPool::getInstance();

        // 3. 创建UserRepository实例
        UserRepository userRepo;

        // 4. 测试findById方法 (用户ID=2)
        std::cout << "\n=== 测试1: 查询用户ID=2 ===" << std::endl;
        try {
            auto userOpt = userRepo.findById(2);
            if (userOpt.has_value()) {
                User user = userOpt.value();
                std::cout << "✅ 查询成功!" << std::endl;
                std::cout << "   物理ID: " << user.getId() << std::endl;
                std::cout << "   业务ID: " << user.getUserId() << std::endl;
                std::cout << "   用户名: " << user.getUsername() << std::endl;
            } else {
                std::cout << "⚠️  用户不存在" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ 异常: " << e.what() << std::endl;
            return 1;
        }

        // 5. 测试findById方法 (用户ID=35)
        std::cout << "\n=== 测试2: 查询用户ID=35 ===" << std::endl;
        try {
            auto userOpt = userRepo.findById(35);
            if (userOpt.has_value()) {
                User user = userOpt.value();
                std::cout << "✅ 查询成功!" << std::endl;
                std::cout << "   物理ID: " << user.getId() << std::endl;
                std::cout << "   业务ID: " << user.getUserId() << std::endl;
                std::cout << "   用户名: " << user.getUsername() << std::endl;
            } else {
                std::cout << "⚠️  用户不存在" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ 异常: " << e.what() << std::endl;
            return 1;
        }

        // 6. 测试不存在的用户
        std::cout << "\n=== 测试3: 查询不存在的用户ID=99999 ===" << std::endl;
        try {
            auto userOpt = userRepo.findById(99999);
            if (userOpt.has_value()) {
                std::cout << "❌ 不应该找到用户" << std::endl;
                return 1;
            } else {
                std::cout << "✅ 正确返回nullopt" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "❌ 异常: " << e.what() << std::endl;
            return 1;
        }

        std::cout << "\n=== 所有测试通过! ===" << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "致命错误: " << e.what() << std::endl;
        return 1;
    }
}

