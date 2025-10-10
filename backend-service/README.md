# Knot - Image Sharing System

一个基于C++的高性能图片分享系统后端服务，使用现代C++17技术栈构建，提供完整的用户管理和图片分享功能。

---

## 📑 目录

- [🚀 项目概述](#项目概述)
  - [核心功能](#核心功能)
  - [技术特性](#技术特性)
- [📚 项目文档](#项目文档)
  - [文档导航](#文档导航)
  - [快速入口](#快速入口)
  - [按阶段查看](#按阶段查看)
  - [技术专题](#技术专题)
  - [新人入门路径](#新人入门路径)
  - [按角色查看](#按角色查看)
- [🏗️ 技术架构](#技术架构)
  - [核心技术栈](#核心技术栈)
  - [九层架构设计](#九层架构设计)
  - [性能指标](#性能指标)
- [📁 项目结构](#项目结构)
- [🔧 环境要求](#环境要求)
  - [系统要求](#系统要求)
  - [必需依赖](#必需依赖)
  - [MySQL数据库配置](#mysql数据库配置)
- [🚀 快速开始](#快速开始)
- [📚 API文档](#api文档)
  - [认证接口（6个）](#认证接口6个)
  - [系统接口（2个）](#系统接口2个)
  - [快速示例](#快速示例)
- [🔒 安全特性](#安全特性)
- [🚀 性能优化](#性能优化)
  - [连接池优化](#连接池优化)
  - [并发优化](#并发优化)
  - [性能指标](#性能指标-1)
- [📊 监控与健康检查](#监控与健康检查)
- [🛠️ 开发指南](#开发指南)
  - [编译选项](#编译选项)
  - [日志级别](#日志级别)
  - [代码规范](#代码规范)
  - [文档](#文档)
- [📈 项目进度](#项目进度)
  - [已完成功能 ✅](#已完成功能-)
  - [规划中功能 ⏳](#规划中功能-)
- [📝 待办事项](#待办事项)
- [🎯 技术亮点](#技术亮点)
- [📊 代码统计](#代码统计)
- [📄 许可证](#许可证)
- [👥 贡献者](#贡献者)
- [📞 联系方式](#联系方式)

---

## 🚀 项目概述

本项目实现了一个完整的图片分享系统后端，包括：

### 核心功能
- **用户认证系统**（已完成✅）
  - 用户注册与登录
  - JWT令牌管理（生成、验证、刷新）
  - 安全的密码哈希存储（PBKDF2-HMAC-SHA256）
  - 多设备登录支持
  - 🆕 **用户信息管理** (v2.1.0)
    - 获取当前用户信息
    - 修改用户信息（真实姓名、邮箱、头像、手机号、个人简介、性别、所在地）
    - 获取其他用户公开信息
    - 用户名可用性检查

- **图片管理系统**（待开发⏳）
  - 图片上传与存储
  - 图片压缩与缩略图生成
  - 图片浏览与Feed推荐
  - 互动系统（点赞、收藏、关注）
  - 分享功能（Deep Link唤醒）

### 技术特性
- RESTful API接口
- 数据库连接池管理（RAII自动管理）
- 高并发处理能力（支持100+并发用户）
- 完善的日志记录与监控
- 九层架构设计

## 📚 项目文档

### 文档导航

本项目提供完整的技术文档，位于 `project_document/` 目录。

#### 快速入口

- **[📖 文档总览](project_document/README.md)** - 项目开发方案和文档导航（必读 ⭐）
- **[📋 API文档](project_document/[000]API文档.md)** - 所有API接口的完整说明
- **[💾 数据库设计](project_document/[001]数据库设计文档.md)** - 数据库表结构和关系设计
- **[🏗️ 架构文档](project_document/[002]项目架构文档.md)** - 九层架构设计和技术栈
- **[⚙️ 环境配置](project_document/[003]环境安装配置教程.md)** - 开发环境搭建指南

#### 按阶段查看

| 阶段 | 文档 | 说明 |
|------|------|------|
| 阶段A | [基础模块实现](project_document/[100]阶段A-基础模块实现.md) | 项目骨架和基础设施 |
| 阶段B | [用户认证模块](project_document/[101]阶段B-用户认证模块.md) | 用户认证系统完整实现 |
| 阶段C | 图片管理模块 | 待开发⏳ |

#### 技术专题

| 专题 | 文档 | 说明 |
|------|------|------|
| 数据库 | [数据库设计文档](project_document/[200]数据库设计文档.md) | 所有表结构和关系设计 |
| 性能 | [性能优化专题](project_document/[201]性能优化专题.md) | 连接池和并发性能优化 |

#### 新人入门路径

如果你是新加入项目的开发者，建议按以下顺序阅读：

1. **[文档总览](project_document/README.md)** - 了解项目开发方案和文档结构
2. **[项目架构文档](project_document/[002]项目架构文档.md)** - 了解整体架构
3. **[环境安装配置教程](project_document/[003]环境安装配置教程.md)** - 搭建开发环境
4. **[API文档](project_document/[000]API文档.md)** - 了解所有API接口
5. **[数据库设计文档](project_document/[001]数据库设计文档.md)** - 了解数据库结构
6. **阶段文档** - 按需阅读各阶段的实现细节

#### 按角色查看

- **前端开发者**: [API文档](project_document/[000]API文档.md) → [项目架构文档](project_document/[002]项目架构文档.md)
- **后端开发者**: [文档总览](project_document/README.md) → [项目架构文档](project_document/[002]项目架构文档.md) → [数据库设计文档](project_document/[001]数据库设计文档.md) → 阶段文档
- **运维工程师**: [环境安装配置教程](project_document/[003]环境安装配置教程.md) → [性能优化专题](project_document/[201]性能优化专题.md)
- **项目经理**: [文档总览](project_document/README.md) → [项目架构文档](project_document/[002]项目架构文档.md)

---

## 🏗️ 技术架构

### 核心技术栈
- **C++17**: 现代C++标准，提供更好的性能和安全性
- **cpp-httplib 0.11.0**: 轻量级HTTP服务器库，支持高并发
- **MySQL 8.0.43**: 生产级关系型数据库
- **jwt-cpp 0.6.0**: JWT令牌处理库
- **JsonCpp 1.9.5**: JSON解析库
- **OpenSSL 1.1.1**: 密码学库，用于密码哈希和HTTPS支持
- **spdlog 1.9.2**: 高性能日志库

### 九层架构设计

```
第1层：客户端层（HTTP客户端）
    ↓
第2层：HTTP服务器层（HTTPServer）
    ↓
第3层：API接口层（Handler）
    ├── AuthHandler（用户认证）
    ├── PostHandler（帖子管理）
    ├── ImageHandler（图片管理）
    └── TagHandler（标签管理）
    ↓
第4层：业务逻辑层（Service）
    ├── AuthService（认证业务）
    ├── PostService（帖子业务）
    ├── ImageService（图片业务）
    └── TagService（标签业务）
    ↓
第5层：数据访问层（Repository）
    ├── UserRepository（用户数据访问）
    ├── PostRepository（帖子数据访问）
    ├── ImageRepository（图片数据访问）
    └── TagRepository（标签数据访问）
    ↓
第6层：安全层
    ├── JWTManager（JWT管理）
    └── PasswordHasher（密码哈希）
    ↓
第7层：基础设施层
    ├── DatabaseConnectionPool（连接池）
    ├── ConnectionGuard（RAII连接管理）
    ├── ConfigManager（配置管理）
    └── Logger（日志系统）
    ↓
第8层：数据模型层（Model）
    ├── User（用户模型）
    ├── Post（帖子模型）
    ├── Image（图片模型）
    └── Tag（标签模型）
    ↓
第9层：数据库层（MySQL）
    ├── users表
    ├── posts表
    ├── images表
    ├── tags表
    └── post_tags表
```

### 性能指标

| 指标 | 数值 |
|------|------|
| 并发连接数 | 128 |
| 线程池大小 | 32 |
| 连接池大小 | 10 |
| 并发处理能力 | ~100请求/秒 |
| 支持用户数 | ~100人 |
| 单请求响应时间 | ~45ms |

## 📁 项目结构

```
Knot/
├── CMakeLists.txt              # CMake构建配置
├── README.md                   # 项目说明文档
├── config/
│   ├── config.json            # 应用配置文件
│   └── database.sql           # 数据库初始化脚本
├── project_document/          # 项目文档目录
│   ├── README.md              # 文档说明
│   ├── [000]API文档.md        # API接口文档
│   ├── [001]数据库设计文档.md # 数据库设计文档
│   ├── [002]项目架构文档.md   # 架构设计文档
│   ├── [003]环境安装配置教程.md # 环境搭建指南
│   ├── [100]阶段A-基础模块实现.md # 基础模块文档
│   ├── [101]阶段B-用户认证模块.md # 用户认证文档
│   ├── [102]阶段C-图片管理模块.md # 图片管理文档
│   ├── [105]阶段C-2-多图片帖子系统.md # 多图片帖子系统文档
│   ├── [106]用户信息管理功能完善.md # 用户信息管理文档
│   ├── [200]数据库设计文档.md # 数据库设计文档
│   └── [201]性能优化专题.md   # 性能优化文档
├── src/
│   ├── main.cpp               # 程序入口
│   ├── api/                   # API接口层
│   │   ├── auth_handler.h/cpp # 认证接口处理
│   │   ├── post_handler.h/cpp # 帖子接口处理
│   │   ├── image_handler.h/cpp # 图片接口处理
│   │   └── tag_handler.h/cpp  # 标签接口处理
│   ├── core/                  # 核心业务逻辑层
│   │   ├── auth_service.h/cpp # 认证服务
│   │   ├── post_service.h/cpp # 帖子服务
│   │   ├── image_service.h/cpp # 图片服务
│   │   └── tag_service.h/cpp  # 标签服务
│   ├── database/              # 数据访问层
│   │   ├── connection_pool.h/cpp # 数据库连接池
│   │   ├── connection_guard.h/cpp # RAII连接管理
│   │   ├── user_repository.h/cpp # 用户数据访问
│   │   ├── post_repository.h/cpp # 帖子数据访问
│   │   ├── image_repository.h/cpp # 图片数据访问
│   │   └── tag_repository.h/cpp # 标签数据访问
│   ├── models/                # 数据模型层
│   │   ├── user.h/cpp         # 用户模型
│   │   ├── post.h/cpp         # 帖子模型
│   │   ├── image.h/cpp        # 图片模型
│   │   └── tag.h/cpp          # 标签模型
│   ├── security/              # 安全层
│   │   ├── jwt_manager.h/cpp  # JWT管理
│   │   └── password_hasher.h/cpp # 密码哈希
│   ├── server/                # HTTP服务器层
│   │   └── http_server.h/cpp  # 服务器封装
│   └── utils/                 # 工具类
│       ├── config_manager.h/cpp # 配置管理
│       ├── logger.h/cpp       # 日志系统
│       ├── json_response.h/cpp # JSON响应工具
│       └── image_processor.h/cpp # 图片处理工具
├── third_party/               # 第三方库
│   ├── httplib.h             # cpp-httplib
│   ├── json/                 # JsonCpp
│   └── jwt-cpp/              # jwt-cpp库
├── test/                      # 测试目录
├── build/                     # 构建输出目录
├── uploads/                   # 图片上传目录
│   ├── images/               # 原图存储
│   └── thumbnails/           # 缩略图存储
└── logs/                      # 日志文件目录
```

## 🔧 环境要求

### 系统要求
- **操作系统**: Linux (Ubuntu 20.04+ 推荐)
- **编译器**: GCC 9+ 或 Clang 10+ (支持C++17)
- **CMake**: 3.16+

### 必需依赖
```bash
# Ubuntu/Debian系统
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    pkg-config \
    libmysqlclient-dev \
    libssl-dev \
    git \
    curl \
    mysql-server \
    mysql-client
```

### MySQL数据库配置
1. **启动MySQL服务**:
   ```bash
   sudo systemctl start mysql
   sudo systemctl enable mysql
   ```

2. **创建数据库和用户**:
   ```sql
   CREATE DATABASE knot_image_sharing CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
   CREATE USER 'knot_user'@'localhost' IDENTIFIED BY 'secure_password';
   GRANT ALL PRIVILEGES ON knot_image_sharing.* TO 'knot_user'@'localhost';
   FLUSH PRIVILEGES;
   ```

3. **初始化数据库表**:
   ```bash
   mysql -u knot_user -p knot_image_sharing < config/database.sql
   ```

## 🚀 快速开始

### 1. 克隆项目
```bash
git clone <repository-url>
cd Knot/backend-service
```

### 2. 安装依赖
```bash
# 安装系统依赖
sudo apt install -y build-essential cmake pkg-config libmysqlclient-dev libssl-dev

# 第三方库已包含在项目中，无需额外安装
```

### 3. 配置应用
编辑 `config/config.json` 文件，修改数据库连接信息：
```json
{
  "database": {
    "host": "localhost",
    "port": 3306,
    "database": "knot_image_sharing",
    "username": "knot_user",
    "password": "secure_password"
  }
}
```

### 4. 编译项目
```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

### 5. 运行服务
```bash
./knot_image_sharing ../config/config.json
```

## 📚 API文档

### 在线交互式文档（推荐）

我们提供了基于 OpenAPI 3.0 规范的在线交互式 API 文档，支持直接在浏览器中测试所有接口：

**启动文档服务**:
```bash
cd docs
./serve.sh
```

**访问地址**:
- 本地访问: http://localhost:3000
- 局域网访问: http://192.168.32.128:3000

**文档特性**:
- 🎯 交互式接口测试
- 📖 完整的参数说明和示例
- 🔐 内置 JWT 令牌管理
- 📱 响应式设计，支持移动端
- 🚀 快速开始指南

**相关文档**:
- [API 文档使用指南](docs/README.md)
- [快速入门教程](docs/QUICKSTART.md)
- [OpenAPI 规范文件](docs/openapi.yaml)

### Markdown 格式文档

完整的 Markdown 格式 API 文档请查看：[project_document/[000]API文档.md](project_document/[000]API文档.md)

### 认证接口（6个）

| 接口 | 方法 | 路径 | 说明 |
|------|------|------|------|
| 用户注册 | POST | /api/v1/auth/register | 注册新用户 |
| 用户登录 | POST | /api/v1/auth/login | 用户登录 |
| 令牌验证 | GET | /api/v1/auth/verify | 验证访问令牌 |
| 令牌刷新 | POST | /api/v1/auth/refresh | 刷新访问令牌 |
| 用户登出 | POST | /api/v1/auth/logout | 用户登出 |
| 获取用户信息 | GET | /api/v1/auth/me | 获取当前用户信息 |

### 🆕 用户信息管理接口（4个）v2.1.0

| 接口 | 方法 | 路径 | 说明 |
|------|------|------|------|
| 获取当前用户信息 | GET | /api/v1/users/profile | 获取当前用户完整信息 |
| 修改用户信息 | PUT | /api/v1/users/profile | 修改用户信息 |
| 获取用户公开信息 | GET | /api/v1/users/:user_id | 获取其他用户公开信息 |
| 用户名可用性检查 | GET | /api/v1/users/check-username | 检查用户名是否可用 |

### 系统接口（2个）

| 接口 | 方法 | 路径 | 说明 |
|------|------|------|------|
| 健康检查 | GET | /health | 服务健康检查 |
| 服务指标 | GET | /metrics | 服务性能指标 |

### 快速示例

#### 用户注册
```bash
curl -X POST http://localhost:8080/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "Test123456",
    "real_name": "测试用户",
    "phone": "13800138000"
  }'
```

### 响应格式
```json
{
  "success": true,
  "message": "操作成功",
  "data": { ... },
  "timestamp": 1696147200
}
```

## 🔒 安全特性

- **密码哈希**: 使用PBKDF2-HMAC-SHA256算法，100,000次迭代，每用户独立盐值
- **JWT令牌**: 支持访问令牌（1小时）和刷新令牌（24小时）机制
- **SQL注入防护**: 使用预编译语句和参数绑定
- **输入验证**: 严格的参数验证和数据校验
- **权限控制**: JWT令牌验证和用户权限检查
- **日志记录**: 详细的操作日志和安全审计

## 🚀 性能优化

### 连接池优化
- **RAII自动管理**: 使用ConnectionGuard自动管理数据库连接
- **异常安全**: 确保连接在任何情况下都能正确归还
- **防止泄漏**: 消除连接泄漏问题，100%可靠性

### 并发优化
- **Listen Backlog**: 128（支持128个并发连接）
- **Thread Pool**: 32个线程（支持高并发处理）
- **Connection Pool**: 10个数据库连接
- **并发能力**: 支持100+并发用户，~100请求/秒

### 性能指标
- 单请求响应时间: ~45ms
- 并发响应时间: ~80ms
- 吞吐量: ~100请求/秒
- 成功率: 100%

## 📊 监控与健康检查

### 健康检查
```bash
curl http://localhost:8080/health
```

响应示例：
```json
{
  "status": "healthy",
  "service": "Knot - Image Sharing Service",
  "database": "connected",
  "timestamp": 1696147200
}
```

### 服务指标
```bash
curl http://localhost:8080/metrics
```

## 🛠️ 开发指南

### 编译选项
- **Debug模式**: `cmake -DCMAKE_BUILD_TYPE=Debug ..`
- **Release模式**: `cmake -DCMAKE_BUILD_TYPE=Release ..`

### 日志级别
- DEBUG: 详细调试信息
- INFO: 一般信息
- WARNING: 警告信息
- ERROR: 错误信息
- FATAL: 致命错误

### 代码规范
- 使用C++17标准
- 遵循九层架构设计
- 使用RAII管理资源
- 使用智能指针管理内存
- 详细的中文注释

### 文档
完整的技术文档位于 `project_document/` 目录：
- **[000]API文档.md**: 所有API接口说明
- **[001]数据库设计文档.md**: 数据库表结构和关系设计
- **[002]项目架构文档.md**: 架构设计和技术栈
- **[003]环境安装配置教程.md**: 环境搭建指南
- **[100]阶段A-基础模块实现.md**: 基础模块实现
- **[101]阶段B-用户认证模块.md**: 用户认证系统
- **[102]阶段C-图片管理模块.md**: 图片管理系统
- **[105]阶段C-2-多图片帖子系统.md**: 多图片帖子系统
- **[201]性能优化专题.md**: 性能优化方案

## 📈 项目进度

### 已完成功能 ✅

#### 阶段A: 基础模块
- ✅ 项目骨架搭建
- ✅ 数据库连接池
- ✅ 配置管理系统
- ✅ 日志系统
- ✅ HTTP服务器封装

#### 阶段B: 用户认证模块
- ✅ 用户注册
- ✅ 用户登录
- ✅ JWT令牌管理
- ✅ 密码哈希（PBKDF2-HMAC-SHA256）
- ✅ 令牌验证和刷新
- ✅ 用户信息查询
- 🆕 **用户信息管理** (v2.1.0)
  - ✅ 获取当前用户信息
  - ✅ 修改用户信息（支持个人简介、性别、所在地等新字段）
  - ✅ 获取其他用户公开信息
  - ✅ 用户名可用性检查

#### 阶段C: 图片管理模块
- ✅ 多图片帖子发布（1-9张图片）
- ✅ 图片压缩与缩略图生成
- ✅ 帖子编辑与删除
- ✅ 图片顺序管理
- ✅ Feed推荐算法
- ✅ 标签系统

### 规划中功能 ⏳

#### 阶段D: 社交互动模块
- ⏳ 点赞功能
- ⏳ 收藏功能
- ⏳ 关注/粉丝系统
- ⏳ 评论功能

#### 阶段E: 分享系统
- ⏳ Deep Link生成
- ⏳ 分享统计
- ⏳ 外部平台分享

#### 阶段F: 系统优化
- ⏳ Redis缓存
- ⏳ 读写分离
- ⏳ 负载均衡

## 📝 待办事项

- [ ] 添加单元测试
- [ ] 集成测试
- [ ] 压力测试
- [ ] CI/CD流程
- [ ] 监控指标完善
- [ ] Redis缓存
- [ ] 读写分离
- [ ] 负载均衡

## 🎯 技术亮点

### 1. RAII资源管理
- 使用ConnectionGuard自动管理数据库连接
- 异常安全，防止资源泄漏
- 代码简洁，易于维护

### 2. 高并发处理
- Listen Backlog: 128
- Thread Pool: 32
- 并发能力提升20倍

### 3. 安全性
- PBKDF2-HMAC-SHA256密码哈希
- JWT令牌认证
- SQL注入防护
- 详细的安全审计日志

### 4. 九层架构
- 清晰的层次结构
- 职责分离
- 易于扩展和维护

### 5. 完善的文档
- 8个核心技术文档
- 详细的API文档
- 完整的架构说明
- 性能优化专题

## 📊 代码统计

| 模块 | 文件数 | 代码行数 |
|------|--------|----------|
| 基础模块 | ~20 | ~3000 |
| 用户认证模块 | 12 | ~2000 |
| 图片管理模块 | 16 | ~3500 |
| **总计** | **~48** | **~8500** |

## 📄 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 👥 贡献者

- Knot Development Team

## 📞 联系方式

如有问题或建议，请通过以下方式联系：
- 项目Issue
- 技术文档：`project_document/`

---

**项目状态**: 开发中
**当前版本**: v2.1.0
**最后更新**: 2025-10-09

**注意**: 本项目目前处于开发阶段，已完成用户认证和停车位管理核心功能。在生产环境使用前，请确保完成所有安全配置和测试。
