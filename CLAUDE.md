# CLAUDE.md

# 通用规则
- 你是 Claude 
- 中文回复
- 遵循 KISS 原则，非必要不要过度设计 

* 实现简单可维护，不需要考虑太多防御性的边界条件 
* 你需要逐步进行，通过多轮对话来完成需求，进行渐进式开发 
* 在开始设计方案或实现代码之前，你需要进行充分的调研。如果有任何不明确的要求，请在继续之前向我确认 
* 当你收到一个需求时，首先需要思考相关的方案，并请求我进行审核。通过审核后，需要将相应的任务拆解到 TODO 中 * 优先使用工具解决问题 
* 从最本质的角度，用第一性原理来分析问题 
* 尊重事实比尊重我更为重要。如果我犯错，请毫不犹豫地指正我，以便帮助我提高 

# MCP工具使用策略

## 强制使用场景

- **代码分析请求**: “查看”、“分析”、“检查”、“搜索” → 立即调用相应MCP工具
- **文件操作**: 提及具体文件路径 → 使用filesystem等工具验证和读取
- **项目结构**: “项目结构”、“目录结构” → 使用filesystem等工具浏览
- **代码搜索**: “在哪里”、“如何实现” → 使用codecompass等工具搜索
- **版本控制**: 提及git、提交、分支 → 使用git工具检查状态

## 透明度要求

使用MCP工具前必须告知用户：

- “让我通过文件系统工具检查…”
- “使用代码分析工具搜索…”
- “通过Git工具查看…”



# 本项目

本文件为 Claude Code (claude.ai/code) 在此代码库中工作时提供指导。

## 项目概述

**Knot** 是一个使用 C++17 开发的高性能图片分享系统后端服务。该项目由停车位共享系统转换而来，目前已完成用户认证模块的实现，图片管理系统计划后续开发。

**技术栈**: C++17, MySQL 8.0, cpp-httplib 0.11.0, jwt-cpp 0.6.0, JsonCpp 1.9.5, OpenSSL 1.1.1, spdlog 1.9.2

**当前状态**:
- ✅ 用户认证系统（注册、登录、JWT令牌管理）
- ⏳ 图片管理系统（规划中 - 参见 `/需求分析.md`）
- ⏳ 互动系统（点赞、收藏、关注 - 规划中）
- ⏳ 分享系统（Deep Link唤醒 - 规划中）

## 构建命令

### 初始化设置
```bash
cd backend-service

# 配置数据库
mysql -u root -p < config/database.sql

# 复制并编辑配置文件
cp config/config.example.json config/config.json
# 编辑 config.json，设置数据库凭据和JWT密钥
```

### 编译
```bash
# 清理构建
rm -rf build && mkdir build
cd build
cmake ..
make -j4

# 可执行文件位置: build/knot_image_sharing
```

### 运行服务器
```bash
# 在 backend-service 目录下
./build/knot_image_sharing [配置文件路径]

# 默认配置路径: config/config.json
# 服务器监听地址: 0.0.0.0:8080
```

### 测试
```bash
# 并发负载测试（100个并发请求）
cd test
./test_concurrent.sh
```

## 架构设计

### 九层架构

代码库严格遵循九层架构设计，各层职责清晰分离：

```
第1层：客户端层（HTTP客户端）
第2层：HTTP服务器层（src/server/http_server.{h,cpp}）
第3层：API接口层（src/api/*）
第4层：业务逻辑层（src/core/*_service.{h,cpp}）
第5层：数据访问层（src/database/*_repository.{h,cpp}）
第6层：安全层（src/security/jwt_manager, password_hasher）
第7层：基础设施层（src/database/connection_pool, src/utils/*）
第8层：数据模型层（src/models/*）
第9层：数据库层（MySQL）
```

**关键规则**: 每一层只能调用其下层的功能，严禁跨层调用或向上调用。

### 核心架构模式

**RAII资源管理**: 所有数据库连接必须使用 `ConnectionGuard` 实现自动清理：
```cpp
ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
if (!connGuard.isValid()) {
    // 处理错误
}
// 作用域结束时自动归还连接到连接池
```

**单例模式**: `DatabaseConnectionPool`、`ConfigManager` 和 `Logger` 均为单例，通过 `getInstance()` 访问。

**JWT认证机制**:
- 访问令牌（Access Token）有效期：1小时
- 刷新令牌（Refresh Token）有效期：24小时
- 受保护的资源必须在Handler层验证令牌

**密码安全**: 使用PBKDF2-HMAC-SHA256算法，迭代100,000次，每个用户独立盐值。

## 代码组织

### 测试文件目录
如果需要进行项目测试，可将测试文件放在/home/kun/projects/Knot/backend-service/test下，如果是临时测试文件，强调使用完毕后就要删除。另外方便知道测试文件作用，在每个测试文件开头需要写注释标注测试文件的内容。

### 添加新功能时的开发流程

遵循从下到上的层级开发方式（以图片功能为例）：

1. **第8层 - 模型层**: 创建 `src/models/image.{h,cpp}`
   - 定义 Image 类及其成员变量
   - 实现 getters/setters
   - 实现 `toJson()` 和 `fromJson()` 方法
   - 添加数据验证逻辑

2. **第5层 - 数据访问层**: 创建 `src/database/image_repository.{h,cpp}`
   - 所有SQL查询使用预编译语句
   - 使用 `ConnectionGuard` 管理连接
   - 返回模型对象

3. **第4层 - 业务逻辑层**: 创建 `src/core/image_service.{h,cpp}`
   - 实现业务逻辑（验证、图片处理等）
   - 调用 Repository 方法
   - 返回结果结构体

4. **第3层 - API接口层**: 创建 `src/api/image_handler.{h,cpp}`
   - 解析HTTP请求
   - 对受保护端点进行JWT验证
   - 调用 Service 方法
   - 构建JSON响应

5. **第2层 - 服务器层**: 在 `src/server/http_server.cpp` 中注册路由
   ```cpp
   imageHandler_->registerRoutes(*server_);
   ```

6. **第9层 - 数据库**: 在 `config/database.sql` 中添加数据表

### 用户角色

当前在 `src/models/user.h` 中定义了两种角色：
- `UserRole::USER` - 普通用户（默认）
- `UserRole::ADMIN` - 管理员

注册用户时，角色默认为 `USER`。不要使用已废弃的角色 `OWNER`、`DRIVER` 或 `BOTH`（这些在从停车系统转换为图片分享系统时已被移除）。

## 配置

运行前需编辑 `config/config.json`：

**关键配置项**:
- `database.database`: 应为 `knot_image_sharing`（不是 `shared_parking`）
- `jwt.secret`: 生产环境必须修改
- `security.pbkdf2_iterations`: 100000（不要降低）
- `server.port`: 默认 8080

**性能调优**:
- `database.pool_size`: 连接池大小（默认：10）
- `server.thread_pool_size`: HTTP线程池（默认：8，可提升至32）
- CMakeLists.txt 中定义：
  - `CPPHTTPLIB_LISTEN_BACKLOG=128`
  - `CPPHTTPLIB_THREAD_POOL_COUNT=32`

## API开发

### 端点命名模式

```cpp
// 在 Handler 的 registerRoutes() 中
server.Post("/api/v1/resource", [this](...) { handleCreate(...); });
server.Get("/api/v1/resource/:id", [this](...) { handleGetById(...); });
server.Put("/api/v1/resource/:id", [this](...) { handleUpdate(...); });
server.Delete("/api/v1/resource/:id", [this](...) { handleDelete(...); });
```

### 受保护端点的实现

```cpp
void Handler::handleProtected(const httplib::Request& req, httplib::Response& res) {
    // 1. 提取令牌
    std::string authHeader = req.get_header_value("Authorization");
    if (authHeader.substr(0, 7) != "Bearer ") {
        // 返回 401 未授权
    }
    std::string token = authHeader.substr(7);

    // 2. 验证JWT
    auto jwtManager = std::make_unique<JWTManager>();
    TokenValidationResult validation = jwtManager->validateAccessToken(token);
    if (!validation.valid) {
        // 返回 401 未授权
    }

    // 3. 使用 validation.userId 访问用户数据
}
```

### 响应格式

始终使用统一的JSON结构：
```json
{
  "success": true/false,
  "message": "可读的消息说明",
  "data": { ... },
  "timestamp": 1234567890
}
```

## 数据库

### 连接使用模式

```cpp
// 务必使用 ConnectionGuard
ConnectionGuard guard(DatabaseConnectionPool::getInstance());
if (!guard.isValid()) {
    Logger::error("获取数据库连接失败");
    return ErrorResult;
}

MYSQL* conn = guard.get();
// 使用 conn 执行查询
// 作用域结束时自动归还连接
```

### SQL最佳实践

- **始终使用预编译语句**并进行参数绑定
- 永远不要将用户输入直接拼接到SQL中
- 多步操作使用事务
- 添加适当的索引（参见 `config/database.sql`）

### 当前数据库结构

表：
- `users` - 用户认证和个人资料
- 未来计划：`images`、`likes`、`favorites`、`follows`、`user_stats`、`tags`、`image_tags`、`share_links`

数据库名：`knot_image_sharing`

## 日志记录

```cpp
#include "utils/logger.h"

Logger::debug("调试信息");
Logger::info("普通信息");
Logger::warning("警告信息");
Logger::error("错误信息");
Logger::fatal("致命错误");
```

日志文件位置：`logs/auth-service.log`（在 config.json 中配置）

## 文档

完整文档位于 `backend-service/project_document/`：
- `[000]API文档.md` - 所有API端点说明
- `[001]项目架构文档.md` - 架构详细说明
- `[002]环境安装配置教程.md` - 环境搭建指南
- `[100]阶段A-基础模块实现.md` - 基础设施实现
- `[101]阶段B-用户认证模块.md` - 认证系统实现
- `[200]数据库设计文档.md` - 数据库设计
- `[201]性能优化专题.md` - 性能优化指南

**文档编号注意事项：**[000]到[099]存放项目开发的指导性文件，[100]到[199]存放项目开发的阶段性实现的相关文档，[200]到[299]存放的为部分技术上设计与实现的文档。你需要在/home/kun/projects/Knot/backend-service/project_document下的文档发生变化的时候同时更新`/backend-service/README.md`和本文档的文档纲要。

**产品需求文档**: `/需求分析.md` 包含完整的Knot图片分享应用需求。

## 重要背景

**项目历史**: 本代码库最近从停车位共享系统转换为图片分享系统。所有停车位相关代码已被清理。如果发现任何"parking"或"ParkingSpace"相关引用，应当移除或标记为遗留代码。

**命名规范**: 可执行文件现为 `knot_image_sharing`（原为 `shared_parking_auth`）。服务名称为"Knot - Image Sharing Service"。

**下一阶段开发**: 按照 `/需求分析.md` 中的要求实现图片管理模块，包括：
- 图片上传（压缩和缩略图生成）
- Feed推荐算法
- 图片存储和检索
- 与未来互动系统（点赞、收藏、关注）集成
