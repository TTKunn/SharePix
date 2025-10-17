# Knot 静态编译部署目录

## � 目录

- [目录说明](#目录说明)
- [目录结构](#目录结构)
- [快速开始](#快速开始)
- [部署包内容](#部署包内容)
- [静态编译特性](#静态编译特性)
- [与开发构建的区别](#与开发构建的区别)
- [使用场景](#使用场景)
- [注意事项](#注意事项)
- [故障排查](#故障排查)
- [性能对比](#性能对比)
- [服务器部署教程](#服务器部署教程)
  - [前置条件](#前置条件)
  - [部署步骤](#部署步骤)
  - [停止并删除旧服务](#停止并删除旧服务)
  - [后台运行服务](#后台运行服务)
  - [快速命令](#快速命令)
  - [验证部署](#验证部署)
- [相关文档](#相关文档)
- [技术支持](#技术支持)

## �📋 目录说明

本目录专门用于生成静态编译的可执行文件，用于生产环境部署。

**重要**: 
- ✅ 本目录仅用于**生产部署**
- ✅ 日常开发请使用上层目录的 `build/`
- ✅ 两个构建系统完全独立，互不影响

## 🎯 目录结构

```
deploy-static/
├── CMakeLists.txt           # 静态编译专用配置
├── build.sh                 # 自动化编译脚本
├── build/                   # 编译输出目录（自动生成）
│   ├── knot_image_sharing   # 静态编译的可执行文件
│   └── knot-deploy-YYYYMMDD-HHMMSS.tar.gz  # 部署包
└── README.md                # 本文件
```

## 🚀 快速开始

### 1. 安装依赖（仅首次需要）

```bash
sudo apt install -y \
    build-essential \
    cmake \
    libmysqlclient-dev \
    libjsoncpp-dev \
    libssl-dev
```

### 2. 编译

```bash
cd backend-service/deploy-static
./build.sh
```

### 3. 获取部署包

编译成功后，在 `build/` 目录下会生成：
- `knot_image_sharing` - 静态编译的可执行文件
- `knot-deploy-YYYYMMDD-HHMMSS.tar.gz` - 完整的部署包

## 📦 部署包内容

```
knot-deploy-YYYYMMDD-HHMMSS/
├── knot_image_sharing              # 可执行文件
├── config/
│   └── config.production.json      # 生产配置模板
├── docs/
│   └── api/                        # API文档
│       ├── openapi.yaml
│       ├── api-docs.html
│       └── start-api-docs.sh
├── start.sh                        # 启动脚本
└── README.txt                      # 部署说明
```

## 🔧 静态编译特性

### 优势
- ✅ **无依赖部署**: 服务器无需安装任何第三方库
- ✅ **跨发行版**: 可在任何Linux发行版运行
- ✅ **版本独立**: 不受系统库版本影响
- ✅ **部署简单**: 只需上传一个文件

### 技术细节

**静态链接的库**:
- libstdc++ (C++标准库)
- libgcc_s (GCC运行时)
- libjsoncpp (JSON解析)
- libssl/libcrypto (OpenSSL)
- libmysqlclient (MySQL客户端)

**仅依赖系统库**（所有Linux都有）:
- libc.so.6 (C标准库)
- libm.so.6 (数学库)
- libpthread.so.0 (线程库)
- libdl.so.2 (动态加载)
- librt.so.1 (实时扩展)
- libresolv.so.2 (DNS解析)

## 🔍 与开发构建的区别

| 特性 | 开发构建 (../build/) | 静态构建 (deploy-static/build/) |
|------|---------------------|--------------------------------|
| 用途 | 日常开发和调试 | 生产环境部署 |
| 链接方式 | 动态链接 | 静态链接 |
| 文件大小 | 较小（~5MB） | 较大（~20MB） |
| 依赖库 | 需要系统库 | 仅需系统基础库 |
| 编译速度 | 快 | 较慢 |
| 调试信息 | 包含 | 已strip |
| Anaconda | 可能冲突 | 完全避免 |

## 📝 使用场景

### 开发阶段
使用上层目录的构建系统：
```bash
cd backend-service
mkdir build && cd build
cmake ..
make -j4
./knot_image_sharing ../config/config.json
```

### 部署阶段
使用本目录的静态编译：
```bash
cd backend-service/deploy-static
./build.sh
# 上传 build/knot-deploy-*.tar.gz 到服务器
```

## ⚠️ 注意事项

### 1. Anaconda环境
编译脚本会自动禁用Anaconda环境，避免库冲突。

### 2. 静态库要求
必须安装开发包（-dev）才能获得静态库：
- libmysqlclient-dev
- libjsoncpp-dev
- libssl-dev

### 3. 编译时间
静态编译比动态编译慢，因为需要链接更多代码。

### 4. 文件大小
静态编译的可执行文件较大（15-25MB），这是正常的。

## 🐛 故障排查

### 问题1: 找不到静态库

**错误信息**:
```
MySQL static library not found in /usr/lib/x86_64-linux-gnu
```

**解决方案**:
```bash
sudo apt install -y libmysqlclient-dev libjsoncpp-dev libssl-dev
```

### 问题2: Anaconda库冲突

**错误信息**:
```
GLIBCXX_3.4.30 not found
```

**解决方案**:
编译脚本已自动处理。如果仍有问题，手动禁用Anaconda：
```bash
export PATH=/usr/bin:/bin:/usr/sbin:/sbin
unset LD_LIBRARY_PATH
./build.sh
```

### 问题3: 编译失败

**检查步骤**:
1. 确认安装了所有依赖
2. 确认CMake版本 >= 3.16
3. 确认GCC版本 >= 9.0
4. 查看详细错误信息

## 📊 性能对比

| 指标 | 动态链接 | 静态链接 |
|------|---------|---------|
| 启动时间 | 快 | 稍慢 |
| 运行性能 | 相同 | 相同 |
| 内存占用 | 稍低 | 稍高 |
| 部署便利性 | 低 | 高 |

## � 服务器部署教程

### 前置条件

服务器需要安装3个运行时库：
```bash
sudo apt update
sudo apt install -y libjsoncpp25 libzstd1 libfmt8
```

### 部署步骤

#### 1. 上传部署包

```bash
# 在本地机器上
cd /home/kun/projects/Knot/backend-service/deploy-static/build
scp knot-deploy-*.tar.gz user@your-server:/opt/
```

#### 2. 解压部署包

```bash
# 在服务器上
cd /opt
tar -xzf knot-deploy-*.tar.gz
cd knot-deploy-*/
```

#### 3. 修改配置

```bash
vim config/config.production.json
```

**必须修改的配置项**:
```json
{
  "database": {
    "host": "your-db-host",
    "port": 3306,
    "user": "your-db-user",
    "password": "your-db-password",
    "database": "knot_image_sharing"
  },
  "jwt": {
    "secret": "your-production-secret-key-change-this"
  },
  "server": {
    "host": "0.0.0.0",
    "port": 8080
  }
}
```

#### 4. 停止并删除旧服务

如果服务器上已有旧版本运行，需要先停止并清理：

```bash
# 停止后端API服务
pkill knot_image_sharing

# 停止API文档服务
pkill -f "http.server 8081"

# 验证进程已停止
ps aux | grep knot_image_sharing
ps aux | grep "http.server 8081"

# 删除旧的部署目录（可选）
cd /opt
rm -rf knot-deploy-old-version/
```

#### 5. 测试运行

```bash
# 前台运行测试
./start.sh

# 如果正常启动，按Ctrl+C停止
```

### 后台运行服务

#### 方法1：使用 nohup（简单快速）

**后端API服务**：
```bash
cd /opt/knot-deploy-*/
nohup ./start.sh > knot.log 2>&1 &

# 查看进程
ps aux | grep knot_image_sharing

# 查看日志
tail -f knot.log
```

**API文档服务**：
```bash
cd /opt/knot-deploy-*/docs/api
nohup ./start-api-docs.sh > api-docs.log 2>&1 &

# 查看进程
ps aux | grep "http.server 8081"

# 查看日志
tail -f api-docs.log
```

#### 方法2：使用 systemd（推荐，开机自启）

**后端API服务**：
```bash
sudo tee /etc/systemd/system/knot.service > /dev/null <<EOF
[Unit]
Description=Knot Image Sharing Service
After=network.target

[Service]
Type=simple
User=root
WorkingDirectory=/opt/knot-deploy-YYYYMMDD-HHMMSS
ExecStart=/opt/knot-deploy-YYYYMMDD-HHMMSS/knot_image_sharing config/config.production.json
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable knot
sudo systemctl start knot
```

**API文档服务**：
```bash
sudo tee /etc/systemd/system/knot-api-docs.service > /dev/null <<EOF
[Unit]
Description=Knot API Documentation Server
After=network.target

[Service]
Type=simple
User=root
WorkingDirectory=/opt/knot-deploy-YYYYMMDD-HHMMSS/docs/api
ExecStart=/usr/bin/python3 -m http.server 8081
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable knot-api-docs
sudo systemctl start knot-api-docs
```

### 配置防火墙

```bash
# Ubuntu/Debian
sudo ufw allow 8080/tcp
sudo ufw allow 8081/tcp
sudo ufw reload

# CentOS/RHEL
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --permanent --add-port=8081/tcp
sudo firewall-cmd --reload
```

### 快速命令

#### 使用 nohup 管理

```bash
# 启动后端服务
cd /opt/knot-deploy-*/ && nohup ./start.sh > knot.log 2>&1 &

# 启动文档服务
cd /opt/knot-deploy-*/docs/api && nohup ./start-api-docs.sh > api-docs.log 2>&1 &

# 查看进程
ps aux | grep knot_image_sharing
ps aux | grep "http.server 8081"

# 停止服务
pkill knot_image_sharing
pkill -f "http.server 8081"

# 查看日志
tail -f /opt/knot-deploy-*/knot.log
tail -f /opt/knot-deploy-*/docs/api/api-docs.log
```

#### 使用 systemd 管理

```bash
# 启动服务
sudo systemctl start knot
sudo systemctl start knot-api-docs

# 停止服务
sudo systemctl stop knot
sudo systemctl stop knot-api-docs

# 重启服务
sudo systemctl restart knot
sudo systemctl restart knot-api-docs

# 查看状态
sudo systemctl status knot
sudo systemctl status knot-api-docs

# 查看日志
sudo journalctl -u knot -f
sudo journalctl -u knot-api-docs -f
```

### 验证部署

```bash
# 测试后端API
curl http://localhost:8080/api/v1/images

# 测试API文档
curl http://localhost:8081/api-docs.html

# 从外部访问（替换为您的服务器IP）
curl http://43.142.157.145:8080/api/v1/images
curl http://8.138.115.164:8081/api-docs.html
```

## �🔗 相关文档

- [项目主README](../README.md)
- [API文档](../docs/api/README.md)
- [网络访问配置](../docs/NETWORK-ACCESS.md)
- [项目架构文档](../project_document/[001]项目架构文档.md)

## 📞 技术支持

如有问题，请查看：
- 项目文档目录: `backend-service/project_document/`
- 在线API文档: http://localhost:8081/api-docs.html

---

**最后更新**: 2025-10-08
**版本**: v1.0.0
**维护者**: Knot Team

