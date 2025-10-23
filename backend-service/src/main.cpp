// Knot - 图片分享服务主入口
// Knot Team

#include <iostream>
#include <memory>
#include <signal.h>

// 第三方库
#include "httplib.h"
#include <json/json.h>

// 项目头文件
#include "utils/config_manager.h"
#include "utils/logger.h"
#include "server/http_server.h"
#include "database/connection_pool.h"

using Json::Value;

// 全局服务器实例，用于信号处理
std::unique_ptr<HttpServer> g_server;

// 信号处理函数：优雅关闭服务
// 参数 signal: 信号编号
void signalHandler(int signal) {
    Logger::info("收到信号 " + std::to_string(signal) + "，正在优雅关闭服务...");
    
    if (g_server) {
        g_server->stop();
    }
    
    Logger::info("服务器已成功停止");
    exit(0);
}

// 初始化信号处理器
void setupSignalHandlers() {
    signal(SIGINT, signalHandler);   // Ctrl+C
    signal(SIGTERM, signalHandler);  // 终止请求
    signal(SIGQUIT, signalHandler);  // 退出信号
}

// 打印应用启动横幅
void printBanner() {
    std::cout << R"(
    __ __          __
   / //_/___  ____/ /_
  / ,< / _ \/ __ / __/
 / /|_/ / /\____/ /_  
/_/ |_/_/     \__/    

Knot 图片分享服务 v2.9.0
)" << std::endl;
}

// 主函数：应用程序入口
// 参数 argc: 命令行参数数量
// 参数 argv: 命令行参数值数组
// 返回值: 退出状态码
int main(int argc, char* argv[]) {
    try {
        // 打印启动横幅
        printBanner();
        
        // 设置信号处理器
        setupSignalHandlers();
        
        // 加载配置文件
        std::string configPath = "config/config.json";
        if (argc > 1) {
            configPath = argv[1];
        }

        std::cout << "正在加载配置文件: " << configPath << std::endl;
        if (!ConfigManager::getInstance().loadConfig(configPath)) {
            std::cerr << "加载配置文件失败: " << configPath << std::endl;
            return 1;
        }

        // 初始化日志系统
        auto& config = ConfigManager::getInstance();
        std::string logFile = config.get<std::string>("logging.file", "logs/app.log");
        std::string logLevelStr = config.get<std::string>("logging.level", "info");
        bool consoleOutput = config.get<bool>("logging.console", true);

        // 将日志级别字符串转换为枚举
        LogLevel logLevel = LogLevel::INFO;
        if (logLevelStr == "debug") logLevel = LogLevel::DEBUG;
        else if (logLevelStr == "info") logLevel = LogLevel::INFO;
        else if (logLevelStr == "warning") logLevel = LogLevel::WARNING;
        else if (logLevelStr == "error") logLevel = LogLevel::ERROR;
        else if (logLevelStr == "fatal") logLevel = LogLevel::FATAL;

        if (!Logger::initialize(logFile, logLevel, consoleOutput)) {
            std::cerr << "初始化日志系统失败" << std::endl;
            return 1;
        }

        Logger::info("配置文件加载成功");
        Logger::info("正在初始化 Knot 图片分享服务...");
        
        // 初始化数据库连接池
        Logger::info("正在初始化数据库连接池...");
        auto& dbPool = DatabaseConnectionPool::getInstance();
        if (!dbPool.initialize()) {
            Logger::error("初始化数据库连接池失败");
            return 1;
        }
        
        // 创建 HTTP 服务器
        Logger::info("正在创建 HTTP 服务器...");
        g_server = std::make_unique<HttpServer>();
        
        // 初始化并启动服务器
        if (!g_server->initialize()) {
            Logger::error("初始化 HTTP 服务器失败");
            return 1;
        }
        
        Logger::info("正在启动 HTTP 服务器...");
        if (!g_server->start()) {
            Logger::error("启动 HTTP 服务器失败");
            return 1;
        }

        Logger::info("Knot 服务启动成功");
        Logger::info("服务器正在运行，准备接受连接");
        
        // 保持主线程运行
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        Logger::error("致命错误: " + std::string(e.what()));
        return 1;
    } catch (...) {
        Logger::error("发生未知致命错误");
        return 1;
    }
    
    return 0;
}
