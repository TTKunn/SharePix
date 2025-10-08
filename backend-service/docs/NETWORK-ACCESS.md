# 🌐 局域网访问配置指南

## 📋 概述

本文档说明如何从局域网内的其他设备（如Windows宿主机）访问虚拟机中运行的Knot服务。

## 🔍 当前网络配置

### 虚拟机信息
- **IP地址**: `192.168.32.128`
- **网络接口**: `ens33`
- **网络模式**: 桥接模式（Bridged）或NAT模式
- **子网**: `192.168.32.0/24`

### 服务端口
- **后端API服务**: `8080`
- **API文档服务**: `8081`

## 🚀 快速访问

### 从Windows宿主机访问

#### 1. 访问API文档
```
http://192.168.32.128:8081/api-docs.html
```

#### 2. 访问后端API
```
http://192.168.32.128:8080/api/v1/
```

#### 3. 测试连接
在Windows命令提示符或PowerShell中：
```cmd
ping 192.168.32.128
```

如果ping通，说明网络连接正常。

## 🔧 配置步骤

### 步骤1：确认虚拟机网络模式

#### VMware Workstation
1. 右键虚拟机 → 设置
2. 选择"网络适配器"
3. 确认网络连接模式：
   - **桥接模式（推荐）**: 虚拟机获得与宿主机同网段的IP
   - **NAT模式**: 需要配置端口转发

#### VirtualBox
1. 设置 → 网络
2. 网卡1 → 连接方式
3. 选择"桥接网卡"

### 步骤2：检查服务是否运行

在虚拟机中执行：
```bash
# 检查后端API服务
netstat -tlnp | grep 8080

# 检查API文档服务
netstat -tlnp | grep 8081
```

应该看到类似输出：
```
tcp        0      0 0.0.0.0:8080            0.0.0.0:*               LISTEN      12345/knot_image_sh
tcp        0      0 0.0.0.0:8081            0.0.0.0:*               LISTEN      12346/python3
```

**重要**: 确保服务监听在 `0.0.0.0` 而不是 `127.0.0.1`

### 步骤3：检查防火墙（如果需要）

#### 检查UFW状态
```bash
sudo ufw status
```

#### 如果UFW激活，开放端口
```bash
# 开放后端API端口
sudo ufw allow 8080/tcp

# 开放API文档端口
sudo ufw allow 8081/tcp

# 重新加载防火墙
sudo ufw reload
```

#### 检查firewalld（CentOS/RHEL）
```bash
# 检查状态
sudo firewall-cmd --state

# 开放端口
sudo firewall-cmd --permanent --add-port=8080/tcp
sudo firewall-cmd --permanent --add-port=8081/tcp
sudo firewall-cmd --reload
```

### 步骤4：从Windows测试访问

#### 方法1：使用浏览器
直接访问：
- API文档: http://192.168.32.128:8081/api-docs.html
- 后端API: http://192.168.32.128:8080/health

#### 方法2：使用curl（Windows PowerShell）
```powershell
# 测试健康检查
curl http://192.168.32.128:8080/health

# 测试API文档
curl http://192.168.32.128:8081/api-docs.html
```

#### 方法3：使用Postman
1. 打开Postman
2. 创建新请求
3. URL: `http://192.168.32.128:8080/api/v1/auth/login`
4. 方法: POST
5. Body: 
   ```json
   {
     "username": "testuser",
     "password": "Test123456"
   }
   ```
6. 发送请求

## 🐛 常见问题

### Q1: 无法ping通虚拟机IP

**可能原因**:
1. 虚拟机网络模式不正确
2. Windows防火墙阻止ICMP
3. 虚拟机网络适配器未启用

**解决方案**:
```bash
# 在虚拟机中检查网络接口
ip link show ens33

# 如果接口down，启用它
sudo ip link set ens33 up

# 重启网络服务
sudo systemctl restart NetworkManager
```

### Q2: ping通但无法访问服务

**可能原因**:
1. 服务未启动
2. 服务监听在127.0.0.1而不是0.0.0.0
3. 防火墙阻止端口

**解决方案**:
```bash
# 检查服务是否监听在0.0.0.0
netstat -tlnp | grep -E "8080|8081"

# 如果监听在127.0.0.1，需要修改服务配置
# 对于API文档服务，start-api-docs.sh已配置为0.0.0.0
# 对于后端API，检查config/config.json中的host配置
```

### Q3: 浏览器显示"无法访问此网站"

**可能原因**:
1. IP地址错误
2. 端口号错误
3. 服务未运行

**解决方案**:
```bash
# 在虚拟机中确认IP地址
ip addr show ens33 | grep inet

# 确认服务运行状态
ps aux | grep -E "knot_image_sharing|python3.*8081"
```

### Q4: NAT模式下无法访问

**解决方案**:
配置端口转发（VMware示例）：
1. 编辑 → 虚拟网络编辑器
2. 选择NAT网络
3. NAT设置 → 端口转发
4. 添加规则：
   - 主机端口: 8080 → 虚拟机IP: 192.168.32.128:8080
   - 主机端口: 8081 → 虚拟机IP: 192.168.32.128:8081

然后从Windows访问：
- http://localhost:8080
- http://localhost:8081/api-docs.html

## 📱 移动设备访问

如果您的手机和虚拟机在同一WiFi网络：

1. 确保手机连接到与虚拟机相同的WiFi
2. 在手机浏览器中访问：
   - http://192.168.32.128:8081/api-docs.html
   - http://192.168.32.128:8080/api/v1/

## 🔒 安全建议

### 开发环境
- ✅ 可以使用0.0.0.0监听所有接口
- ✅ 可以开放防火墙端口
- ⚠️ 注意不要暴露到公网

### 生产环境
- ❌ 不要使用0.0.0.0监听
- ✅ 使用反向代理（Nginx）
- ✅ 配置HTTPS
- ✅ 限制访问IP范围
- ✅ 使用防火墙规则

## 📝 快速检查清单

- [ ] 虚拟机IP地址: `192.168.32.128`
- [ ] 后端API服务运行中（端口8080）
- [ ] API文档服务运行中（端口8081）
- [ ] 服务监听在0.0.0.0
- [ ] 防火墙已开放端口（如果启用）
- [ ] Windows可以ping通虚拟机
- [ ] 浏览器可以访问API文档

## 🛠️ 自动化脚本

创建一个快速检查脚本：

```bash
#!/bin/bash
# 文件: check-network-access.sh

echo "=== Knot 网络访问检查 ==="
echo ""

# 1. 检查IP地址
echo "1. 虚拟机IP地址:"
ip addr show ens33 | grep "inet " | awk '{print $2}'
echo ""

# 2. 检查服务状态
echo "2. 服务运行状态:"
echo "后端API (8080):"
netstat -tlnp 2>/dev/null | grep 8080 || echo "  未运行"
echo "API文档 (8081):"
netstat -tlnp 2>/dev/null | grep 8081 || echo "  未运行"
echo ""

# 3. 检查防火墙
echo "3. 防火墙状态:"
if command -v ufw &> /dev/null; then
    sudo ufw status | grep -E "8080|8081|Status"
else
    echo "  UFW未安装"
fi
echo ""

# 4. 访问地址
echo "4. 访问地址:"
VM_IP=$(ip addr show ens33 | grep "inet " | awk '{print $2}' | cut -d'/' -f1)
echo "  API文档: http://$VM_IP:8081/api-docs.html"
echo "  后端API: http://$VM_IP:8080/api/v1/"
echo ""

echo "=== 检查完成 ==="
```

保存并运行：
```bash
chmod +x check-network-access.sh
./check-network-access.sh
```

## 📞 技术支持

如果仍然无法访问，请检查：
1. 虚拟机网络配置
2. Windows防火墙设置
3. 路由器配置（如果跨子网）

---

**最后更新**: 2025-10-08  
**适用版本**: Knot v1.2.0

