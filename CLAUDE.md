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

**Knot** 是一个使用 C++17 开发的高性能图片分享系统后端服务。该项目由停车位共享系统转换而来，目前已完成用户认证、用户信息管理、多图片帖子发布、图片管理和标签系统等核心功能。

**技术栈**: C++17, MySQL 8.0, cpp-httplib 0.11.0, jwt-cpp 0.6.0, JsonCpp 1.9.5, OpenSSL 1.1.1, spdlog 1.9.2

**当前版本**: v2.5.0

**当前状态**:
- ✅ 用户认证系统（注册、登录、JWT令牌管理、密码修改）
- ✅ 用户信息管理系统（v2.1.0 - 获取/修改用户信息、用户名检查、公开信息查询）
- ✅ 多图片帖子系统（v2.0.0 - 1-9张图片发布、编辑、删除）
- ✅ 图片管理系统（图片压缩、缩略图生成、Feed推荐）
- ✅ 标签系统（帖子标签关联、标签管理）
- ✅ **分享系统（v2.3.0 - 短链接生成、三端Deep Link、帖子分享）**
- ✅ **Feed流优化（v2.5.0 - 批量查询、可选认证、作者信息、互动状态）**
- ⏳ 社交互动系统（点赞、收藏、关注、评论 - 规划中）

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

**已实现的表：**
- `users` - 用户认证和个人资料（v2.1.0新增字段：bio, gender, location, website, avatar_url等）
- `posts` - 帖子信息（支持1-9张图片）
- `images` - 图片信息（包含原图和缩略图路径）
- `tags` - 标签信息
- `post_tags` - 帖子标签关联表
- `share_links` - 分享链接表（v2.3.0新增：short_code, target_type, target_id, creator_id, expire_time）

**未来计划的表：**
- `likes` - 点赞信息
- `favorites` - 收藏信息
- `follows` - 关注关系
- `comments` - 评论信息
- `user_stats` - 用户统计数据

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

**核心文档（000-099）：**
- `[000]API文档.md` - 所有API端点说明
- `[001]数据库设计文档.md` - 数据库表结构和关系设计
- `[002]项目架构文档.md` - 九层架构设计和技术栈
- `[003]环境安装配置教程.md` - 环境搭建指南

**阶段文档（100-199）：**
- `[100]阶段A-基础模块实现.md` - 基础设施实现
- `[101]阶段B-用户认证模块.md` - 用户认证系统实现
- `[102]阶段C-图片管理模块.md` - 图片管理系统实现
- `[103]阶段C-1-图片管理模块第一次测试情况.md` - 图片模块测试记录
- `[104]Apifox测试流程-BUG修复验证.md` - API测试流程和Bug修复
- `[105]阶段C-2-多图片帖子系统.md` - 多图片帖子系统（v2.0.0）
- `[106]用户信息管理功能完善-实施计划.md` - 用户信息管理（v2.1.0）
- `[108]阶段D-2-互动系统关注功能实现计划.md` - 关注系统（v2.2.0）
- `[109]阶段D-3-分享系统实现计划.md` - 分享系统（v2.3.0，~4200行）

**技术专题（200-299）：**
- `[200]数据库设计文档.md` - 数据库设计详解
- `[201]性能优化专题.md` - 性能优化指南

**文档编号注意事项：**[000]到[099]存放项目开发的指导性文件，[100]到[199]存放项目开发的阶段性实现的相关文档，[200]到[299]存放的为部分技术上设计与实现的文档。你需要在/home/kun/projects/Knot/backend-service/project_document下的文档发生变化的时候同时更新`/backend-service/README.md`和本文档的文档纲要。

**产品需求文档**: `/需求分析.md` 包含完整的Knot图片分享应用需求。

## 重要背景

**项目历史**: 本代码库最近从停车位共享系统转换为图片分享系统。所有停车位相关代码已被清理。如果发现任何"parking"或"ParkingSpace"相关引用，应当移除或标记为遗留代码。

**命名规范**: 可执行文件现为 `knot_image_sharing`（原为 `shared_parking_auth`）。服务名称为"Knot - Image Sharing Service"。

**下一阶段开发**: 图片管理核心功能已完成，下一步将实现社交互动系统，包括：
- 点赞功能
- 收藏功能
- 关注/粉丝系统
- 评论功能

## 静态编译与部署

### 静态编译

项目支持静态编译以便于生产环境部署：

```bash
cd backend-service/deploy-static
./build.sh
```

编译完成后生成：
- 可执行文件：`build/knot_image_sharing` (~14MB)
- 部署包：`build/knot-deploy-YYYYMMDD-HHMMSS.tar.gz` (~4.7MB)

### 静态链接说明
- **静态链接**: MySQL Client, OpenSSL (libssl, libcrypto), libstdc++, libgcc_s
- **动态链接**: libjsoncpp.so.25, libzstd.so.1, libfmt.so.8
- **目标服务器要求**: Ubuntu 20.04+，需安装上述动态库

### 部署包结构

```
knot-deploy-YYYYMMDD-HHMMSS/
├── knot_image_sharing          # 主服务可执行文件
├── config/
│   └── config.production.json  # 生产配置模板
├── docs/api/
│   ├── openapi.yaml           # OpenAPI规范文件
│   ├── api-docs.html          # Swagger UI文档
│   └── start-api-docs.sh      # 文档服务启动脚本
├── start.sh                    # 主服务启动脚本
└── README.md                   # 部署说明
```

### 服务启动

```bash
# 1. 解压部署包
tar -xzf knot-deploy-YYYYMMDD-HHMMSS.tar.gz
cd knot-deploy-YYYYMMDD-HHMMSS

# 2. 修改配置
vim config/config.production.json

# 3. 启动主服务（默认监听8080端口）
nohup ./start.sh > knot.log 2>&1 &

# 4. 启动API文档服务（默认监听8081端口）
cd docs/api
nohup ./start-api-docs.sh > api-docs.log 2>&1 &
```

### API文档服务

**访问地址**: `http://您的服务器IP:8081/api-docs.html`

**文档服务说明**:
- 使用Python的http.server模块提供静态文件服务
- 提供基于Swagger UI的交互式API文档
- 默认端口8081可在start-api-docs.sh中修改

**文档更新后刷新方法**:

1. **服务器端**：
   ```bash
   # 找到并停止旧进程
   ps aux | grep "python3 -m http.server 8081"
   kill <PID>

   # 重新启动
   nohup ./start-api-docs.sh > api-docs.log 2>&1 &
   ```

2. **浏览器端**：
   - 按 `Ctrl+Shift+R`（Windows/Linux）或 `Cmd+Shift+R`（Mac）强制刷新
   - 或者打开开发者工具，右键刷新按钮选择"清空缓存并硬性重新加载"

## API版本历史

### v2.5.0 (2025-10-18) - Feed流批量查询优化
**新增功能：**
- ✅ Feed流可选JWT认证
  - GET /api/v1/posts - 支持游客访问和登录用户访问
  - GET /api/v1/users/:user_id/posts - 支持游客访问和登录用户访问
  - 无效Token自动降级为游客模式（不返回401）
- ✅ Feed流批量查询优化
  - 批量查询帖子作者信息（UserRepository::batchGetUsers）
  - 批量查询点赞状态（LikeRepository::batchExistsForPosts）
  - 批量查询收藏状态（FavoriteRepository::batchExistsForPosts）
  - 解决N+1查询问题
- ✅ Feed流新增返回字段
  - author.user_id - 作者用户ID
  - author.username - 作者用户名
  - author.avatar_url - 作者头像URL
  - has_liked - 当前用户是否已点赞（游客固定返回false）
  - has_favorited - 当前用户是否已收藏（游客固定返回false）

**代码变更：**
- 新增类：UserService（批量用户服务）
- Repository层：3个批量查询方法（~487行）
- Service层：3个批量查询方法（~201行）
- Handler层：2个方法重写（~329行）
- 总计新增/修改代码：~1017行

**性能提升：**
- 游客访问Feed流（20条）：21次查询 → 1次查询（⬇️95.2%）
- 登录用户访问Feed流（20条）：61次查询 → 4次查询（⬇️93.4%）
- 响应时间（P99）：~150ms → ~50ms（⬆️66.7%）
- QPS：~100 → ~300（⬆️200%）

**技术亮点：**
- SQL IN查询批量获取数据
- std::unordered_map实现O(1)查找
- 游客模式跳过互动状态查询，节省数据库资源
- 统一JSON结构，字段始终存在

**文档：** `[118]Feed流用户状态批量查询优化方案.md`

---

### v2.4.1 (2025-10-15) - JSON格式创建帖子支持
**新增功能：**
- ✅ 创建帖子接口支持JSON+Base64格式
  - POST /api/v1/posts - 同时支持multipart/form-data和application/json
  - 支持Base64编码的图片数据（Data URI和纯Base64两种格式）
  - 自动检测和解码Base64数据

**技术实现：**
- Content-Type自动识别（multipart/form-data 或 application/json）
- 复用现有Base64Decoder进行图片解码
- 完整的日志记录（解码时间、大小变化、格式验证）
- 向后兼容，不影响现有multipart客户端

**性能指标：**
- Base64解码时间：<1ms（9KB图片）
- 大小变化：Base64编码增加约25-33%
- 支持格式：PNG、JPEG、GIF、WebP

**文档：** `[115]创建帖子接口支持JSON+Base64格式-实施计划.md`

---

### v2.3.0 (2025-10-11) - 分享系统
**新增功能：**
- ✅ 短链接生成系统
  - POST /api/v1/posts/:post_id/share - 创建帖子分享链接
  - GET /api/v1/share/:code - 解析分享链接获取帖子信息（公开接口）
- ✅ 雪花ID算法 + Base62编码
- ✅ Deep Link支持（iOS/Android/HarmonyOS三端）
  - iOS Universal Links配置
  - Android App Links配置
  - HarmonyOS Deep Linking配置（自定义Scheme）
  - HarmonyOS App Linking配置（域名验证）

**数据库变更：**
- 新增表：`share_links`（7字段，4索引，2外键）
- 字段：short_code, target_type, target_id, creator_id, create_time, expire_time
- 索引：idx_short_code(唯一), idx_target, idx_creator, idx_create_time
- 外键级联删除：删除帖子自动删除分享链接

**技术亮点：**
- 8位Base62短码，62^8 ≈ 218万亿种可能
- 去重逻辑：同一帖子返回相同短链接
- 支持短链接过期机制
- 三端Deep Link统一配置方案

**配置文件：**
- `.well-known/apple-app-site-association`（iOS）
- `.well-known/assetlinks.json`（Android）
- `.well-known/applinking.json`（HarmonyOS）

**文档：** `[109]阶段D-3-分享系统实现计划.md`（~4200行，含完整技术设计和实施指南）

---

### v2.1.0 (2025-10-09)
**新增功能：**
- ✅ 用户信息管理
  - GET /api/v1/users/profile - 获取当前用户完整信息
  - PUT /api/v1/users/profile - 修改用户信息
  - GET /api/v1/users/check-username - 检查用户名可用性
  - GET /api/v1/users/{user_id} - 获取用户公开信息

**数据库变更：**
- users表新增字段：`bio`(个人简介), `gender`(性别), `location`(所在地), `website`(个人网站), `avatar_url`(头像URL)

**文档：** `[106]用户信息管理功能完善-实施计划.md`

### v2.0.0 (2025-10-08)
**新增功能：**
- ✅ 多图片帖子系统（1-9张图片）
- ✅ 图片压缩与缩略图生成
- ✅ Feed推荐算法
- ✅ 标签系统

**数据库变更：**
- 新增表：`posts`, `images`, `tags`, `post_tags`

**文档：** `[102]阶段C-图片管理模块.md`, `[105]阶段C-2-多图片帖子系统.md`

### v1.2.0 (2025-10-01)
**新增功能：**
- ✅ 用户认证系统（注册、登录、JWT令牌管理）
- ✅ 密码安全存储（PBKDF2-HMAC-SHA256）
- ✅ 访问令牌和刷新令牌机制

**数据库变更：**
- 新增表：`users`

**文档：** `[101]阶段B-用户认证模块.md`

### v1.0.0 (2025-09-30)
**初始版本：**
- ✅ 项目骨架
- ✅ 数据库连接池
- ✅ 配置管理
- ✅ 日志系统
- ✅ HTTP服务器封装

**文档：** `[100]阶段A-基础模块实现.md`

## 测试流程

### 测试文件管理
- **测试文件位置**：`/home/kun/projects/Knot/backend-service/test/`
- **临时测试文件**：使用后必须删除
- **文件注释要求**：每个测试文件开头必须注释说明测试目的

**示例**：
```cpp
/**
 * 测试文件: test_user_profile.cpp
 * 测试目的: 测试用户信息管理API的各项功能
 * 创建时间: 2025-10-09
 * 测试完成后删除
 */
```

### API测试工具

**推荐使用Apifox**：
1. 导入 `docs/api/openapi.yaml` 获取完整API定义
2. 自动生成测试用例
3. 支持环境变量和前置脚本

**测试流程文档**：`project_document/[104]Apifox测试流程-BUG修复验证.md`

### 并发性能测试

```bash
cd backend-service/test
./test_concurrent.sh  # 100并发请求测试
```

**测试指标**：
- 并发用户数：100
- 请求总数：100
- 成功率：100%
- 平均响应时间：~45ms
- 并发响应时间：~80ms

## 开发工作流（RIPER-5）

本项目遵循RIPER-5阶段性开发工作流：

### R (Research - 研究)
**目标**: 精准理解需求
- 使用@context7获取权威信息
- 明确核心问题和用户价值
- **产出**: 清晰的需求定义、验收标准、上下文来源
- **交互**: 调用@mcp-feedback-enhanced提交研究成果

### I (Investigate - 调查)
**目标**: 深入分析并提出多种方案
- 使用@code-reasoning分析现有代码
- 使用@mcp-deepwiki查询内部知识库
- 使用@memory回忆过往决策
- **产出**: 至少两种可行方案及优缺点评估
- **交互**: 调用@mcp-feedback-enhanced提交方案选项

### P (Plan - 计划)
**目标**: 制定可执行的详细计划
- 将选定方案转化为todolist计划
- **计划文件保存至** `project_document/` 目录
- **文件命名格式**: `[编号]简要任务描述.md`
- 使用@shrimp-task-manager分解任务（可选）
- **产出**: 已保存的详细todolist计划文件
- **交互**: 调用@mcp-feedback-enhanced提交任务计划

### E (Execute - 执行)
**目标**: 高质量完成实现工作
- 严格按照计划进行编码
- **每完成一个步骤立即更新计划文档**
- 防止中断造成记忆丢失
- **产出**: 符合要求的功能代码
- **交互**: 调用@mcp-feedback-enhanced展示进度

### R (Review - 审查)
**目标**: 质量保证和知识沉淀
- 使用@code-reasoning进行代码审查
- 更新项目文档至`project_document/`
- 存储最佳实践到@memory
- **产出**: 审查报告、更新的文档、内存记忆
- **交互**: 调用@mcp-feedback-enhanced请求最终确认

### 文档同步规则

**必须同步更新的文档：**
1. **API变更** → 更新 `[000]API文档.md`
2. **架构变更** → 更新 `[002]项目架构文档.md` 和 `CLAUDE.md`
3. **数据库变更** → 更新 `[001]数据库设计文档.md` 和 `[200]数据库设计文档.md`
4. **README变更** → 同时更新 `/backend-service/README.md`

## 项目统计

### 代码规模
- **源代码文件数**: 52个
- **总代码行数**: ~11,500行
- **文档数量**: 14个
- **文档总大小**: ~250KB

### 模块分布

| 模块 | 文件数 | 代码行数(估算) | 说明 |
|------|--------|---------------|------|
| API层 (api/) | 8 | ~2,000 | Handler接口处理 |
| 业务层 (core/) | 8 | ~2,500 | Service业务逻辑 |
| 数据层 (database/) | 10 | ~2,500 | Repository数据访问 |
| 模型层 (models/) | 8 | ~1,500 | Model数据模型 |
| 安全层 (security/) | 4 | ~800 | JWT和密码管理 |
| 工具层 (utils/) | 10 | ~1,500 | 配置、日志、工具类 |
| 服务层 (server/) | 2 | ~400 | HTTP服务器 |
| 主程序 | 2 | ~300 | 入口和配置 |

### 功能模块统计

| 功能模块 | 状态 | API数量 | 数据表 |
|---------|------|---------|--------|
| 用户认证 | ✅ 已完成 | 6个 | users |
| 用户信息管理 | ✅ 已完成 (v2.1.0) | 4个 | users |
| 多图片帖子 | ✅ 已完成 (v2.0.0) | 8个 | posts, images |
| 标签系统 | ✅ 已完成 | 4个 | tags, post_tags |
| 分享系统 | ✅ 已完成 (v2.3.0) | 2个 | share_links |
| 社交互动 | ⏳ 规划中 | 预计12个 | likes, favorites, follows, comments |

### 性能指标

| 指标 | 当前值 | 目标值 |
|------|--------|--------|
| 并发连接数 | 128 | 128 |
| 线程池大小 | 32 | 32 |
| 连接池大小 | 10 | 10-20 |
| 并发处理能力 | ~100请求/秒 | ~200请求/秒 |
| 单请求响应时间 | ~45ms | <50ms |
| 并发响应时间 | ~80ms | <100ms |
| 成功率 | 100% | >99.9% |
- ## 发现的核心问题
在多图片功能测试中发现数据库一致性问题：
- **症状**: `posts.image_count` 字段与 `images` 表中的实际图片记录数量不匹配
- **影响**: 导致图片排序功能失败，数据不一致
- **根本原因**: Service层代码在图片管理操作中未正确同步更新 `posts.image_count` 字段

### 具体表现
1. **多图片上传后**: 帖子显示的 `image_count` 与实际图片数量不符
2. **图片删除后**: `posts.image_count` 未相应减少
3. **图片排序验证**: 因为数量不匹配导致排序功能失败   你先阅读这个项目，重点阅读图片、帖子管理的相关代码和存储内容，然后针对我上面说的的问题进行测试