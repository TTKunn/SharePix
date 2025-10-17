# .well-known 域名验证文件参考

**目录说明**: 本目录包含Deep Link域名验证文件的参考配置，**仅用于生产环境**。

## ⚠️ 重要提示

**开发测试环境不需要这个目录下的任何文件！**

- ✅ 开发测试环境使用自定义Scheme（`knot://`），无需域名验证
- ✅ 只有生产环境才需要这些文件（需要域名备案）
- ✅ 详见：`project_document/[120]开发测试环境Deep-Link使用指南.md`

---

## 📋 文件清单

1. **apple-app-site-association** - iOS Universal Links配置
2. **assetlinks.json** - Android App Links配置
3. **applinking.json** - HarmonyOS App Linking配置

---

## ⚠️ 重要提示

**这些文件仅供参考，不会被后端服务使用！**

- ❌ 后端API服务不提供这些文件
- ✅ 需要由运维/前端团队部署到Web服务器
- ✅ 必须部署到域名根目录的 `.well-known/` 下

---

## 🌐 生产环境部署

### 前置条件

1. ✅ 域名已备案（例如：`knot.app`）
2. ✅ 配置了HTTPS证书
3. ✅ 有Web服务器（Nginx/Apache/CDN）

### 部署位置

**必须部署到以下URL（可公开访问）**：

```
https://knot.app/.well-known/apple-app-site-association
https://knot.app/.well-known/assetlinks.json
https://knot.app/.well-known/applinking.json
```

### Nginx配置示例

```nginx
server {
    listen 443 ssl http2;
    server_name knot.app;
    
    # SSL证书配置
    ssl_certificate /path/to/cert.pem;
    ssl_certificate_key /path/to/key.pem;
    
    # .well-known文件配置
    location /.well-known/ {
        alias /var/www/knot/.well-known/;
        
        # iOS Universal Links
        location ~ /apple-app-site-association$ {
            default_type application/json;
            add_header Content-Type application/json;
        }
        
        # Android App Links
        location ~ /assetlinks.json$ {
            default_type application/json;
            add_header Content-Type application/json;
        }
        
        # HarmonyOS App Linking
        location ~ /applinking.json$ {
            default_type application/json;
            add_header Content-Type application/json;
        }
    }
    
    # 其他配置...
}
```

### 部署步骤

```bash
# 1. 在Web服务器上创建目录
sudo mkdir -p /var/www/knot/.well-known

# 2. 复制验证文件
sudo cp apple-app-site-association /var/www/knot/.well-known/
sudo cp assetlinks.json /var/www/knot/.well-known/
sudo cp applinking.json /var/www/knot/.well-known/

# 3. 设置权限
sudo chmod 644 /var/www/knot/.well-known/*
sudo chown www-data:www-data /var/www/knot/.well-known/*

# 4. 重载Nginx
sudo nginx -t
sudo systemctl reload nginx
```

---

## 🔧 配置修改指南

### 1. apple-app-site-association

**需要修改的字段**:
```json
{
  "applinks": {
    "apps": [],
    "details": [{
      "appID": "TEAM_ID.com.yourcompany.knot",  // 改为实际Team ID和Bundle ID
      "paths": ["/post/*"]
    }]
  }
}
```

**获取Team ID**:
- 登录 https://developer.apple.com
- 进入 Membership 页面
- 查看 Team ID（10位字符）

### 2. assetlinks.json

**需要修改的字段**:
```json
[{
  "target": {
    "namespace": "android_app",
    "package_name": "com.yourcompany.knot",     // 改为实际包名
    "sha256_cert_fingerprints": [
      "SHA256_FINGERPRINT"                       // 改为实际证书指纹
    ]
  }
}]
```

**获取证书指纹**:
```bash
# 使用keytool
keytool -list -v -keystore your-release-key.keystore

# 或使用gradlew
./gradlew signingReport
```

### 3. applinking.json

**需要修改的字段**:
```json
{
  "applinks": {
    "apps": [],
    "details": [{
      "appID": "YOUR_APP_ID",                   // 改为AGC控制台的App ID
      "paths": ["/post/*"]
    }]
  }
}
```

**获取App ID**:
- 登录 https://developer.huawei.com/consumer/cn/service/josp/agc/index.html
- 进入项目 -> 应用信息
- 查看 App ID

---

## ✅ 验证部署

### 方法1：浏览器访问

```bash
# 检查文件是否可访问
curl https://knot.app/.well-known/apple-app-site-association
curl https://knot.app/.well-known/assetlinks.json
curl https://knot.app/.well-known/applinking.json
```

**预期结果**:
- HTTP状态码：200
- Content-Type：application/json
- 返回完整的JSON内容

### 方法2：官方验证工具

**iOS**:
```bash
# 使用Apple的验证工具
https://search.developer.apple.com/appsearch-validation-tool/
```

**Android**:
```bash
# 使用Google的验证工具
https://developers.google.com/digital-asset-links/tools/generator
```

**HarmonyOS**:
- 在AGC控制台的"App Linking"页面验证

---

## 🐛 常见问题

### Q1: 为什么返回404？

**原因**: Nginx配置错误或文件路径不对。

**解决方案**:
1. 检查Nginx配置中的 `alias` 路径
2. 确认文件确实存在
3. 检查文件权限（需要www-data可读）

### Q2: Content-Type不正确？

**原因**: Nginx未正确设置MIME类型。

**解决方案**:
```nginx
location ~ /apple-app-site-association$ {
    default_type application/json;
}
```

### Q3: 证书指纹不匹配？

**原因**: 使用了debug证书，但配置的是release证书指纹。

**解决方案**:
- 开发阶段使用debug证书指纹
- 生产环境使用release证书指纹
- 可以配置多个指纹

---

## 📚 参考文档

**iOS Universal Links**:
- https://developer.apple.com/documentation/xcode/supporting-universal-links-in-your-app

**Android App Links**:
- https://developer.android.com/training/app-links

**HarmonyOS App Linking**:
- https://developer.huawei.com/consumer/cn/doc/development/AppGallery-connect-Guides/agc-applinking-introduction

---

## 🔄 更新流程

当应用包名、证书或配置变更时：

```bash
# 1. 修改验证文件
vim /var/www/knot/.well-known/assetlinks.json

# 2. 验证JSON格式
cat /var/www/knot/.well-known/assetlinks.json | python3 -m json.tool

# 3. 不需要重载Nginx（静态文件立即生效）

# 4. 验证更新
curl https://knot.app/.well-known/assetlinks.json
```

---

## 💡 开发环境提示

**开发/测试阶段不需要这些文件！**

- 使用自定义Scheme（`knot://`）即可
- 无需域名、HTTPS、验证文件
- 详见：`project_document/[118]Deep-Link配置参考-前端对接指南.md`

---

**最后更新**: 2025-10-16  
**维护者**: 运维团队 / 前端团队  
**版本**: v1.0.0

