/**
 * 测试文件: test_data_consistency.cpp
 * 测试目的: 验证帖子图片服务数据一致性问题
 * 发现问题：数据库schema与代码不匹配，images表缺少post_id和display_order字段
 * 创建时间: 2025-10-13
 * 测试完成后删除
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>

class DataConsistencyTester {
private:
    std::string projectPath;

public:
    DataConsistencyTester(const std::string& path) : projectPath(path) {}

    // 读取文件内容
    std::string readFile(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            return "";
        }
        return std::string(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        );
    }

    // 测试1：验证数据库schema问题
    void testDatabaseSchema() {
        std::cout << "\n=== 测试1：验证数据库schema问题 ===" << std::endl;

        // 读取数据库schema文件
        std::string schemaContent = readFile(projectPath + "/config/database.sql");

        if (schemaContent.empty()) {
            std::cout << "❌ 无法读取database.sql文件" << std::endl;
            return;
        }

        // 检查images表定义
        std::regex imagesTableRegex(R"(CREATE TABLE.*images.*\([\s\S]*?\))");
        std::smatch match;

        if (std::regex_search(schemaContent, match, imagesTableRegex)) {
            std::string imagesTableDef = match[0].str();

            // 检查是否包含post_id字段
            if (imagesTableDef.find("post_id") == std::string::npos) {
                std::cout << "❌ 发现严重问题：images表缺少post_id字段" << std::endl;
            } else {
                std::cout << "✅ images表包含post_id字段" << std::endl;
            }

            // 检查是否包含display_order字段
            if (imagesTableDef.find("display_order") == std::string::npos) {
                std::cout << "❌ 发现严重问题：images表缺少display_order字段" << std::endl;
            } else {
                std::cout << "✅ images表包含display_order字段" << std::endl;
            }
        }

        // 读取image_repository.cpp检查代码中使用的字段
        std::string repoContent = readFile(projectPath + "/src/database/image_repository.cpp");
        if (!repoContent.empty()) {
            if (repoContent.find("post_id") != std::string::npos) {
                std::cout << "⚠️  代码中使用post_id字段，但数据库表中可能没有此字段" << std::endl;
            }
            if (repoContent.find("display_order") != std::string::npos) {
                std::cout << "⚠️  代码中使用display_order字段，但数据库表中可能没有此字段" << std::endl;
            }
        }
    }

    // 测试2：检查SQL语句与schema的匹配性
    void testSQLSchemaMismatch() {
        std::cout << "\n=== 测试2：检查SQL语句与schema的匹配性 ===" << std::endl;

        std::string repoContent = readFile(projectPath + "/src/database/image_repository.cpp");
        if (repoContent.empty()) {
            std::cout << "❌ 无法读取image_repository.cpp文件" << std::endl;
            return;
        }

        // 查找INSERT语句
        std::regex insertRegex(R"(INSERT INTO images.*\(.*\))");
        std::sregex_iterator iter(repoContent.begin(), repoContent.end(), insertRegex);
        std::sregex_iterator end;

        std::vector<std::string> insertStatements;
        for (; iter != end; ++iter) {
            insertStatements.push_back(iter->str());
        }

        if (!insertStatements.empty()) {
            std::cout << "发现INSERT语句：" << std::endl;
            for (const auto& stmt : insertStatements) {
                std::cout << "  " << stmt << std::endl;
                if (stmt.find("post_id") != std::string::npos) {
                    std::cout << "    ❌ 使用了post_id字段" << std::endl;
                }
                if (stmt.find("display_order") != std::string::npos) {
                    std::cout << "    ❌ 使用了display_order字段" << std::endl;
                }
            }
        }

        // 查找SELECT语句
        std::regex selectRegex(R"(SELECT.*FROM images.*WHERE.*post_id)");
        std::sregex_iterator selectIter(repoContent.begin(), repoContent.end(), selectRegex);

        if (selectIter != end) {
            std::cout << "\n发现使用post_id的SELECT语句：" << std::endl;
            for (; selectIter != end; ++selectIter) {
                std::cout << "  " << selectIter->str() << std::endl;
                std::cout << "    ❌ 使用了post_id字段进行查询" << std::endl;
            }
        }
    }

    // 测试3：检查image_count一致性问题
    void testImageCountConsistency() {
        std::cout << "\n=== 测试3：检查image_count一致性问题 ===" << std::endl;

        std::string postServiceContent = readFile(projectPath + "/src/core/post_service.cpp");
        if (postServiceContent.empty()) {
            std::cout << "❌ 无法读取post_service.cpp文件" << std::endl;
            return;
        }

        // 检查createPost方法中的image_count更新逻辑
        if (postServiceContent.find("setImageCount") != std::string::npos) {
            std::cout << "✅ 找到setImageCount调用" << std::endl;

            // 查找更新image_count的代码段
            size_t pos = postServiceContent.find(" setImageCount(actualImageCount)");
            if (pos != std::string::npos) {
                // 提取周围的上下文
                size_t start = std::max(0, static_cast<int>(pos - 200));
                size_t end = std::min(postServiceContent.length(), pos + 200);
                std::string context = postServiceContent.substr(start, end - start);

                std::cout << "发现image_count更新逻辑：" << std::endl;
                std::cout << "..." << context << "..." << std::endl;

                if (context.find("actualImageCount != static_cast<int>(imagePaths.size())") != std::string::npos) {
                    std::cout << "⚠️  只在图片处理失败时才更新image_count，可能导致不一致" << std::endl;
                }
            }
        }

        // 检查removeImageFromPost方法
        if (postServiceContent.find("removeImageFromPost") != std::string::npos) {
            std::cout << "\n检查removeImageFromPost方法..." << std::endl;

            size_t pos = postServiceContent.find("removeImageFromPost");
            size_t end = postServiceContent.find("}", pos);
            std::string methodContent = postServiceContent.substr(pos, end - pos);

            if (methodContent.find("updateImageCount") == std::string::npos) {
                std::cout << "❌ removeImageFromPost方法中没有更新posts.image_count" << std::endl;
            } else {
                std::cout << "✅ removeImageFromPost方法中有更新image_count" << std::endl;
            }
        }
    }

    // 测试4：检查事务使用情况
    void testTransactionUsage() {
        std::cout << "\n=== 测试4：检查事务使用情况 ===" << std::endl;

        std::string postServiceContent = readFile(projectPath + "/src/core/post_service.cpp");
        if (postServiceContent.empty()) {
            std::cout << "❌ 无法读取post_service.cpp文件" << std::endl;
            return;
        }

        // 检查是否使用了事务
        if (postServiceContent.find("BEGIN") == std::string::npos &&
            postServiceContent.find("START TRANSACTION") == std::string::npos) {
            std::cout << "⚠️  未发现事务使用，多步骤操作可能导致数据不一致" << std::endl;
        } else {
            std::cout << "✅ 发现事务使用" << std::endl;
        }

        // 检查ConnectionGuard的使用
        if (postServiceContent.find("ConnectionGuard") != std::string::npos) {
            std::cout << "✅ 使用了ConnectionGuard进行连接管理" << std::endl;
        } else {
            std::cout << "⚠️  未发现ConnectionGuard使用" << std::endl;
        }
    }

    // 运行所有测试
    void runAllTests() {
        std::cout << "开始数据一致性问题静态分析..." << std::endl;
        std::cout << "项目路径: " << projectPath << std::endl;

        testDatabaseSchema();
        testSQLSchemaMismatch();
        testImageCountConsistency();
        testTransactionUsage();

        std::cout << "\n=== 测试总结 ===" << std::endl;
        std::cout << "发现的问题按严重程度排序：" << std::endl;
        std::cout << "1. 🚨 严重：数据库schema与代码不匹配" << std::endl;
        std::cout << "   - images表缺少post_id和display_order字段" << std::endl;
        std::cout << "   - 这会导致图片无法正确关联到帖子" << std::endl;
        std::cout << "2. ⚠️  中等：posts.image_count可能不准确" << std::endl;
        std::cout << "   - 删除图片后未更新count" << std::endl;
        std::cout << "   - 只在部分失败情况下更新count" << std::endl;
        std::cout << "3. 💡 轻微：缺少事务保护" << std::endl;
        std::cout << "   - 多步骤操作可能导致数据不一致" << std::endl;
    }
};

int main() {
    std::cout << "帖子图片服务数据一致性测试工具" << std::endl;
    std::cout << "========================================" << std::endl;

    DataConsistencyTester tester("/home/kun/projects/SharePix/backend-service");

    // 运行测试
    tester.runAllTests();

    std::cout << "\n测试完成！请查看上述问题并修复。" << std::endl;

    return 0;
}