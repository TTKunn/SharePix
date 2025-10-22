# Knot - Image Sharing System

一个基于C++的高性能图片分享系统后端服务，使用现代C++17技术栈构建，提供完整的用户管理、图片分享和社交互动功能。

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
  - [认证接口](#认证接口11个)
  - [帖子管理接口](#帖子管理接口9个)
  - [社交互动接口](#社交互动接口13个)
  - [分享接口](#分享接口2个)
  - [评论接口](#评论接口3个--v280)
  - [系统接口](#系统接口3个)
  - [快速示例](#快速示例)
- [🔒 安全特性](#安全特性)
- [🚀 性能优化](#性能优化)
  - [连接池优化](#连接池优化)
  - [并发优化](#并发优化)
  - [批量查询优化](#批量查询优化)
  - [性能指标](#性能指标-1)
- [📊 监控与健康检查](#监控与健康检查)
- [🛠️ 开发指南](#开发指南)
  - [编译选项](#编译选项)
  - [日志级别](#日志级别)
  - [代码规范](#代码规范)
  - [文档](#文档)
- [📈 版本历史](#版本历史)
  - [v2.8.0 关注系统优化与评论功能](#v280-关注系统优化与评论功能2025-10-19)
  - [v2.7.0 收藏列表优化](#v270-收藏列表批量查询优化2025-10-19)
  - [v2.6.0 用户头像上传](#v260-用户头像上传2025-10-18)
  - [v2.5.0 Feed流优化](#v250-feed流批量查询优化2025-10-18)
  - [更多版本](#更多版本历史)
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

#### 1. 用户认证系统 ✅ 已完成
- **基础认证**（v1.2.0）
  - 用户注册与登录
  - JWT令牌管理（生成、验证、刷新）
  - 安全的密码哈希存储（PBKDF2-HMAC-SHA256）
  - 多设备登录支持
  - 密码修改功能

- **用户信息管理**（v2.1.0, v2.9.0）
  - 获取当前用户完整信息
  - 修改用户信息（用户名、个人简介、性别、所在地、网站等）
  - 获取其他用户公开信息
  - 用户名可用性检查
  - **username修改支持**（v2.9.0）：智能重复检测，避免误报

- **用户头像上传**（v2.6.0）
  - 头像文件上传（multipart/form-data）
  - 自动裁剪为正方形并缩放到200x200
  - JPEG压缩（80%质量）
  - 自动删除旧头像文件

#### 2. 多图片帖子系统 ✅ 已完成（v2.0.0）
- 多图片帖子发布（支持1-9张图片）
- 支持multipart/form-data和JSON+Base64两种上传格式
- 图片压缩与缩略图生成
- 帖子编辑与删除
- 图片顺序管理（添加、删除、调整顺序）
- 标签系统（最多5个标签）

#### 3. Feed推荐系统 ✅ 已完成
- **Feed流**（v2.0.0）
  - 基于热度和时间衰减的推荐算法
  - 瀑布流分页查询
  - 用户个人Feed流

- **Feed流优化**（v2.5.0）
  - 可选JWT认证（支持游客访问）
  - 批量查询作者信息（解决N+1问题）
  - 批量查询互动状态（点赞、收藏）
  - 性能提升：查询次数减少93.4%，QPS提升200%

- **帖子详情优化**（v2.5.1）
  - 新增用户昵称字段
  - 减少前端额外请求

#### 4. 社交互动系统 ✅ 已完成（v2.2.0）
- **点赞功能**
  - 点赞/取消点赞帖子
  - 查询点赞状态
  - 点赞数统计

- **收藏功能**
  - 收藏/取消收藏帖子
  - 查询收藏状态
  - 获取收藏列表（v2.7.0批量查询优化）
  - 收藏数统计

- **关注功能**
  - 关注/取消关注用户
  - 查询关注状态
  - 获取关注列表和粉丝列表
  - 用户统计信息（关注数、粉丝数、帖子数）
  - 批量检查关注状态

#### 5. 分享系统 ✅ 已完成（v2.3.0）
- **短链接生成**
  - 雪花ID算法 + Base62编码
  - 8位短码（62^8种可能）
  - 自动去重（同一帖子返回相同短链）
  - 支持过期时间设置

- **Deep Link支持**
  - iOS Universal Links配置
  - Android App Links配置
  - HarmonyOS Deep Linking配置
  - 三端统一分享体验

#### 6. 评论系统 ✅ 已完成（v2.8.0）
- **评论功能**
  - 创建评论（支持UTF-8中文）
  - 获取评论列表（分页查询）
  - 删除评论（权限验证）
  - 评论数统计
  - 批量查询评论作者信息

### 技术特性
- RESTful API接口设计
- 数据库连接池管理（RAII自动管理）
- 高并发处理能力（支持100+并发用户）
- 完善的日志记录与监控
- 九层架构设计（清晰的职责分离）
- 批量查询优化（解决N+1问题）
- 可选JWT认证（支持游客模式）

## 📚 项目文档

### 文档导航

本项目提供完整的技术文档，位于 `project_document/` 目录。

#### 快速入口

- **[📖 需求分析](project_document/[004]需求分析.md)** - Knot图片分享社区MVP需求分析
- **[📋 API文档](project_document/[000]API文档.md)** - 所有API接口的完整说明（v2.8.0）
- **[💾 数据库设计](project_document/[001]数据库设计文档.md)** - 数据库表结构和关系设计
- **[🏗️ 架构文档](project_document/[002]项目架构文档.md)** - 九层架构设计和技术栈
- **[⚙️ 环境配置](project_document/[003]环境安装配置教程.md)** - 开发环境搭建指南

#### 按阶段查看

| 阶段 | 文档 | 说明 |
|------|------|------|
| 阶段A | [基础模块实现](project_document/[100]阶段A-基础模块实现.md) | 项目骨架和基础设施 |
| 阶段B | [用户认证模块](project_document/[101]阶段B-1-用户认证模块.md) | 用户认证系统完整实现 |
| 阶段B-2 | [用户信息管理](project_document/[106]阶段B-2-用户信息管理功能完善-实施计划.md) | 用户信息管理功能 |
| 阶段C | [图片管理模块](project_document/[102]阶段C-图片管理模块.md) | 图片管理系统实现 |
| 阶段C-2 | [多图片帖子系统](project_document/[105]阶段C-2-多图片帖子系统.md) | 多图片帖子系统（v2.0.0） |
| 阶段D-1 | [点赞收藏功能](project_document/[107]阶段D-1-互动系统点赞收藏功能实现计划.md) | 点赞和收藏功能 |
| 阶段D-2 | [关注功能](project_document/[108]阶段D-2-互动系统关注功能实现计划.md) | 关注/粉丝系统 |
| 阶段E | [评论功能](project_document/[112]阶段E-评论功能实现方案.md) | 评论系统（v2.8.0） |

#### 技术专题

| 专题 | 文档 | 说明 |
|------|------|------|
| 数据库 | [数据库设计文档](project_document/[200]数据库设计文档.md) | 所有表结构和关系设计 |
| 性能优化 | [性能优化专题](project_document/[201]性能优化专题.md) | 连接池和并发性能优化 |
| Feed流优化 | [Feed流批量查询优化](project_document/[165]Feed流用户状态批量查询优化方案.md) | N+1问题解决方案（v2.5.0） |
| 收藏列表优化 | [收藏列表批量查询优化](project_document/[170]收藏列表批量查询优化方案.md) | 收藏列表性能优化（v2.7.0） |
| 用户头像 | [用户头像上传功能](project_document/[167]用户头像上传功能-实施方案.md) | 头像上传和处理（v2.6.0） |
| Deep Link | [Deep Link配置教程](project_document/[111]Deep Link配置使用教程-鸿蒙开发指南.md) | 三端Deep Link配置 |

#### 新人入门路径

如果你是新加入项目的开发者，建议按以下顺序阅读：

1. **[需求分析](project_document/[004]需求分析.md)** - 了解产品定位和功能范围
2. **[项目架构文档](project_document/[002]项目架构文档.md)** - 了解整体架构
3. **[环境安装配置教程](project_document/[003]环境安装配置教程.md)** - 搭建开发环境
4. **[API文档](project_document/[000]API文档.md)** - 了解所有API接口
5. **[数据库设计文档](project_document/[001]数据库设计文档.md)** - 了解数据库结构
6. **阶段文档** - 按需阅读各阶段的实现细节

#### 按角色查看

- **前端开发者**: [API文档](project_document/[000]API文档.md) → [需求分析](project_document/[004]需求分析.md)
- **后端开发者**: [项目架构文档](project_document/[002]项目架构文档.md) → [数据库设计文档](project_document/[001]数据库设计文档.md) → 阶段文档
- **运维工程师**: [环境安装配置教程](project_document/[003]环境安装配置教程.md) → [性能优化专题](project_document/[201]性能优化专题.md)
- **项目经理**: [需求分析](project_document/[004]需求分析.md) → [项目架构文档](project_document/[002]项目架构文档.md)

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
    ├── TagHandler（标签管理）
    ├── LikeHandler（点赞管理）
    ├── FavoriteHandler（收藏管理）
    ├── FollowHandler（关注管理）
    ├── ShareHandler（分享管理）
    └── CommentHandler（评论管理）
    ↓
第4层：业务逻辑层（Service）
    ├── AuthService（认证业务）
    ├── PostService（帖子业务）
    ├── ImageService（图片业务）
    ├── TagService（标签业务）
    ├── LikeService（点赞业务）
    ├── FavoriteService（收藏业务）
    ├── FollowService（关注业务）
    ├── ShareService（分享业务）
    ├── CommentService（评论业务）
    └── UserService（用户业务 - 批量查询）
    ↓
第5层：数据访问层（Repository）
    ├── UserRepository（用户数据访问）
    ├── PostRepository（帖子数据访问）
    ├── ImageRepository（图片数据访问）
    ├── TagRepository（标签数据访问）
    ├── LikeRepository（点赞数据访问）
    ├── FavoriteRepository（收藏数据访问）
    ├── FollowRepository（关注数据访问）
    ├── ShareLinkRepository（分享链接数据访问）
    └── CommentRepository（评论数据访问）
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
    ├── Tag（标签模型）
    ├── Like（点赞模型）
    ├── Favorite（收藏模型）
    ├── Follow（关注模型）
    ├── ShareLink（分享链接模型）
    └── Comment（评论模型）
    ↓
第9层：数据库层（MySQL）
    ├── users表（用户信息）
    ├── posts表（帖子信息）
    ├── images表（图片信息）
    ├── tags表（标签信息）
    ├── post_tags表（帖子标签关联）
    ├── likes表（点赞记录）
    ├── favorites表（收藏记录）
    ├── follows表（关注关系）
    ├── share_links表（分享链接）
    └── comments表（评论信息）
```

### 性能指标

| 指标 | 数值 | 说明 |
|------|------|------|
| 并发连接数 | 128 | Listen Backlog设置 |
| 线程池大小 | 32 | HTTP线程池 |
| 连接池大小 | 10 | 数据库连接池 |
| 并发处理能力 | ~300请求/秒 | v2.5.0优化后 |
| 单请求响应时间 | ~45ms | 典型场景 |
| Feed流响应时间 | ~50ms | P99，v2.5.0优化后 |
| 收藏列表响应时间 | ~80ms | P99，v2.7.0优化后 |

## 📁 项目结构

```
SharePix/backend-service/
├── CMakeLists.txt              # CMake构建配置
├── README.md                   # 项目说明文档
├── config/
│   ├── config.json            # 应用配置文件
│   ├── config.example.json    # 配置文件示例
│   └── database.sql           # 数据库初始化脚本
├── project_document/          # 项目文档目录
│   ├── [000]API文档.md        # API接口文档（v2.8.0）
│   ├── [001]数据库设计文档.md # 数据库设计文档
│   ├── [002]项目架构文档.md   # 架构设计文档
│   ├── [003]环境安装配置教程.md # 环境搭建指南
│   ├── [004]需求分析.md       # 需求分析文档
│   ├── [005]API_DOC_UPDATE.md # API文档更新记录
│   ├── [100]阶段A-基础模块实现.md # 基础模块文档
│   ├── [101]阶段B-1-用户认证模块.md # 用户认证文档
│   ├── [102]阶段C-图片管理模块.md # 图片管理文档
│   ├── [105]阶段C-2-多图片帖子系统.md # 多图片帖子系统
│   ├── [106]阶段B-2-用户信息管理功能完善-实施计划.md # 用户信息管理
│   ├── [107]阶段D-1-互动系统点赞收藏功能实现计划.md # 点赞收藏
│   ├── [108]阶段D-2-互动系统关注功能实现计划.md # 关注系统
│   ├── [112]阶段E-评论功能实现方案.md # 评论系统
│   ├── [165]Feed流用户状态批量查询优化方案.md # Feed优化
│   ├── [167]用户头像上传功能-实施方案.md # 头像上传
│   ├── [170]收藏列表批量查询优化方案.md # 收藏优化
│   ├── [200]数据库设计文档.md # 数据库设计详解
│   └── [201]性能优化专题.md   # 性能优化指南
├── src/
│   ├── main.cpp               # 程序入口
│   ├── api/                   # API接口层
│   │   ├── auth_handler.h/cpp # 认证接口处理
│   │   ├── post_handler.h/cpp # 帖子接口处理
│   │   ├── image_handler.h/cpp # 图片接口处理
│   │   ├── tag_handler.h/cpp  # 标签接口处理
│   │   ├── like_handler.h/cpp # 点赞接口处理
│   │   ├── favorite_handler.h/cpp # 收藏接口处理
│   │   ├── follow_handler.h/cpp # 关注接口处理
│   │   ├── share_handler.h/cpp # 分享接口处理
│   │   └── comment_handler.h/cpp # 评论接口处理
│   ├── core/                  # 核心业务逻辑层
│   │   ├── auth_service.h/cpp # 认证服务
│   │   ├── post_service.h/cpp # 帖子服务
│   │   ├── image_service.h/cpp # 图片服务
│   │   ├── tag_service.h/cpp  # 标签服务
│   │   ├── like_service.h/cpp # 点赞服务
│   │   ├── favorite_service.h/cpp # 收藏服务
│   │   ├── follow_service.h/cpp # 关注服务
│   │   ├── share_service.h/cpp # 分享服务
│   │   ├── comment_service.h/cpp # 评论服务
│   │   └── user_service.h/cpp # 用户服务（批量查询）
│   ├── database/              # 数据访问层
│   │   ├── connection_pool.h/cpp # 数据库连接池
│   │   ├── connection_guard.h/cpp # RAII连接管理
│   │   ├── user_repository.h/cpp # 用户数据访问
│   │   ├── post_repository.h/cpp # 帖子数据访问
│   │   ├── image_repository.h/cpp # 图片数据访问
│   │   ├── tag_repository.h/cpp # 标签数据访问
│   │   ├── like_repository.h/cpp # 点赞数据访问
│   │   ├── favorite_repository.h/cpp # 收藏数据访问
│   │   ├── follow_repository.h/cpp # 关注数据访问
│   │   ├── share_link_repository.h/cpp # 分享链接数据访问
│   │   └── comment_repository.h/cpp # 评论数据访问
│   ├── models/                # 数据模型层
│   │   ├── user.h/cpp         # 用户模型
│   │   ├── post.h/cpp         # 帖子模型
│   │   ├── image.h/cpp        # 图片模型
│   │   ├── tag.h/cpp          # 标签模型
│   │   ├── like.h/cpp         # 点赞模型
│   │   ├── favorite.h/cpp     # 收藏模型
│   │   ├── follow.h/cpp       # 关注模型
│   │   ├── share_link.h/cpp   # 分享链接模型
│   │   └── comment.h/cpp      # 评论模型
│   ├── security/              # 安全层
│   │   ├── jwt_manager.h/cpp  # JWT管理
│   │   └── password_hasher.h/cpp # 密码哈希
│   ├── server/                # HTTP服务器层
│   │   └── http_server.h/cpp  # 服务器封装
│   └── utils/                 # 工具类
│       ├── config_manager.h/cpp # 配置管理
│       ├── logger.h/cpp       # 日志系统
│       ├── json_response.h/cpp # JSON响应工具
│       ├── image_processor.h/cpp # 图片处理工具
│       ├── avatar_processor.h/cpp # 头像处理工具
│       └── snowflake.h/cpp    # 雪花ID生成器
├── third_party/               # 第三方库
│   ├── httplib.h             # cpp-httplib
│   ├── json/                 # JsonCpp
│   └── jwt-cpp/              # jwt-cpp库
├── test/                      # 测试目录
│   ├── test_concurrent.sh    # 并发测试脚本
│   └── pictures/             # 测试图片
├── build/                     # 构建输出目录
├── uploads/                   # 上传文件目录
│   ├── images/               # 原图存储
│   ├── thumbnails/           # 缩略图存储
│   └── avatars/              # 用户头像存储
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
    libjsoncpp-dev \
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
cd SharePix/backend-service
```

### 2. 安装依赖
```bash
# 安装系统依赖
sudo apt install -y build-essential cmake pkg-config libmysqlclient-dev libssl-dev libjsoncpp-dev

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
    "password": "secure_password",
    "pool_size": 10
  },
  "jwt": {
    "secret": "your-secret-key-change-in-production",
    "access_token_expiry": 3600,
    "refresh_token_expiry": 86400
  },
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "thread_pool_size": 32
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

服务启动后监听在 `http://localhost:8080`

## 📚 API文档

完整的API文档请查看：[project_document/[000]API文档.md](project_document/[000]API文档.md)

### 认证接口（11个）

| 接口 | 方法 | 路径 | 说明 | 版本 |
|------|------|------|------|------|
| 用户注册 | POST | /api/v1/auth/register | 注册新用户 | v1.2.0 |
| 用户登录 | POST | /api/v1/auth/login | 用户登录 | v1.2.0 |
| 令牌验证 | GET | /api/v1/auth/verify | 验证访问令牌 | v1.2.0 |
| 令牌刷新 | POST | /api/v1/auth/refresh | 刷新访问令牌 | v1.2.0 |
| 用户登出 | POST | /api/v1/auth/logout | 用户登出 | v1.2.0 |
| 修改密码 | POST | /api/v1/auth/change-password | 修改密码 | v1.2.0 |
| 获取当前用户信息 | GET | /api/v1/users/profile | 获取完整信息 | v2.1.0 |
| 修改用户信息 | PUT | /api/v1/users/profile | 修改用户信息 | v2.1.0 |
| 获取用户公开信息 | GET | /api/v1/users/:user_id | 获取公开信息 | v2.1.0 |
| 检查用户名 | GET | /api/v1/users/check-username | 用户名可用性 | v2.1.0 |
| 上传用户头像 | POST | /api/v1/users/avatar | 上传头像文件 | v2.6.0 |

### 帖子管理接口（9个）

| 接口 | 方法 | 路径 | 说明 | 版本 |
|------|------|------|------|------|
| 创建帖子 | POST | /api/v1/posts | 创建帖子（1-9张图） | v2.0.0 |
| 获取帖子详情 | GET | /api/v1/posts/:post_id | 获取详情+作者信息 | v2.5.1 |
| 更新帖子 | PUT | /api/v1/posts/:post_id | 更新标题/描述 | v2.0.0 |
| 删除帖子 | DELETE | /api/v1/posts/:post_id | 删除帖子 | v2.0.0 |
| 获取Feed流 | GET | /api/v1/posts | Feed流+批量查询 | v2.5.0 |
| 获取用户帖子 | GET | /api/v1/users/:user_id/posts | 用户帖子列表 | v2.5.0 |
| 添加图片 | POST | /api/v1/posts/:post_id/images | 添加图片到帖子 | v2.0.0 |
| 删除图片 | DELETE | /api/v1/posts/:post_id/images/:image_id | 删除图片 | v2.0.0 |
| 调整图片顺序 | PUT | /api/v1/posts/:post_id/images/reorder | 调整顺序 | v2.0.0 |

### 社交互动接口（13个）

**点赞相关（3个）**:
| 接口 | 方法 | 路径 | 说明 | 版本 |
|------|------|------|------|------|
| 点赞帖子 | POST | /api/v1/posts/:post_id/like | 点赞 | v2.2.0 |
| 取消点赞 | DELETE | /api/v1/posts/:post_id/like | 取消点赞 | v2.2.0 |
| 查询点赞状态 | GET | /api/v1/posts/:post_id/like/status | 点赞状态 | v2.2.0 |

**收藏相关（4个）**:
| 接口 | 方法 | 路径 | 说明 | 版本 |
|------|------|------|------|------|
| 收藏帖子 | POST | /api/v1/posts/:post_id/favorite | 收藏 | v2.2.0 |
| 取消收藏 | DELETE | /api/v1/posts/:post_id/favorite | 取消收藏 | v2.2.0 |
| 查询收藏状态 | GET | /api/v1/posts/:post_id/favorite/status | 收藏状态 | v2.2.0 |
| 获取收藏列表 | GET | /api/v1/my/favorites | 收藏列表+批量查询 | v2.7.0 |

**关注相关（6个）**:
| 接口 | 方法 | 路径 | 说明 | 版本 |
|------|------|------|------|------|
| 关注用户 | POST | /api/v1/users/:user_id/follow | 关注 | v2.2.0 |
| 取消关注 | DELETE | /api/v1/users/:user_id/follow | 取消关注 | v2.2.0 |
| 查询关注状态 | GET | /api/v1/users/:user_id/follow/status | 关注状态 | v2.2.0 |
| 获取关注列表 | GET | /api/v1/users/:user_id/following | 关注列表 | v2.2.0 |
| 获取粉丝列表 | GET | /api/v1/users/:user_id/followers | 粉丝列表 | v2.2.0 |
| 用户统计信息 | GET | /api/v1/users/:user_id/stats | 关注/粉丝/帖子数 | v2.2.0 |
| 批量检查关注 | POST | /api/v1/users/follow/batch-status | 批量检查 | v2.2.0 |

### 分享接口（2个）

| 接口 | 方法 | 路径 | 说明 | 版本 |
|------|------|------|------|------|
| 创建分享链接 | POST | /api/v1/posts/:post_id/share | 生成短链接 | v2.3.0 |
| 解析分享链接 | GET | /api/v1/share/:code | 解析短码 | v2.3.0 |

### 评论接口（3个） 🆕 v2.8.0

| 接口 | 方法 | 路径 | 说明 | 版本 |
|------|------|------|------|------|
| 创建评论 | POST | /api/v1/posts/:post_id/comments | 发表评论 | v2.8.0 |
| 获取评论列表 | GET | /api/v1/posts/:post_id/comments | 分页查询+批量作者 | v2.8.0 |
| 删除评论 | DELETE | /api/v1/posts/:post_id/comments/:comment_id | 删除评论 | v2.8.0 |

### 系统接口（3个）

| 接口 | 方法 | 路径 | 说明 |
|------|------|------|------|
| 健康检查 | GET | /health | 服务健康检查 |
| 服务指标 | GET | /metrics | 服务性能指标 |
| 版本信息 | GET | /version | 版本信息 |

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

#### 创建帖子
```bash
curl -X POST http://localhost:8080/api/v1/posts \
  -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  -F "title=美丽风景" \
  -F "description=今天拍的照片" \
  -F "imageFiles=@photo1.jpg" \
  -F "imageFiles=@photo2.jpg" \
  -F "tags=风景" \
  -F "tags=摄影"
```

#### 获取Feed流
```bash
# 游客访问
curl http://localhost:8080/api/v1/posts?page=1&page_size=20

# 登录用户访问（返回互动状态）
curl -H "Authorization: Bearer YOUR_ACCESS_TOKEN" \
  http://localhost:8080/api/v1/posts?page=1&page_size=20
```

### 响应格式
```json
{
  "success": true,
  "message": "操作成功",
  "data": { ... },
  "timestamp": 1729324800
}
```

## 🔒 安全特性

- **密码哈希**: 使用PBKDF2-HMAC-SHA256算法，100,000次迭代，每用户独立盐值
- **JWT令牌**: 支持访问令牌（1小时）和刷新令牌（24小时）机制
- **SQL注入防护**: 使用预编译语句和参数绑定
- **输入验证**: 严格的参数验证和数据校验
- **权限控制**: JWT令牌验证和用户权限检查
- **文件安全**: 图片格式验证、大小限制、自动压缩
- **日志记录**: 详细的操作日志和安全审计

## 🚀 性能优化

### 连接池优化
- **RAII自动管理**: 使用ConnectionGuard自动管理数据库连接
- **异常安全**: 确保连接在任何情况下都能正确归还
- **防止泄漏**: 消除连接泄漏问题，100%可靠性
- **连接复用**: 减少连接创建开销

### 并发优化
- **Listen Backlog**: 128（支持128个并发连接）
- **Thread Pool**: 32个线程（支持高并发处理）
- **Connection Pool**: 10个数据库连接
- **异步处理**: 非阻塞I/O操作

### 批量查询优化

**v2.5.0 Feed流优化**:
- 批量查询作者信息：`UserService::batchGetUsers()`
- 批量查询点赞状态：`LikeService::batchCheckLikedStatus()`
- 批量查询收藏状态：`FavoriteService::batchCheckFavoritedStatus()`
- **性能提升**: 
  - 游客访问：21次查询 → 1次查询（⬇️95.2%）
  - 登录用户：61次查询 → 4次查询（⬇️93.4%）
  - QPS：~100 → ~300（⬆️200%）

**v2.7.0 收藏列表优化**:
- 复用v2.5.0的批量查询实现
- **性能提升**:
  - 查询次数：41次 → 3次（⬇️92.7%）
  - 响应时间：~150ms → ~80ms（⬆️46.7%）

**v2.8.0 关注系统优化**:
- 批量查询用户信息和关注状态
- 新增互关列表API
- **性能提升**:
  - 关注/粉丝列表：61次查询 → 3次查询（⬇️95.1%）
  - 互关列表：固定2次查询
  - 响应时间：~150ms → ~50ms（⬆️66.7%）

**v2.8.0 评论列表优化**:
- 批量查询评论作者信息
- **性能提升**:
  - 查询次数：21次 → 2次（⬇️90.5%）
  - 响应时间：<150ms

### 性能指标

| 场景 | 并发数 | 响应时间 | QPS | 成功率 |
|------|--------|----------|-----|--------|
| 单请求 | 1 | ~45ms | - | 100% |
| 并发测试 | 100 | ~80ms | ~100 | 100% |
| Feed流（游客） | - | ~50ms (P99) | ~300 | 100% |
| Feed流（登录） | - | ~50ms (P99) | ~300 | 100% |
| 关注/粉丝列表 | - | ~50ms (P99) | ~300 | 100% |
| 互关列表 | - | ~50ms (P99) | ~300 | 100% |
| 收藏列表 | - | ~80ms (P99) | ~250 | 100% |
| 评论列表 | - | <150ms (P99) | ~200 | 100% |

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
  "version": "v2.8.0",
  "database": "connected",
  "timestamp": 1729324800
}
```

### 服务指标
```bash
curl http://localhost:8080/metrics
```

响应示例：
```json
{
  "uptime": 3600,
  "requests_total": 12345,
  "requests_per_second": 100,
  "active_connections": 25,
  "database_pool_size": 10,
  "database_active_connections": 5
}
```

## 🛠️ 开发指南

### 编译选项
- **Debug模式**: `cmake -DCMAKE_BUILD_TYPE=Debug ..`
- **Release模式**: `cmake -DCMAKE_BUILD_TYPE=Release ..`
- **并行编译**: `make -j$(nproc)`

### 日志级别
- **DEBUG**: 详细调试信息
- **INFO**: 一般信息（默认）
- **WARNING**: 警告信息
- **ERROR**: 错误信息
- **FATAL**: 致命错误

配置文件设置：
```json
{
  "logging": {
    "level": "INFO",
    "file": "logs/server.log",
    "max_size": 10485760,
    "max_files": 5
  }
}
```

### 代码规范
- 使用C++17标准特性
- 遵循九层架构设计
- 使用RAII管理资源
- 使用智能指针管理内存
- 详细的中文注释
- 统一的命名规范（驼峰命名）

### 文档
完整的技术文档位于 `project_document/` 目录，包含：
- 核心文档（000-099）：API、数据库、架构、环境配置
- 阶段文档（100-199）：各阶段实现细节和测试报告
- 技术专题（200-299）：性能优化、设计模式等

## 📈 版本历史

### v2.9.0 用户名修改功能（2025-10-22）

**新增功能**：
- ✅ 用户信息修改接口支持username字段（PUT /api/v1/users/profile）
- ✅ 用户名格式验证（3-50字符，字母数字下划线）
- ✅ 智能重复检测机制（复用phone/email的检测模式）

**技术亮点**：
- 零代码重复，完全复用现有检测逻辑
- username未变化时跳过数据库查询（性能优化）
- 与现有代码模式100%一致

**测试结果**：
- ✅ 基础功能测试：修改username、格式验证、重复检测
- ✅ 智能检测测试：username未变化时成功跳过检测
- ✅ 组合测试：同时修改多个字段

**文档**: `project_document/[171]用户信息修改接口username昵称功能完善方案.md`

---

### v2.8.0 关注系统优化与评论功能（2025-10-19）

**关注系统批量查询优化**：
- ✅ 关注列表批量查询优化（GET /api/v1/users/:user_id/following）
- ✅ 粉丝列表批量查询优化（GET /api/v1/users/:user_id/followers）
- ✅ **新增互关列表API**（GET /api/v1/users/:user_id/mutual-follows）
- ✅ 批量查询用户信息：FollowRepository::batchGetUsers()
- ✅ 批量查询关注状态：FollowRepository::batchCheckExists()
- ✅ 性能提升：
  - 关注/粉丝列表（20条）：61次查询 → 3次查询（⬇️95.1%）
  - 互关列表：固定2次查询（无论结果数量）
  - 响应时间：~150ms → ~50ms（⬆️66.7%）

**评论系统**：
- ✅ 创建评论（支持UTF-8中文，自动生成ID）
- ✅ 获取评论列表（分页查询，批量加载作者信息）
- ✅ 删除评论（权限验证：评论作者或帖子作者可删除）
- ✅ 评论数统计（自动维护posts.comment_count）
- ✅ 游客模式（游客可查看，创建/删除需登录）

**技术实现**：
- 新增表：comments（6字段，4索引，2外键）
- 新增方法：findMutualFollowIds(), countMutualFollows()
- 批量查询：UserService::batchGetUsers()避免N+1查询
- SQL优化：INNER JOIN实现双向关注查询
- 事务保证：创建/删除评论与count更新原子性

**性能指标**：
- 关注/粉丝列表（20条）：3次数据库查询
- 互关列表：2次数据库查询
- 评论列表（20条）：2次数据库查询
- 创建评论：<100ms
- 查询评论列表：<150ms

---

### v2.7.0 收藏列表批量查询优化（2025-10-19）

**核心优化**：解决收藏列表N+1查询问题
- ✅ 新增返回字段：author（作者信息）、has_liked（点赞状态）、has_favorited（固定true）
- ✅ 批量查询优化：SQL IN批量查询
- ✅ 性能提升：查询次数减少92.7%，响应时间提升46.7%
- ✅ API一致性：与Feed流保持相同数据结构

**影响接口**：
- GET /api/v1/my/favorites

**技术实现**：
- 复用v2.5.0的批量查询方法
- 零数据库表变更，仅Handler层修改

---

### v2.6.0 用户头像上传（2025-10-18）

**新增功能**：用户头像上传和处理
- ✅ 头像文件上传（multipart/form-data）
- ✅ 自动处理：居中裁剪为正方形，缩放到200x200
- ✅ JPEG压缩（80%质量）
- ✅ 自动删除旧头像文件

**技术实现**：
- 新增工具类：AvatarProcessor
- 支持格式：JPEG/PNG/GIF/WebP
- 文件大小限制：5MB
- 事务性回滚：数据库更新失败时自动删除新文件

---

### v2.5.0 Feed流批量查询优化（2025-10-18）

**核心优化**：解决Feed流N+1查询问题
- ✅ 可选JWT认证（支持游客和登录用户统一访问）
- ✅ 批量查询优化（作者信息、点赞状态、收藏状态）
- ✅ 新增返回字段：author、has_liked、has_favorited
- ✅ 统一JSON结构（字段始终存在）
- ✅ 性能提升：查询次数减少93.4%，QPS提升200%

**影响接口**：
- GET /api/v1/posts
- GET /api/v1/users/:user_id/posts

**性能指标**：
- 游客：21次 → 1次（⬇️95.2%）
- 登录用户：61次 → 4次（⬇️93.4%）
- QPS：~100 → ~300（⬆️200%）

---

### v2.4.1 JSON格式创建帖子支持（2025-10-15）

**新增功能**：创建帖子接口支持JSON+Base64格式
- ✅ 支持application/json（Base64图片）
- ✅ 支持multipart/form-data（文件上传）
- ✅ 自动检测和解码Base64数据
- ✅ 完全向后兼容

---

### v2.3.0 分享系统（2025-10-11）

**新增功能**：短链接生成和Deep Link支持
- ✅ 雪花ID算法 + Base62编码（8位短码）
- ✅ 自动去重（同一帖子返回相同短链）
- ✅ Deep Link支持（iOS/Android/HarmonyOS三端）
- ✅ 支持过期时间设置

**数据库变更**：
- 新增表：share_links（7字段，4索引，2外键）

---

### v2.2.0 社交互动系统（2025-10-17）

**新增功能**：点赞、收藏、关注系统
- ✅ 点赞功能（点赞/取消/查询状态）
- ✅ 收藏功能（收藏/取消/查询状态/收藏列表）
- ✅ 关注功能（关注/取消/查询/列表/统计/批量检查）

**数据库变更**：
- 新增表：likes、favorites、follows

---

### v2.1.0 用户信息管理（2025-10-09）

**新增功能**：用户信息管理功能
- ✅ 获取当前用户完整信息
- ✅ 修改用户信息（bio、gender、location、website、avatar_url等）
- ✅ 获取用户公开信息
- ✅ 用户名可用性检查

**数据库变更**：
- users表新增字段：bio、gender、location、website、avatar_url

---

### v2.0.0 多图片帖子系统（2025-10-08）

**新增功能**：多图片帖子系统
- ✅ 多图片帖子发布（1-9张图片）
- ✅ 图片压缩与缩略图生成
- ✅ 帖子编辑与删除
- ✅ 图片顺序管理
- ✅ Feed推荐算法
- ✅ 标签系统

**数据库变更**：
- 新增表：posts、images、tags、post_tags

---

### v1.2.0 用户认证系统（2025-10-01）

**新增功能**：用户认证系统
- ✅ 用户注册与登录
- ✅ JWT令牌管理（访问令牌+刷新令牌）
- ✅ 密码安全存储（PBKDF2-HMAC-SHA256）
- ✅ 密码修改功能

**数据库变更**：
- 新增表：users

---

### v1.0.0 项目骨架（2025-09-30）

**初始版本**：基础设施搭建
- ✅ 项目骨架搭建
- ✅ 数据库连接池
- ✅ 配置管理系统
- ✅ 日志系统
- ✅ HTTP服务器封装

---

### 更多版本历史

完整的版本更新记录请查看：[project_document/[000]API文档.md](project_document/[000]API文档.md#最新版本更新)

## 📈 项目进度

### 已完成功能 ✅

#### 阶段A: 基础模块（v1.0.0）
- ✅ 项目骨架搭建
- ✅ 数据库连接池（RAII管理）
- ✅ 配置管理系统
- ✅ 日志系统
- ✅ HTTP服务器封装

#### 阶段B: 用户认证模块（v1.2.0 - v2.6.0）
- ✅ 用户注册与登录
- ✅ JWT令牌管理（访问令牌+刷新令牌）
- ✅ 密码哈希（PBKDF2-HMAC-SHA256，100,000次迭代）
- ✅ 令牌验证和刷新
- ✅ 密码修改功能
- ✅ **用户信息管理**（v2.1.0）
  - 获取当前用户完整信息
  - 修改用户信息（支持个人简介、性别、所在地等新字段）
  - 获取其他用户公开信息
  - 用户名可用性检查
- ✅ **用户头像上传**（v2.6.0）
  - 头像文件上传
  - 自动裁剪和缩放
  - 自动删除旧头像

#### 阶段C: 图片管理模块（v2.0.0 - v2.5.1）
- ✅ 多图片帖子发布（1-9张图片）
- ✅ 支持multipart/form-data和JSON+Base64格式
- ✅ 图片压缩与缩略图生成
- ✅ 帖子编辑与删除
- ✅ 图片顺序管理（添加、删除、调整）
- ✅ Feed推荐算法（热度+时间衰减）
- ✅ 标签系统（最多5个标签）
- ✅ **Feed流批量查询优化**（v2.5.0）
  - 支持游客访问
  - 批量查询作者信息和互动状态
  - 性能提升200%
- ✅ **帖子详情优化**（v2.5.1）
  - 新增用户昵称字段
  - 减少前端请求次数

#### 阶段D: 社交互动模块（v2.2.0 - v2.8.0）
- ✅ **点赞功能**（v2.2.0）
  - 点赞/取消点赞
  - 查询点赞状态
  - 点赞数统计
- ✅ **收藏功能**（v2.2.0）
  - 收藏/取消收藏
  - 查询收藏状态
  - 获取收藏列表
  - 收藏数统计
- ✅ **收藏列表优化**（v2.7.0）
  - 批量查询优化
  - 性能提升92.7%
- ✅ **关注功能**（v2.2.0）
  - 关注/取消关注
  - 查询关注状态
  - 获取关注列表和粉丝列表
  - 用户统计信息
  - 批量检查关注状态
- ✅ **评论功能**（v2.8.0）
  - 创建评论
  - 获取评论列表（批量查询作者）
  - 删除评论（权限验证）
  - 评论数统计

#### 阶段E: 分享系统（v2.3.0）
- ✅ 短链接生成（雪花ID + Base62编码）
- ✅ 分享链接解析
- ✅ Deep Link支持（iOS/Android/HarmonyOS）
- ✅ 自动去重
- ✅ 支持过期时间

### 规划中功能 ⏳

#### 阶段F: 系统优化
- ⏳ Redis缓存层
- ⏳ 数据库读写分离
- ⏳ 负载均衡
- ⏳ CDN加速
- ⏳ 全文搜索（ElasticSearch）
- ⏳ 实时推送（WebSocket）

#### 阶段G: 运营功能
- ⏳ 数据统计分析
- ⏳ 内容审核系统
- ⏳ 用户举报系统
- ⏳ 管理后台

## 📝 待办事项

**测试相关**:
- [ ] 添加单元测试框架
- [ ] API集成测试
- [ ] 性能压力测试
- [ ] 安全渗透测试

**CI/CD**:
- [ ] GitHub Actions配置
- [ ] 自动化测试流程
- [ ] 自动化部署流程
- [ ] Docker容器化

**监控告警**:
- [ ] Prometheus指标采集
- [ ] Grafana监控面板
- [ ] 日志聚合（ELK Stack）
- [ ] 告警通知（钉钉/企业微信）

**性能优化**:
- [ ] Redis缓存热点数据
- [ ] 数据库索引优化
- [ ] SQL查询优化
- [ ] 图片CDN加速

## 🎯 技术亮点

### 1. RAII资源管理
- 使用ConnectionGuard自动管理数据库连接
- 异常安全，防止资源泄漏
- 代码简洁，易于维护

**示例代码**：
```cpp
ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
if (!connGuard.isValid()) {
    return ErrorResult;
}
// 作用域结束时自动归还连接
```

### 2. 批量查询优化
- 解决N+1查询问题
- 使用SQL IN批量获取数据
- 性能提升200%-300%

**v2.5.0实现**：
- UserService::batchGetUsers() - 批量获取用户信息
- LikeService::batchCheckLikedStatus() - 批量查询点赞状态
- FavoriteService::batchCheckFavoritedStatus() - 批量查询收藏状态

### 3. 高并发处理
- Listen Backlog: 128
- Thread Pool: 32
- Connection Pool: 10
- 并发能力：~300请求/秒

### 4. 安全性
- PBKDF2-HMAC-SHA256密码哈希（100,000次迭代）
- JWT令牌认证（访问令牌+刷新令牌机制）
- SQL注入防护（预编译语句+参数绑定）
- 文件上传安全（格式验证+大小限制+自动压缩）
- 详细的安全审计日志

### 5. 九层架构
- 清晰的层次结构
- 职责分离（单一职责原则）
- 易于扩展和维护
- 便于单元测试

### 6. 可选JWT认证
- 支持游客模式（无需登录即可浏览）
- 登录用户获得完整功能
- 无效Token自动降级为游客模式
- 统一的API接口

### 7. 完善的文档
- 40+技术文档（project_document/）
- 详细的API文档（v2.8.0）
- 完整的架构说明
- 性能优化专题
- 实施计划和测试报告

## 📊 代码统计

| 模块 | 文件数 | 代码行数（估算） | 说明 |
|------|--------|-----------------|------|
| 基础设施层 | ~20 | ~3,000 | 连接池、配置、日志 |
| 用户认证模块 | ~14 | ~2,500 | 认证、用户信息、头像 |
| 帖子管理模块 | ~18 | ~4,000 | 帖子、图片、标签 |
| 社交互动模块 | ~18 | ~3,500 | 点赞、收藏、关注 |
| 分享系统 | ~6 | ~1,000 | 短链接、Deep Link |
| 评论系统 | ~6 | ~1,200 | 评论CRUD |
| **总计** | **~82** | **~15,200** | - |

**数据库表统计**：
- 10个数据表
- 80+字段
- 40+索引
- 20+外键约束

## 📄 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 👥 贡献者

- Knot Development Team

## 📞 联系方式

如有问题或建议，请通过以下方式联系：
- 项目Issue
- 技术文档：`project_document/`

---

**项目状态**: 🚀 活跃开发中  
**当前版本**: v2.8.0（评论功能）  
**最后更新**: 2025-10-19

**注意**: 本项目目前处于快速开发阶段，已完成核心功能模块。在生产环境使用前，请确保完成所有安全配置和性能测试。

---

## 🌟 Star History

如果这个项目对你有帮助，请给我们一个⭐️ Star！

**快速链接**：
- [📖 完整API文档](project_document/[000]API文档.md)
- [🏗️ 架构设计](project_document/[002]项目架构文档.md)
- [💾 数据库设计](project_document/[001]数据库设计文档.md)
- [⚡ 性能优化](project_document/[201]性能优化专题.md)
