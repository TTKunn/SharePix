# Knot 项目文档中心

## 📖 简介

本目录包含Knot图片分享系统的所有文档资源，包括API文档、部署文档等。

## 📁 目录结构

```
docs/
├── README.md                    # 本文件 - 文档中心导航
├── NETWORK-ACCESS.md            # 局域网访问配置指南
├── check-network-access.sh      # 网络访问检查脚本
└── api/                         # API文档目录
    ├── README.md                # API文档使用指南
    ├── openapi.yaml             # OpenAPI 3.0规范文档
    ├── api-docs.html            # Swagger UI展示页面
    ├── start-api-docs.sh        # API文档服务器启动脚本
    └── docker-compose-swagger.yml # Docker部署配置
```

## 🚀 快速开始

### 启动API在线文档（推荐）

```bash
# 进入API文档目录
cd /home/kun/projects/Knot/backend-service/docs/api

# 启动文档服务器
./start-api-docs.sh
```

然后访问: http://localhost:8081/api-docs.html

### 查看详细使用说明

请参阅: [API文档使用指南](./api/README.md)

## 📚 文档分类

### 1. API文档 (`api/`)

**在线交互式文档**
- 基于OpenAPI 3.0规范
- 使用Swagger UI可视化展示
- 支持在线测试API
- 包含完整的请求/响应示例

**访问方式**:
- 启动服务: `cd api && ./start-api-docs.sh`
- 浏览器访问: http://localhost:8081/api-docs.html

**包含内容**:
- 认证API（6个接口）
- 图片管理API（6个接口）
- 系统API（3个接口）

### 2. 网络访问配置 (`NETWORK-ACCESS.md`)

**局域网访问指南**
- 虚拟机网络配置
- 从Windows宿主机访问服务
- 防火墙配置
- 常见问题解决

**快速检查**: 运行 `./check-network-access.sh` 自动检查网络配置

### 3. 项目文档 (`../project_document/`)

**开发文档**（位于backend-service/project_document/）
- [000] API文档（Markdown格式）
- [001] 项目架构文档
- [002] 环境安装配置教程
- [100] 阶段A-基础模块实现
- [101] 阶段B-用户认证模块
- [102] 阶段C-图片管理模块
- [103] 阶段C-1-图片管理模块第一次测试情况
- [200] 数据库设计文档
- [201] 性能优化专题

## 🌐 访问地址

启动API文档服务器后，可以通过以下地址访问：

- **本地访问**: http://localhost:8081/api-docs.html
- **局域网访问**: http://[你的IP]:8081/api-docs.html

## 📚 使用指南

### 1. 浏览 API 接口

打开文档页面后，您可以看到所有可用的 API 接口，按照以下分类：

- **认证** - 用户注册、登录、令牌管理等
- **图片管理** - 图片上传、查询、编辑、删除
- **系统** - 健康检查和系统指标

### 2. 测试 API 接口

#### 步骤 1: 登录获取令牌
1. 找到 `POST /auth/login` 接口
2. 点击 "Try it out" 按钮
3. 填写用户名和密码（测试用户: testuser / Test123456）
4. 点击 "Execute" 执行请求
5. 从响应中复制 `access_token`

#### 步骤 2: 配置认证令牌
1. 点击页面右上角的 "Authorize" 按钮
2. 在弹出的对话框中输入: `Bearer {your_access_token}`
3. 点击 "Authorize" 确认
4. 点击 "Close" 关闭对话框

#### 步骤 3: 测试需要认证的接口
现在您可以测试所有需要认证的接口，例如：
- `POST /images` - 上传图片
- `PUT /images/{id}` - 更新图文配文
- `DELETE /images/{id}` - 删除图片
- `POST /auth/change-password` - 修改密码

### 3. 查看接口详情

每个接口都包含以下信息：
- **请求方法** - GET、POST、PUT、DELETE 等
- **请求路径** - API 端点路径
- **请求参数** - 路径参数、查询参数、请求体参数
- **响应示例** - 成功和失败的响应示例
- **响应状态码** - 200、400、401、404 等

## 🔧 配置说明

### 修改服务器地址

如果需要修改 API 服务器地址，编辑 `openapi.yaml` 文件中的 `servers` 部分：

```yaml
servers:
  - url: http://localhost:8080/api/v1
    description: 本地开发环境
  - url: http://your-server-ip:8080/api/v1
    description: 测试环境
```

### 修改文档端口

编辑 `serve.sh` 文件，修改 `PORT` 变量：

```bash
PORT=3000  # 修改为您想要的端口号
```

## 📝 API 接口列表

### 认证接口（6个）

| 接口 | 方法 | 路径 | 说明 |
|------|------|------|------|
| 用户注册 | POST | /auth/register | 注册新用户 |
| 用户登录 | POST | /auth/login | 用户登录 |
| 令牌验证 | POST | /auth/validate | 验证JWT令牌 |
| 令牌刷新 | POST | /auth/refresh | 刷新访问令牌 |
| 用户登出 | POST | /auth/logout | 用户登出 |
| 修改密码 | POST | /auth/change-password | 修改用户密码 |

### 图片管理接口（6个）

| 接口 | 方法 | 路径 | 说明 |
|------|------|------|------|
| 上传图片 | POST | /images | 上传图文推文 |
| 获取最新图片列表 | GET | /images | 获取最新图片列表（分页） |
| 获取图片详情 | GET | /images/{id} | 获取图片详情（增加浏览数） |
| 更新图文配文 | PUT | /images/{id} | 更新标题和配文 |
| 删除图片 | DELETE | /images/{id} | 删除图片 |
| 获取用户图片列表 | GET | /users/{id}/images | 获取指定用户的图片列表 |

### 系统接口（3个）

| 接口 | 方法 | 路径 | 说明 |
|------|------|------|------|
| 健康检查 | GET | /health | 服务健康检查 |
| 系统指标 | GET | /metrics | 服务性能指标 |
| 版本信息 | GET | /version | API版本信息 |

## 🔒 认证说明

### JWT 令牌

系统使用 JWT（JSON Web Token）进行身份认证：

- **访问令牌（Access Token）**: 有效期 1 小时，用于访问受保护的 API
- **刷新令牌（Refresh Token）**: 有效期 24 小时，用于获取新的访问令牌

### 使用方式

在请求头中添加 Authorization 字段：

```
Authorization: Bearer {access_token}
```

### 令牌刷新

当访问令牌过期时，使用刷新令牌获取新的访问令牌：

```bash
curl -X POST http://localhost:8080/api/v1/auth/refresh \
  -H "Content-Type: application/json" \
  -d '{"refresh_token": "your_refresh_token"}'
```

## 🛠️ 开发说明

### 更新 API 文档

1. 编辑 `openapi.yaml` 文件
2. 刷新浏览器页面查看更新

### 验证 OpenAPI 规范

使用在线工具验证 OpenAPI 规范：
- https://editor.swagger.io/
- 将 `openapi.yaml` 内容粘贴到编辑器中

### 导出 API 文档

Swagger UI 支持导出为多种格式：
- JSON
- YAML
- Postman Collection

## 🔧 维护指南

### 更新API文档

当API发生变化时：

1. 编辑 `api/openapi.yaml` 文件
2. 刷新浏览器页面查看更新
3. 同步更新 `../project_document/[000]API文档.md`

### 添加新文档

1. 确定文档类型（API、开发、设计等）
2. 放入对应目录
3. 更新本README文件

## 📞 技术支持

如有问题或建议，请查看：
- 项目主 README: `/home/kun/projects/Knot/backend-service/README.md`
- 项目文档目录: `/home/kun/projects/Knot/backend-service/project_document/`
- API文档详细说明: `./api/README.md`

## 📄 许可证

本项目采用 MIT 许可证。

---

**最后更新**: 2025-10-08
**版本**: v1.2.0
**维护者**: Knot Team

