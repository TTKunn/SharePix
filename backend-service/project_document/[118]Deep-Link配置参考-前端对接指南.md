# Deep Link配置参考 - 前端对接指南

**文档编号**: [118]  
**创建时间**: 2025-10-16  
**适用对象**: 前端开发者  
**相关版本**: v2.3.0+

---

## 📋 后端提供的内容

### API接口

#### 1. 创建分享链接

```http
POST /api/v1/posts/:post_id/share
Authorization: Bearer JWT_TOKEN
Content-Type: application/json

{
  "expire_days": 0  // 0表示永不过期
}
```

**响应示例（开发环境）**:
```json
{
  "success": true,
  "data": {
    "short_code": "kHr7ZS9a",
    "short_url": "http://8.138.115.164:8080/s/kHr7ZS9a",
    "deep_links": {
      "harmonyos": "knot://post/POST_XXX",
      "harmonyos_scheme": "knot://post/POST_XXX",
      "ios": "knot://post/POST_XXX",
      "android": "knot://post/POST_XXX",
      "environment": "development",
      "recommendation": "开发环境：建议使用自定义Scheme (knot://)，无需域名备案"
    }
  }
}
```

**响应示例（生产环境）**:
```json
{
  "success": true,
  "data": {
    "short_code": "kHr7ZS9a",
    "short_url": "https://knot.app/s/kHr7ZS9a",
    "deep_links": {
      "harmonyos": "knot://post/POST_XXX",
      "harmonyos_scheme": "knot://post/POST_XXX",
      "harmonyos_app_link": "https://knot.app/post/POST_XXX",
      "ios": "https://knot.app/post/POST_XXX",
      "android": "https://knot.app/post/POST_XXX",
      "environment": "production",
      "recommendation": "生产环境：建议使用域名验证的App Link"
    }
  }
}
```

#### 2. 解析分享链接

```http
GET /api/v1/share/:code
```

**响应**:
```json
{
  "success": true,
  "data": {
    "post": {
      "post_id": "POST_XXX",
      "title": "标题",
      "content": "内容",
      // ... 完整帖子信息
    },
    "expired": false
  }
}
```

---

## 🎯 前端集成方案

### 开发/测试环境

**特点**:
- ✅ 使用自定义Scheme（`knot://`）
- ✅ 无需域名备案
- ✅ 无需HTTPS证书
- ✅ 本地即可测试

**Deep Link格式**:
- HarmonyOS: `knot://post/POST_XXX`
- iOS: `knot://post/POST_XXX`
- Android: `knot://post/POST_XXX`

**前端需要做的事**:
1. 在应用中配置URL Scheme为 `knot`
2. 注册路由 `/post/:id` 的处理函数
3. 接收到Deep Link时解析参数跳转页面

### 生产环境

**特点**:
- ✅ 使用HTTPS域名（`https://knot.app/post/POST_XXX`）
- ✅ 无需用户确认，体验更流畅
- ❌ 需要域名备案和HTTPS证书
- ❌ 需要配置域名验证文件

**Deep Link格式**:
- iOS Universal Links: `https://knot.app/post/POST_XXX`
- Android App Links: `https://knot.app/post/POST_XXX`
- HarmonyOS App Linking: `https://knot.app/post/POST_XXX`
- HarmonyOS Scheme（降级）: `knot://post/POST_XXX`

**前端需要做的事**:
1. 配置Universal Links / App Links
2. 上传域名验证文件（见下文）
3. 在应用中处理HTTPS链接

---

## 📱 各平台配置要点

### HarmonyOS

#### 方案1：自定义Scheme（开发环境推荐）

**module.json5**:
```json
{
  "abilities": [{
    "skills": [{
      "uris": [
        {
          "scheme": "knot",
          "path": "post"
        }
      ]
    }]
  }]
}
```

**处理Deep Link**:
```typescript
// EntryAbility.ts
onNewWant(want: Want) {
  const uri = want.uri;
  if (uri && uri.startsWith('knot://post/')) {
    const postId = uri.split('/').pop();
    // 跳转到帖子详情页
  }
}
```

#### 方案2：App Linking（生产环境推荐）

**要求**:
- ✅ 域名备案
- ✅ 在AGC控制台配置
- ✅ 上传 `applinking.json` 到域名根目录

**module.json5**:
```json
{
  "abilities": [{
    "skills": [{
      "uris": [
        {
          "scheme": "https",
          "host": "knot.app",
          "path": "post"
        }
      ]
    }]
  }]
}
```

### iOS

#### 方案1：自定义Scheme（开发环境）

**Info.plist**:
```xml
<key>CFBundleURLTypes</key>
<array>
  <dict>
    <key>CFBundleURLSchemes</key>
    <array>
      <string>knot</string>
    </array>
  </dict>
</array>
```

#### 方案2：Universal Links（生产环境）

**EntryInfo.plist**:
```xml
<key>com.apple.developer.associated-domains</key>
<array>
  <string>applinks:knot.app</string>
</array>
```

### Android

#### 方案1：自定义Scheme（开发环境）

**AndroidManifest.xml**:
```xml
<activity>
  <intent-filter>
    <action android:name="android.intent.action.VIEW" />
    <category android:name="android.intent.category.DEFAULT" />
    <category android:name="android.intent.category.BROWSABLE" />
    <data android:scheme="knot" android:host="post" />
  </intent-filter>
</activity>
```

#### 方案2：App Links（生产环境）

**AndroidManifest.xml**:
```xml
<activity>
  <intent-filter android:autoVerify="true">
    <action android:name="android.intent.action.VIEW" />
    <category android:name="android.intent.category.DEFAULT" />
    <category android:name="android.intent.category.BROWSABLE" />
    <data android:scheme="https" 
          android:host="knot.app" 
          android:pathPrefix="/post" />
  </intent-filter>
</activity>
```

---

## 🌐 域名验证文件（生产环境）

### 存放位置

后端项目中提供了参考配置（仅供参考）：
- `deploy-static/well-known-reference/apple-app-site-association`（iOS）
- `deploy-static/well-known-reference/assetlinks.json`（Android）
- `deploy-static/well-known-reference/applinking.json`（HarmonyOS）

### 部署要求

**必须部署到域名根目录的 `.well-known/` 下**：
- `https://knot.app/.well-known/apple-app-site-association`
- `https://knot.app/.well-known/assetlinks.json`
- `https://knot.app/.well-known/applinking.json`

**关键要求**:
1. ✅ 必须是HTTPS
2. ✅ Content-Type必须正确
3. ✅ 可公开访问（无需认证）
4. ✅ 返回200状态码

**Nginx配置示例**:
```nginx
server {
    listen 443 ssl;
    server_name knot.app;
    
    location /.well-known/ {
        alias /var/www/knot/.well-known/;
        
        location ~ /apple-app-site-association$ {
            default_type application/json;
        }
        
        location ~ /assetlinks.json$ {
            default_type application/json;
        }
        
        location ~ /applinking.json$ {
            default_type application/json;
        }
    }
}
```

---

## 🧪 测试方法

### 开发环境测试

#### HarmonyOS
```bash
# 使用hdc（HarmonyOS DevEco CLI）
hdc shell aa start -a EntryAbility -b com.example.knot -U "knot://post/POST_123"
```

#### iOS
```bash
# 使用xcrun（Xcode Command Line Tools）
xcrun simctl openurl booted "knot://post/POST_123"
```

#### Android
```bash
# 使用adb
adb shell am start -W -a android.intent.action.VIEW -d "knot://post/POST_123"
```

### 生产环境测试

**验证域名文件**:
```bash
curl -I https://knot.app/.well-known/apple-app-site-association
curl -I https://knot.app/.well-known/assetlinks.json
curl -I https://knot.app/.well-known/applinking.json
```

**验证Deep Link**（需要在真机上测试）:
1. 在微信/短信中发送 `https://knot.app/post/POST_123`
2. 点击链接
3. 应该直接打开应用并跳转到帖子详情页

---

## 📊 环境对比

| 特性 | 开发环境 | 生产环境 |
|------|---------|---------|
| Deep Link格式 | `knot://post/XXX` | `https://knot.app/post/XXX` |
| 域名要求 | ❌ 不需要 | ✅ 需要备案 |
| HTTPS证书 | ❌ 不需要 | ✅ 必需 |
| 验证文件 | ❌ 不需要 | ✅ 必需 (.well-known) |
| 用户体验 | 需要确认 | 无需确认（无缝跳转） |
| 配置复杂度 | 低 | 高 |
| 测试便利性 | 高（本地即可） | 低（需真实域名） |

---

## 🚀 快速开始（前端开发者）

### 第一步：确定环境

询问后端当前配置的环境（development/production）：
```bash
curl http://your-server/api/v1/posts/1/share -H "Authorization: Bearer TOKEN"
```

查看响应中的 `environment` 字段。

### 第二步：配置应用

- **开发环境**: 只需配置自定义Scheme `knot`
- **生产环境**: 配置Universal/App/App Linking

### 第三步：处理Deep Link

```typescript
// 伪代码示例
function handleDeepLink(url: string) {
  // 提取postId
  const match = url.match(/\/post\/([^\/]+)/);
  if (match) {
    const postId = match[1];
    
    // 跳转到帖子详情页
    router.push({
      name: 'PostDetail',
      params: { id: postId }
    });
  }
}
```

### 第四步：测试

使用上面的测试命令验证Deep Link是否正常工作。

---

## 📝 常见问题

### Q1: 为什么开发环境不用HTTPS链接？

A: 开发/测试阶段域名通常未备案，无法使用HTTPS和域名验证。自定义Scheme无需域名，可本地测试。

### Q2: 自定义Scheme和HTTPS链接可以共存吗？

A: 可以。应用可以同时支持两种方式，优先使用HTTPS链接，降级到Scheme。

### Q3: HarmonyOS为什么始终包含Scheme格式？

A: 因为App Linking配置较复杂，提供Scheme作为降级方案，确保开发/测试阶段可用。

### Q4: 需要前端自己生成Deep Link吗？

A: 不需要。后端API会根据环境配置自动生成正确格式的Deep Link，前端直接使用即可。

---

## 📚 相关文档

- `[117]分享系统后端实现总结.md` - 后端实现细节
- `[109]阶段D-3-分享系统实现计划.md` - 完整技术设计
- `[000]API文档.md` - API接口文档
- `deploy-static/well-known-reference/README.md` - 域名验证文件参考

---

**提示**: 本文档仅提供前端配置参考。详细的前端实现由前端团队根据各平台SDK文档完成。

**最后更新**: 2025-10-16




