# Knot图片分享系统 - Apifox测试流程

**测试目标**: 验证BUG #1修复后,POST/PUT/DELETE请求是否正常返回JSON响应  
**测试环境**: http://localhost:8080  
**前置条件**: 服务器已启动并运行

---

## 📋 测试准备

### 1. 启动服务器

```bash
cd /home/kun/projects/Knot/backend-service
LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./build/knot_image_sharing config/config.json
```

### 2. 验证服务器状态

浏览器访问或curl测试:
```bash
curl http://localhost:8080/health
```

预期返回:
```json
{
  "status": "healthy",
  "database": "connected",
  "service": "Knot - Image Sharing Service"
}
```

---

## 🧪 Apifox测试步骤

### 测试1: 用户登录(获取TOKEN)

**目的**: 获取JWT令牌用于后续认证

**配置:**
- **方法**: POST
- **URL**: `http://localhost:8080/api/v1/auth/login`
- **Headers**:
  ```
  Content-Type: application/json
  ```
- **Body (JSON)**:
  ```json
  {
    "username": "testuser",
    "password": "Test123456"
  }
  ```

**预期响应:**
```json
{
  "success": true,
  "message": "登录成功",
  "data": {
    "user_id": "...",
    "username": "testuser",
    "access_token": "eyJhbGciOiJIUzI1NiJ9...",
    "refresh_token": "eyJhbGciOiJIUzI1NiJ9...",
    "token_type": "Bearer",
    "expires_in": 3600
  },
  "timestamp": 1759850000
}
```

**操作:**
1. 点击"发送"按钮
2. ✅ **验证**: 检查HTTP状态码是否为 `200 OK`
3. ✅ **验证**: 响应体中`success`字段为`true`
4. ✅ **验证**: `data.access_token`存在且不为空
5. **复制** `data.access_token` 的值(后续测试需要)

---

### 测试2: POST上传图片 ⚠️ **核心测试**

**目的**: 验证POST请求能否正常返回JSON响应(BUG #1修复验证)

**配置:**
- **方法**: POST
- **URL**: `http://localhost:8080/api/v1/images`
- **Headers**:
  ```
  Authorization: Bearer <粘贴刚才复制的access_token>
  ```
- **Body (form-data)**:
  | Key | Type | Value |
  |-----|------|-------|
  | image | File | 选择一张图片文件(如test/pictures/mysql.png) |
  | title | Text | Apifox测试-图片上传 |
  | description | Text | 验证POST请求响应是否正常 |
  | tags | Text | 测试,Apifox |

**预期响应:**
```json
{
  "success": true,
  "message": "图片上传成功",
  "data": {
    "id": 6,
    "image_id": "IMG_2025Q4_XXXXX",
    "user_id": 2,
    "title": "Apifox测试-图片上传",
    "description": "验证POST请求响应是否正常",
    "file_url": "/uploads/images/xxx.jpg",
    "thumbnail_url": "/uploads/thumbnails/xxx_thumb.jpg",
    "file_size": 9133,
    "width": 472,
    "height": 325,
    "mime_type": "image/png",
    "like_count": 0,
    "favorite_count": 0,
    "view_count": 0,
    "status": "APPROVED",
    "create_time": 1759850000,
    "update_time": 1759850000
  },
  "timestamp": 1759850000
}
```

**验证重点:**
1. ✅ **HTTP状态码**: `201 Created`
2. ✅ **响应延迟**: 应该在1-3秒内返回(不是无响应卡住)
3. ✅ **响应体**: 完整的JSON格式
4. ✅ **success字段**: `true`
5. ✅ **data.image_id**: 存在(格式如 IMG_2025Q4_XXXXX)
6. ✅ **响应头**: 包含CORS头
   ```
   Access-Control-Allow-Origin: *
   Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS
   ```

**⚠️ BUG #1症状(修复前):**
- ❌ 无任何响应(请求卡住或超时)
- ❌ Apifox显示"请求超时"或"无响应"
- ❌ 但数据库和文件系统中图片已成功保存

**✅ 修复后预期:**
- ✅ 1-3秒内返回完整JSON响应
- ✅ HTTP 201状态码
- ✅ 响应体包含完整图片信息

**记录image_id:** 从响应中复制 `data.image_id` 的值(后续测试需要)

---

### 测试3: GET获取图片列表(回归测试)

**目的**: 确认GET请求不受修复影响

**配置:**
- **方法**: GET
- **URL**: `http://localhost:8080/api/v1/images?page=1&page_size=10`
- **Headers**: 无需认证

**预期响应:**
```json
{
  "success": true,
  "message": "查询成功",
  "data": {
    "total": 2,
    "page": 1,
    "page_size": 10,
    "images": [
      {
        "image_id": "IMG_2025Q4_XXXXX",
        "title": "Apifox测试-图片上传",
        ...
      }
    ]
  },
  "timestamp": 1759850000
}
```

**验证:**
1. ✅ HTTP状态码: `200 OK`
2. ✅ `data.images`数组包含刚才上传的图片
3. ✅ 快速响应(< 500ms)

---

### 测试4: PUT更新图文配文 ⚠️ **核心测试**

**目的**: 验证PUT请求能否正常返回JSON响应(BUG #1修复验证)

**配置:**
- **方法**: PUT
- **URL**: `http://localhost:8080/api/v1/images/<image_id>`
  - 将`<image_id>`替换为测试2中获取的image_id
- **Headers**:
  ```
  Authorization: Bearer <access_token>
  Content-Type: application/json
  ```
- **Body (JSON)**:
  ```json
  {
    "title": "Apifox测试-图片上传(已更新)",
    "description": "验证PUT请求响应是否正常 - 修复成功!"
  }
  ```

**预期响应:**
```json
{
  "success": true,
  "message": "更新成功",
  "data": null,
  "timestamp": 1759850000
}
```

**验证重点:**
1. ✅ **HTTP状态码**: `200 OK`
2. ✅ **快速响应**: < 500ms
3. ✅ **完整JSON**: 不是空响应
4. ✅ **success**: `true`

**⚠️ BUG #1症状(修复前):**
- ❌ 无任何响应
- ❌ Apifox显示请求完成但无响应体

**✅ 修复后预期:**
- ✅ 立即返回JSON响应
- ✅ HTTP 200状态码

---

### 测试5: GET获取图片详情(验证更新)

**目的**: 验证PUT操作确实生效

**配置:**
- **方法**: GET
- **URL**: `http://localhost:8080/api/v1/images/<image_id>`
- **Headers**: 无需认证

**预期响应:**
```json
{
  "success": true,
  "message": "查询成功",
  "data": {
    "image_id": "IMG_2025Q4_XXXXX",
    "title": "Apifox测试-图片上传(已更新)",
    "description": "验证PUT请求响应是否正常 - 修复成功!",
    "view_count": 1,
    ...
  }
}
```

**验证:**
1. ✅ 标题已更新为"Apifox测试-图片上传(已更新)"
2. ✅ 描述已更新
3. ✅ view_count增加(每次GET会+1)

---

### 测试6: DELETE删除图片 ⚠️ **核心测试**

**目的**: 验证DELETE请求能否正常返回JSON响应(BUG #1修复验证)

**配置:**
- **方法**: DELETE
- **URL**: `http://localhost:8080/api/v1/images/<image_id>`
- **Headers**:
  ```
  Authorization: Bearer <access_token>
  ```
- **Body**: 无

**预期响应:**
```json
{
  "success": true,
  "message": "删除成功",
  "data": null,
  "timestamp": 1759850000
}
```

**验证重点:**
1. ✅ **HTTP状态码**: `200 OK`
2. ✅ **快速响应**: < 500ms
3. ✅ **完整JSON**: 不是空响应
4. ✅ **success**: `true`

**⚠️ BUG #1症状(修复前):**
- ❌ 无任何响应
- ❌ Apifox显示请求完成但无响应体
- ❌ 但数据库中图片记录已删除

**✅ 修复后预期:**
- ✅ 立即返回JSON响应
- ✅ HTTP 200状态码

---

### 测试7: GET验证删除(确认已删除)

**目的**: 确认DELETE操作确实生效

**配置:**
- **方法**: GET
- **URL**: `http://localhost:8080/api/v1/images/<image_id>`
- **Headers**: 无需认证

**预期响应:**
```json
{
  "success": false,
  "message": "图片不存在",
  "data": null,
  "timestamp": 1759850000
}
```

**验证:**
1. ✅ HTTP状态码: `404 Not Found`
2. ✅ `success`: `false`
3. ✅ 提示图片不存在

---

## 📊 测试检查清单

### BUG #1修复验证

| 测试项 | 修复前表现 | 修复后预期 | 测试结果 |
|--------|-----------|-----------|---------|
| POST上传图片响应 | ❌ 无响应/超时 | ✅ 完整JSON,HTTP 201 | ⬜ 待测 |
| PUT更新配文响应 | ❌ 无响应/超时 | ✅ 完整JSON,HTTP 200 | ⬜ 待测 |
| DELETE删除图片响应 | ❌ 无响应/超时 | ✅ 完整JSON,HTTP 200 | ⬜ 待测 |
| GET请求(回归) | ✅ 正常 | ✅ 仍正常 | ⬜ 待测 |
| CORS头 | ✅ 正常 | ✅ 仍正常 | ⬜ 待测 |

### 功能完整性验证

| 测试项 | 预期结果 | 测试结果 |
|--------|---------|---------|
| 用户登录 | ✅ 返回TOKEN | ⬜ 待测 |
| 图片上传 | ✅ 文件保存+数据库记录 | ⬜ 待测 |
| 图片列表 | ✅ 返回最新图片 | ⬜ 待测 |
| 配文更新 | ✅ 数据库更新 | ⬜ 待测 |
| 图片详情 | ✅ 返回完整信息 | ⬜ 待测 |
| 图片删除 | ✅ 数据库删除+404 | ⬜ 待测 |

---

## 🔧 Apifox配置技巧

### 1. 环境变量设置

在Apifox中设置环境变量,方便切换:

**环境名**: Local Development

| 变量名 | 值 |
|--------|-----|
| base_url | http://localhost:8080 |
| access_token | (运行测试1后手动填入) |
| current_image_id | (运行测试2后手动填入) |

**使用方式:**
- URL改为: `{{base_url}}/api/v1/images`
- Authorization改为: `Bearer {{access_token}}`

### 2. 自动化测试脚本

在Apifox的"后置操作"中添加脚本,自动提取TOKEN:

**测试1(登录)后置脚本:**
```javascript
// 自动提取access_token到环境变量
const response = pm.response.json();
if (response.success) {
    pm.environment.set("access_token", response.data.access_token);
    console.log("Token已保存:", response.data.access_token);
}
```

**测试2(上传)后置脚本:**
```javascript
// 自动提取image_id到环境变量
const response = pm.response.json();
if (response.success) {
    pm.environment.set("current_image_id", response.data.image_id);
    console.log("Image ID已保存:", response.data.image_id);
}
```

### 3. 断言检查

在Apifox的"断言"标签添加自动验证:

**POST上传图片断言:**
```javascript
pm.test("状态码为201", () => {
    pm.response.to.have.status(201);
});

pm.test("响应时间小于3秒", () => {
    pm.expect(pm.response.responseTime).to.be.below(3000);
});

pm.test("返回成功标志", () => {
    const json = pm.response.json();
    pm.expect(json.success).to.be.true;
});

pm.test("包含image_id", () => {
    const json = pm.response.json();
    pm.expect(json.data.image_id).to.exist;
});

pm.test("包含CORS头", () => {
    pm.response.to.have.header("Access-Control-Allow-Origin");
});
```

---

## 🐛 问题排查

### 如果POST/PUT/DELETE仍无响应:

1. **检查服务器是否使用修复后的版本:**
   ```bash
   ps aux | grep knot_image_sharing
   # 检查进程启动时间是否在代码修复之后
   ```

2. **重新编译并启动:**
   ```bash
   cd /home/kun/projects/Knot/backend-service
   pkill -f knot_image_sharing
   rm -rf build && mkdir build && cd build
   cmake .. && make -j4
   cd ..
   LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu ./build/knot_image_sharing config/config.json
   ```

3. **检查日志:**
   ```bash
   tail -f /home/kun/projects/Knot/backend-service/logs/auth-service.log
   ```
   应该能看到 "Response: XXX for POST/PUT/DELETE" 的日志

4. **验证修复代码:**
   ```bash
   grep -A 10 "setupMiddleware" src/server/http_server.cpp | grep -c "Access-Control"
   # 应该输出 4 (表示CORS头在setupMiddleware中设置)
   ```

---

## ✅ 测试完成标准

全部测试通过的标志:
- ✅ 所有7个测试都返回JSON响应(无超时)
- ✅ POST返回HTTP 201,PUT/DELETE返回HTTP 200
- ✅ 响应时间都在合理范围内(< 3秒)
- ✅ 所有响应包含CORS头
- ✅ 功能正确执行(上传成功/更新生效/删除成功)

---

**文档版本**: v1.0  
**最后更新**: 2025-10-07  
**用途**: BUG #1修复验证

