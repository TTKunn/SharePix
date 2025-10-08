# Knot API 在线文档使用指南

## 📚 概述

本项目提供了基于OpenAPI 3.0规范的在线API文档，使用Swagger UI进行可视化展示。

## 🚀 快速开始

### 方法一：使用启动脚本（推荐）

```bash
cd /home/kun/projects/Knot/backend-service
./start-api-docs.sh
```

然后在浏览器中访问：
- **本地访问**: http://localhost:8081/api-docs.html
- **局域网访问**: http://[你的IP]:8081/api-docs.html

### 方法二：手动启动

```bash
cd /home/kun/projects/Knot/backend-service
python3 -m http.server 8081
```

然后访问: http://localhost:8081/api-docs.html

### 方法三：使用Docker（如果已安装Docker）

```bash
cd /home/kun/projects/Knot/backend-service
docker-compose -f docker-compose-swagger.yml up -d
```

然后访问: http://localhost:8081

## 📖 文档文件说明

### 核心文件

1. **openapi.yaml** - OpenAPI 3.0规范文档
   - 包含所有API端点定义
   - 包含请求/响应模型
   - 包含认证方式说明

2. **api-docs.html** - Swagger UI展示页面
   - 基于Swagger UI 5.10.0
   - 支持在线测试API
   - 支持搜索和过滤

3. **start-api-docs.sh** - 启动脚本
   - 自动启动HTTP服务器
   - 默认端口8081

4. **docker-compose-swagger.yml** - Docker配置（可选）
   - 使用官方Swagger UI镜像
   - 端口映射8081:8080

## 🎯 功能特性

### Swagger UI功能

- ✅ **交互式文档**: 可视化展示所有API端点
- ✅ **在线测试**: 直接在浏览器中测试API
- ✅ **模型展示**: 查看请求和响应的数据模型
- ✅ **认证支持**: 支持JWT Bearer Token认证
- ✅ **搜索过滤**: 快速查找特定API
- ✅ **代码示例**: 自动生成多种语言的调用示例

### API分类

1. **认证相关** (6个接口)
   - 用户注册
   - 用户登录
   - 令牌验证
   - 令牌刷新
   - 用户登出
   - 修改密码

2. **图片管理** (6个接口)
   - 上传图片
   - 获取最新图片列表
   - 获取图片详情
   - 更新图文配文
   - 删除图片
   - 获取用户图片列表

3. **系统相关** (3个接口)
   - 健康检查
   - 系统指标
   - 版本信息

## 🔧 使用说明

### 1. 查看API文档

访问 http://localhost:8081/api-docs.html 后，您将看到：
- 左侧：API分类和端点列表
- 右侧：详细的API说明、参数、响应示例

### 2. 测试API

#### 步骤1：获取JWT令牌

1. 找到 `POST /auth/login` 接口
2. 点击 "Try it out" 按钮
3. 填写请求参数：
   ```json
   {
     "username": "testuser",
     "password": "Test123456"
   }
   ```
4. 点击 "Execute" 执行
5. 从响应中复制 `access_token`

#### 步骤2：设置认证

1. 点击页面右上角的 "Authorize" 按钮
2. 在弹出框中输入：`Bearer {你的access_token}`
3. 点击 "Authorize" 确认
4. 点击 "Close" 关闭

#### 步骤3：测试需要认证的API

现在您可以测试任何需要认证的API，例如：
- POST /images (上传图片)
- PUT /images/{id} (更新配文)
- DELETE /images/{id} (删除图片)

### 3. 查看数据模型

在文档底部的 "Schemas" 部分，您可以查看所有数据模型的定义：
- User - 用户模型
- Image - 图片模型
- LoginRequest - 登录请求
- LoginResponse - 登录响应
- 等等...

## 📝 注意事项

### 端口冲突

如果8081端口已被占用，可以修改端口：

**修改启动脚本**:
```bash
# 编辑 start-api-docs.sh
PORT=8082  # 改为其他端口
```

**或手动指定端口**:
```bash
python3 -m http.server 8082
```

### CORS问题

如果在浏览器中测试API时遇到CORS错误，请确保：
1. 后端服务器已启动（localhost:8080）
2. 后端已正确配置CORS头（已在http_server.cpp中配置）

### 文件上传测试

测试图片上传API时：
1. 选择 "Try it out"
2. 在 "image" 字段点击 "Choose File" 选择图片
3. 填写其他表单字段（title、description、tags）
4. 点击 "Execute"

## 🔄 更新文档

当API发生变化时，需要更新OpenAPI文档：

1. 编辑 `openapi.yaml` 文件
2. 刷新浏览器页面即可看到更新

## 🛑 停止服务

### 停止Python HTTP服务器

在运行 `start-api-docs.sh` 的终端中按 `Ctrl+C`

### 停止Docker容器

```bash
docker-compose -f docker-compose-swagger.yml down
```

## 📚 相关资源

- [OpenAPI 3.0规范](https://swagger.io/specification/)
- [Swagger UI文档](https://swagger.io/tools/swagger-ui/)
- [Knot项目文档](./project_document/)

## 🐛 常见问题

### Q: 页面显示空白？
A: 检查浏览器控制台是否有错误，确保openapi.yaml文件格式正确。

### Q: 无法加载openapi.yaml？
A: 确保HTTP服务器在backend-service目录下启动，openapi.yaml在同一目录。

### Q: API测试返回401错误？
A: 检查JWT令牌是否正确设置，是否已过期（有效期1小时）。

### Q: 图片上传失败？
A: 检查图片大小（最大5MB）、格式（JPEG/PNG/WebP）、后端服务器是否运行。

## 📞 技术支持

如有问题，请查看：
- 项目文档: `./project_document/[000]API文档.md`
- 测试报告: `./project_document/[103]阶段C-1-图片管理模块第一次测试情况.md`

---

**最后更新**: 2025-10-08
**维护者**: Knot Team

