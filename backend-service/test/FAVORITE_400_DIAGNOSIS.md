# 收藏接口400错误诊断报告

## 问题现象

- **Apifox调用**：正常（200响应）
- **前端调用**：返回400错误
- **特征**：前端请求有**5秒延迟**，且没有进入业务逻辑

## 根本原因

通过分析日志和cpp-httplib源码，确认问题是：

### cpp-httplib读取超时

```cpp
// httplib.h 默认配置
#define CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND 5
```

**触发条件：**
1. 前端设置了`Content-Type: application/json`
2. 但没有发送请求体（或body不完整）
3. 服务器等待读取body数据，5秒后超时返回400

## 日志证据

```log
[2025-10-18 13:09:04.002] Request: POST /api/v1/posts/POST_2025Q4_8qYFhp/favorite
[2025-10-18 13:09:09.007] Response: 400 for POST /api/v1/posts/POST_2025Q4_8qYFhp/favorite
                        ↑ 正好5秒延迟
```

对比Apifox成功请求：
```log
[2025-10-18 13:11:07.291] Request: POST /api/v1/posts/POST_2025Q4_iGOrWA/favorite
[2025-10-18 13:11:07.291] UserRepository initialized  ← 进入了业务逻辑
[2025-10-18 13:11:07.292] User 5 attempting to favorite post
[2025-10-18 13:11:07.913] Post favorited successfully
```

前端请求**没有进入业务逻辑**，说明在HTTP解析阶段就失败了。

## 问题定位：前端代码

最可能的前端代码问题：

### ❌ 错误示例1：设置了Content-Type但无body

```javascript
fetch(`${API_URL}/posts/${postId}/favorite`, {
  method: 'POST',
  headers: {
    'Authorization': `Bearer ${token}`,
    'Content-Type': 'application/json',  // ← 问题在这里
  },
  // 没有body!
})
```

### ❌ 错误示例2：body为undefined/null

```javascript
fetch(`${API_URL}/posts/${postId}/favorite`, {
  method: 'POST',
  headers: {
    'Authorization': `Bearer ${token}`,
    'Content-Type': 'application/json',
  },
  body: undefined  // 或 null
})
```

### ❌ 错误示例3：axios自动设置Content-Type

```javascript
axios.post(`${API_URL}/posts/${postId}/favorite`, 
  {},  // 空对象，axios会序列化但可能导致问题
  {
    headers: {
      'Authorization': `Bearer ${token}`
    }
  }
)
```

## 解决方案

### 方案1：前端修改（推荐）

#### ✅ 正确示例1：不设置Content-Type

```javascript
fetch(`${API_URL}/posts/${postId}/favorite`, {
  method: 'POST',
  headers: {
    'Authorization': `Bearer ${token}`,
    // 不设置Content-Type
  },
  // 不设置body
})
```

#### ✅ 正确示例2：明确设置Content-Length: 0

```javascript
fetch(`${API_URL}/posts/${postId}/favorite`, {
  method: 'POST',
  headers: {
    'Authorization': `Bearer ${token}`,
    'Content-Length': '0',
  },
})
```

#### ✅ 正确示例3：发送空JSON

```javascript
fetch(`${API_URL}/posts/${postId}/favorite`, {
  method: 'POST',
  headers: {
    'Authorization': `Bearer ${token}`,
    'Content-Type': 'application/json',
  },
  body: JSON.stringify({}),  // 发送 '{}'
})
```

### 方案2：后端修改（不推荐）

增加读取超时时间（治标不治本）：

```cpp
// src/server/http_server.cpp
bool HttpServer::initialize() {
    // ...
    server_->set_read_timeout(30, 0);  // 增加到30秒
    // ...
}
```

**不推荐理由**：
- 隐藏了前端的真实问题
- 增加服务器资源占用
- 可能影响其他正常请求

## 验证方法

### 1. 使用测试脚本

```bash
cd /home/kun/projects/SharePix/backend-service/test
./test_favorite_400.sh
```

### 2. 使用curl手动测试

```bash
# 获取token
TOKEN="your_access_token"

# 测试1: 正确方式（无Content-Type）
curl -X POST http://localhost:8080/api/v1/posts/POST_ID/favorite \
  -H "Authorization: Bearer ${TOKEN}"

# 测试2: 错误方式（会等待5秒后返回400）
curl -X POST http://localhost:8080/api/v1/posts/POST_ID/favorite \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json"
  # 注意：没有 -d 参数
```

### 3. 浏览器开发者工具检查

打开Chrome DevTools → Network：
1. 查看Request Headers中的`Content-Type`
2. 查看Request Payload是否为空
3. 查看Timing，如果Waiting时间接近5秒，说明触发了超时

## 其他受影响的接口

从日志中发现，以下接口也有同样的问题：

- POST /api/v1/posts/:post_id/like（点赞）
- POST /api/v1/users/:user_id/follow（关注）
- POST /api/v1/posts/:post_id/favorite（收藏）

**建议统一修复所有无body的POST请求**。

## 为什么Apifox正常？

Apifox在发送无body的POST请求时：
- 不会自动添加Content-Type头
- 或者会正确设置Content-Length: 0

而前端框架（如fetch、axios）可能会：
- 自动添加Content-Type: application/json
- 但body为空或undefined
- 导致Content-Type与实际body不匹配

## 总结

| 检查项 | Apifox | 前端 |
|--------|--------|------|
| Content-Type | 未设置 或 正确设置 | application/json（但无body） |
| Content-Length | 0 或 未设置 | 未设置（期待有数据） |
| 请求body | 空 | 空（但声明了Content-Type） |
| 服务器行为 | 立即响应 | 等待5秒超时 |

**核心原因**：HTTP协议中，如果设置了`Content-Type: application/json`，服务器会期待读取JSON body数据。如果没有数据到来，就会等待直到超时。

## 推荐行动

1. **立即**：检查前端代码，移除无body的POST请求中的Content-Type设置
2. **验证**：使用curl测试修改后的行为
3. **预防**：为其他无body接口（点赞、关注）做同样修复
4. **文档**：更新API文档，说明这些接口不需要请求体

## 附录：cpp-httplib行为说明

当接收到POST请求时：
1. 解析请求行和请求头
2. 如果有Content-Length或Content-Type，尝试读取body
3. 如果读取超时（5秒），返回400 Bad Request
4. 如果读取成功或没有body声明，进入路由处理

这就是为什么前端请求在5秒后返回400，且没有进入handler的原因。

