# 开发测试环境 Deep Link 使用指南

**文档编号**: [120]  
**创建时间**: 2025-10-16  
**适用场景**: 开发测试阶段，后端部署到测试服务器  
**版本**: v2.3.0+

---

## 🎯 你的使用场景

### 当前环境配置

```json
{
  "server": {
    "host": "0.0.0.0",
    "port": 8080,
    "base_url": "http://8.138.115.164:8080",
    "environment": "development"  // ← 关键配置
  }
}
```

### 环境说明

- ✅ 后端部署到测试服务器（8.138.115.164:8080）
- ✅ 前端开发测试时调用这个后端API
- ✅ **仍然属于开发测试阶段**
- ❌ **不是生产上线阶段**
- ❌ **域名未备案，不能使用App Link**

---

## 📱 后端生成的Deep Link格式

### API响应示例

**请求**：
```bash
POST http://8.138.115.164:8080/api/v1/posts/1/share
Authorization: Bearer YOUR_JWT_TOKEN
Content-Type: application/json

{"expire_days": 0}
```

**响应**（development环境）：
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
      "ios_note": "开发环境：需要在Xcode中配置URL Scheme",
      "android": "knot://post/POST_XXX",
      "android_note": "开发环境：需要在AndroidManifest.xml中配置intent-filter",
      "environment": "development",
      "recommendation": "开发环境：建议使用自定义Scheme (knot://)，无需域名备案"
    }
  }
}
```

**关键点**：
- ✅ 所有平台都使用 `knot://` 自定义Scheme
- ✅ 无需域名备案
- ✅ 无需HTTPS证书
- ✅ 无需 .well-known 验证文件
- ❌ 不会生成 `harmonyos_app_link` 字段
- ❌ 不会生成 `https://knot.app/post/xxx` 格式

---

## 🔧 前端需要做什么

### HarmonyOS 配置

#### module.json5

```json
{
  "module": {
    "abilities": [
      {
        "name": "EntryAbility",
        "skills": [
          {
            "uris": [
              {
                "scheme": "knot",
                "path": "post"
              }
            ],
            "actions": [
              "ohos.want.action.viewData"
            ]
          }
        ]
      }
    ]
  }
}
```

#### EntryAbility.ts

```typescript
import Want from '@ohos.app.ability.Want';

export default class EntryAbility extends UIAbility {
  onNewWant(want: Want) {
    // 处理Deep Link
    if (want.uri) {
      const uri = want.uri;
      console.log('收到Deep Link:', uri);
      
      // 解析：knot://post/POST_123
      if (uri.startsWith('knot://post/')) {
        const postId = uri.split('/').pop();
        
        // 跳转到帖子详情页
        router.pushUrl({
          url: 'pages/PostDetail',
          params: { postId: postId }
        });
      }
    }
  }
}
```

### iOS 配置

#### Info.plist

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

#### AppDelegate.swift

```swift
func application(_ app: UIApplication, 
                 open url: URL, 
                 options: [UIApplication.OpenURLOptionsKey : Any] = [:]) -> Bool {
    // 解析：knot://post/POST_123
    if url.scheme == "knot" && url.host == "post" {
        let postId = url.lastPathComponent
        // 跳转到帖子详情页
        navigateToPostDetail(postId: postId)
        return true
    }
    return false
}
```

### Android 配置

#### AndroidManifest.xml

```xml
<activity android:name=".MainActivity">
  <intent-filter>
    <action android:name="android.intent.action.VIEW" />
    <category android:name="android.intent.category.DEFAULT" />
    <category android:name="android.intent.category.BROWSABLE" />
    <data android:scheme="knot" android:host="post" />
  </intent-filter>
</activity>
```

#### MainActivity.kt

```kotlin
override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    
    // 处理Deep Link
    intent?.data?.let { uri ->
        // 解析：knot://post/POST_123
        if (uri.scheme == "knot" && uri.host == "post") {
            val postId = uri.lastPathSegment
            // 跳转到帖子详情页
            navigateToPostDetail(postId)
        }
    }
}
```

---

## ⚠️ 重要提醒

### 不需要的东西

开发测试环境**完全不需要**以下内容：

❌ **不需要域名备案**
❌ **不需要HTTPS证书**
❌ **不需要 .well-known 验证文件**
❌ **不需要 apple-app-site-association**
❌ **不需要 assetlinks.json**
❌ **不需要 applinking.json**
❌ **不需要配置 App Link / Universal Link / App Linking**

### 只需要的东西

✅ **配置自定义 Scheme (`knot`)**
✅ **在应用中处理 `knot://post/xxx` 格式的URL**
✅ **后端 config.json 中设置 `"environment": "development"`**

---

## 🧪 测试方法

### HarmonyOS 测试

```bash
# 使用 hdc 命令行工具
hdc shell aa start -a EntryAbility -b com.example.knot -U "knot://post/POST_123"
```

### iOS 测试

```bash
# 使用 xcrun 命令行工具
xcrun simctl openurl booted "knot://post/POST_123"
```

### Android 测试

```bash
# 使用 adb 命令行工具
adb shell am start -W -a android.intent.action.VIEW -d "knot://post/POST_123" com.example.knot
```

### 预期效果

- ✅ 应用自动打开
- ✅ 跳转到帖子详情页
- ✅ 显示对应的帖子内容

---

## 🔄 从开发到生产的迁移

### 开发测试环境（当前）

**后端配置**：
```json
{
  "server": {
    "environment": "development"
  }
}
```

**生成的Deep Link**：
```
knot://post/POST_XXX
```

**前端配置**：
- 配置自定义Scheme `knot`
- 不需要域名验证文件

---

### 生产环境（未来）

**后端配置**：
```json
{
  "server": {
    "environment": "production",
    "domain": "knot.app"
  }
}
```

**生成的Deep Link**：
```
https://knot.app/post/POST_XXX  (iOS, Android)
knot://post/POST_XXX            (HarmonyOS降级方案)
```

**前端配置**：
- 配置 Universal Link / App Link / App Linking
- 需要部署 .well-known 验证文件（见 `deploy-static/well-known-reference/`）
- 需要域名备案和HTTPS证书

**迁移步骤**：
1. 域名备案完成
2. 配置HTTPS证书
3. 部署 .well-known 验证文件到Web服务器
4. 前端添加 Universal/App Link 配置
5. 后端修改 `environment` 为 `production`
6. 测试验证

---

## ❓ 常见问题

### Q1: well-known-reference 目录是什么？

**A**: 这是生产环境配置的参考文件，**开发测试环境不需要**。

- `deploy-static/well-known-reference/` 目录
- 包含 `apple-app-site-association`, `assetlinks.json`, `applinking.json`
- 这些文件需要部署到 Web 服务器的域名根目录
- 用于生产环境的 App Link / Universal Link / App Linking
- **你现在不需要关心这些文件**

### Q2: 为什么响应中没有 harmonyos_app_link？

**A**: 因为你的配置是 `"environment": "development"`。

- development 环境不生成 App Link 格式
- 只生成自定义 Scheme 格式（`knot://`）
- 这是正确的行为，符合你的需求

### Q3: 如何验证后端配置正确？

**A**: 调用分享API，检查响应中的 `environment` 字段：

```bash
curl -X POST http://8.138.115.164:8080/api/v1/posts/1/share \
  -H "Authorization: Bearer YOUR_TOKEN" \
  -H "Content-Type: application/json" \
  -d '{"expire_days": 0}'
```

**预期响应**：
```json
{
  "data": {
    "deep_links": {
      "environment": "development",  // ← 应该是 "development"
      "harmonyos": "knot://post/xxx" // ← 应该是 knot:// 开头
    }
  }
}
```

### Q4: 需要修改后端代码吗？

**A**: 不需要！后端已经正确实现：

- `config.json` 中 `"environment": "development"` → 生成 `knot://`
- `config.json` 中 `"environment": "production"` → 生成 `https://`
- 代码会自动根据配置选择正确的格式

### Q5: 前端可以同时支持两种格式吗？

**A**: 可以！建议同时配置：

- 配置自定义 Scheme（开发测试用）
- 配置 Universal/App Link（生产环境用）
- 应用会优先使用 HTTPS 格式，降级到 Scheme

---

## 📚 相关文档

- `[117]分享系统后端实现总结.md` - 后端实现细节
- `[118]Deep-Link配置参考-前端对接指南.md` - 完整配置指南（包含生产环境）
- `deploy-static/well-known-reference/README.md` - 生产环境配置参考（当前不需要）

---

## ✅ 总结

**你当前需要做的事情**：

1. ✅ 后端已正确配置（`environment: development`）
2. ✅ 后端会生成 `knot://` 格式的Deep Link
3. ✅ 前端只需配置自定义 Scheme `knot`
4. ✅ 不需要域名、HTTPS、验证文件

**你当前不需要做的事情**：

1. ❌ 不需要关心 `well-known-reference` 目录
2. ❌ 不需要配置 App Link / Universal Link / App Linking
3. ❌ 不需要域名备案
4. ❌ 不需要修改后端代码

**测试流程**：

```bash
# 1. 调用后端API获取Deep Link
curl http://8.138.115.164:8080/api/v1/posts/1/share -H "Authorization: Bearer TOKEN"

# 2. 使用返回的 knot://post/xxx 链接测试
hdc shell aa start -a EntryAbility -b com.example.knot -U "knot://post/POST_XXX"

# 3. 验证应用是否正确跳转到帖子详情页
```

---

**最后更新**: 2025-10-16  
**适用环境**: 开发测试阶段  
**后端配置**: `environment: development`




