# 创建帖子接口支持JSON+Base64格式 - 实施计划

**文档编号**: [115]  
**创建时间**: 2025-10-14  
**版本**: v1.0  
**状态**: ✅ 已完成  
**关联版本**: v2.4.1

---

## 📋 目录

1. [问题背景](#问题背景)
2. [问题分析](#问题分析)
3. [解决方案](#解决方案)
4. [实施计划](#实施计划)
5. [测试计划](#测试计划)
6. [风险评估](#风险评估)
7. [实施进度](#实施进度)

---

## 问题背景

### 问题描述

前端使用 `application/json` + Base64编码的图片数据调用创建帖子接口时，收到400错误：

```
[WARNING] Request is not multipart/form-data, Content-Type: application/json
返回: 400 - "请求必须使用multipart/form-data格式"
```

### 日志记录

```log
[2025-10-14 23:47:50.061] [info] [thread 226948] === [CREATE POST] Request received ===
[2025-10-14 23:47:50.061] [info] [thread 226948] [CREATE POST] User authenticated - UserID: 5
[2025-10-14 23:47:50.061] [warning] [thread 226948] [CREATE POST] Request is not multipart/form-data, Content-Type: application/json
[2025-10-14 23:47:50.061] [info] [thread 226948] Response: 400 for POST /api/v1/posts
```

### 影响范围

- **受影响接口**: `POST /api/v1/posts` (创建帖子)
- **影响用户**: 所有使用JSON格式提交图片的客户端
- **严重程度**: 🔴 高（阻塞性问题）

---

## 问题分析

### 当前代码实现

#### 1. Content-Type强制检查

**位置**: `src/api/post_handler.cpp:119-125`

```cpp
// 2. 检查是否是multipart/form-data
if (!req.is_multipart_form_data()) {
    Logger::warning("[CREATE POST] Request is not multipart/form-data, Content-Type: " + 
                  req.get_header_value("Content-Type"));
    sendErrorResponse(res, 400, "请求必须使用multipart/form-data格式");
    return;  // ❌ 直接拒绝JSON请求
}
```

#### 2. Base64解码逻辑已存在

**位置**: `src/api/post_handler.cpp:664-730`

```cpp
// saveUploadedFile() 函数中已实现Base64检测和解码
if (Base64Decoder::isBase64(content)) {
    Logger::info("[SAVE FILE] ⚠ Base64 encoded data detected!");
    actualContent = Base64Decoder::decode(content);
    // ... 完整的解码逻辑和验证 ...
}
```

### 问题根源

**核心矛盾**：
1. ✅ 代码已支持Base64解码（在 `saveUploadedFile()` 中）
2. ❌ 但在第120行就拦截了JSON请求
3. ❌ 导致Base64解码逻辑永远无法执行

**流程图**：
```
前端发送JSON+Base64
    ↓
第120行：检查Content-Type
    ↓
发现不是multipart/form-data
    ↓
直接返回400错误 ❌
    ↓
❌ 永远到不了saveUploadedFile()的Base64解码逻辑
```

---

## 解决方案

### 方案选择：方案A - 修改后端支持JSON+Base64

**选择理由**：
1. ✅ Base64解码逻辑已实现，无需重复开发
2. ✅ 前端代码不用改，保持现有JSON格式
3. ✅ 向后兼容，同时支持multipart和JSON两种格式
4. ✅ 符合RESTful规范，JSON是API的标准数据格式
5. ✅ 日志已完善，Base64解码过程有详细日志

### 技术方案

#### JSON请求格式

```json
{
  "title": "测试帖子",
  "description": "描述内容",
  "tags": ["标签1", "标签2"],
  "images": [
    {
      "filename": "image1.jpg",
      "content_type": "image/jpeg",
      "data": "data:image/jpeg;base64,/9j/4AAQSkZJRg..."
    },
    {
      "filename": "image2.png",
      "content_type": "image/png",
      "data": "iVBORw0KGgoAAAANSUhEUgAA..."
    }
  ]
}
```

#### 修改范围

**文件**: `src/api/post_handler.cpp`

**修改区域**：
- **第119-220行**: 请求解析部分
- 移除Content-Type强制检查
- 添加JSON格式解析分支
- 复用现有的Base64解码逻辑

---

## 实施计划

### 阶段1: 代码修改

#### 任务1.1: 修改Content-Type检查逻辑 ⏳

**文件**: `src/api/post_handler.cpp`  
**行数**: 119-125

**修改前**:
```cpp
// 2. 检查是否是multipart/form-data
if (!req.is_multipart_form_data()) {
    Logger::warning("[CREATE POST] Request is not multipart/form-data, Content-Type: " + 
                  req.get_header_value("Content-Type"));
    sendErrorResponse(res, 400, "请求必须使用multipart/form-data格式");
    return;
}
```

**修改后**:
```cpp
// 2. 检查请求格式（支持multipart和JSON两种格式）
std::string contentType = req.get_header_value("Content-Type");
bool isMultipart = req.is_multipart_form_data();
bool isJson = (contentType.find("application/json") != std::string::npos);

if (!isMultipart && !isJson) {
    Logger::warning("[CREATE POST] Unsupported Content-Type: " + contentType);
    sendErrorResponse(res, 400, "请求必须使用multipart/form-data或application/json格式");
    return;
}

Logger::info("[CREATE POST] Request format: " + 
            std::string(isMultipart ? "multipart/form-data" : "application/json"));
```

#### 任务1.2: 添加JSON格式解析分支 ⏳

**文件**: `src/api/post_handler.cpp`  
**行数**: 127-220（重构此段）

**伪代码**:
```cpp
// 3. 解析请求参数
std::string title;
std::string description;
std::vector<std::string> tags;
std::vector<std::string> savedImagePaths;

if (isMultipart) {
    // === 现有的multipart/form-data处理逻辑 ===
    // ... 保持不变 ...
    
} else if (isJson) {
    // === 新增的JSON格式处理逻辑 ===
    
    // 3.1 解析JSON请求体
    Json::Value requestBody;
    if (!parseJsonBody(req.body, requestBody)) {
        sendErrorResponse(res, 400, "无效的JSON格式");
        return;
    }
    
    // 3.2 提取文本字段
    title = requestBody.get("title", "").asString();
    description = requestBody.get("description", "").asString();
    
    if (title.empty()) {
        sendErrorResponse(res, 400, "标题不能为空");
        return;
    }
    
    // 3.3 提取标签
    if (requestBody.isMember("tags") && requestBody["tags"].isArray()) {
        for (const auto& tag : requestBody["tags"]) {
            tags.push_back(tag.asString());
        }
    }
    
    // 3.4 处理图片数据（Base64编码）
    if (!requestBody.isMember("images") || !requestBody["images"].isArray()) {
        sendErrorResponse(res, 400, "缺少images字段");
        return;
    }
    
    const Json::Value& imagesArray = requestBody["images"];
    
    if (imagesArray.empty() || imagesArray.size() > 9) {
        sendErrorResponse(res, 400, "图片数量必须在1-9张之间");
        return;
    }
    
    Logger::info("[CREATE POST] Processing " + std::to_string(imagesArray.size()) + 
                " images from JSON");
    
    // 3.5 逐个处理Base64图片
    for (size_t i = 0; i < imagesArray.size(); i++) {
        const Json::Value& imageData = imagesArray[i];
        
        // 提取字段
        std::string filename = imageData.get("filename", "image.jpg").asString();
        std::string contentType = imageData.get("content_type", "image/jpeg").asString();
        std::string base64Data = imageData.get("data", "").asString();
        
        if (base64Data.empty()) {
            // 清理已保存的临时文件
            for (const auto& path : savedImagePaths) {
                std::remove(path.c_str());
            }
            sendErrorResponse(res, 400, "图片" + std::to_string(i+1) + "的data字段为空");
            return;
        }
        
        Logger::info("[CREATE POST] Processing image " + std::to_string(i + 1) + "/" + 
                   std::to_string(imagesArray.size()) + " - Filename: " + filename + 
                   ", ContentType: " + contentType + 
                   ", Base64Size: " + std::to_string(base64Data.size()) + " bytes");
        
        // 验证Content-Type
        if (contentType.find("image/") != 0) {
            // 清理已保存的临时文件
            for (const auto& path : savedImagePaths) {
                std::remove(path.c_str());
            }
            sendErrorResponse(res, 400, "只能上传图片文件");
            return;
        }
        
        // 保存文件（saveUploadedFile会自动检测并解码Base64）
        std::string savedPath = saveUploadedFile(base64Data, filename, contentType);
        
        if (savedPath.empty()) {
            // 清理已保存的临时文件
            for (const auto& path : savedImagePaths) {
                std::remove(path.c_str());
            }
            Logger::error("[CREATE POST] Failed to save image " + std::to_string(i+1));
            sendErrorResponse(res, 500, "保存图片文件失败");
            return;
        }
        
        savedImagePaths.push_back(savedPath);
        Logger::info("[CREATE POST] Image " + std::to_string(i + 1) + " saved successfully");
    }
}

// 4. 验证图片数量（统一验证）
if (savedImagePaths.empty() || savedImagePaths.size() > 9) {
    for (const auto& path : savedImagePaths) {
        std::remove(path.c_str());
    }
    Logger::warning("[CREATE POST] Invalid image count: " + std::to_string(savedImagePaths.size()));
    sendErrorResponse(res, 400, "图片数量必须在1-9张之间");
    return;
}

// 5. 调用Service创建帖子（现有逻辑保持不变）
// ... 后续代码不变 ...
```

#### 任务1.3: 验证saveUploadedFile的Base64支持 ✅

**文件**: `src/api/post_handler.cpp`  
**行数**: 654-781

**检查项**:
- ✅ Base64Decoder::isBase64() 检测逻辑
- ✅ Base64Decoder::decode() 解码逻辑
- ✅ 支持 `data:image/jpeg;base64,xxx` 格式
- ✅ 支持纯Base64格式
- ✅ 图片格式验证（PNG/JPEG签名）
- ✅ 完整的日志记录

**结论**: 无需修改，现有逻辑完善。

---

### 阶段2: 编译和部署

#### 任务2.1: 编译项目 ⏳

```bash
cd /home/kun/projects/SharePix/backend-service
rm -rf build && mkdir build
cd build
cmake ..
make -j4
```

#### 任务2.2: 检查编译结果 ⏳

- 检查是否有编译错误
- 检查是否有警告
- 验证可执行文件生成

#### 任务2.3: 重启服务 ⏳

```bash
# 停止旧服务
pkill -f knot_image_sharing

# 启动新服务
cd /home/kun/projects/SharePix/backend-service
nohup ./build/knot_image_sharing config/config.json > server.log 2>&1 &
```

---

### 阶段3: 功能测试

#### 任务3.1: JSON格式测试 ⏳

**测试用例1: 单图片上传（JSON+Base64）**

```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "JSON格式测试帖子",
    "description": "使用JSON+Base64上传单张图片",
    "tags": ["测试", "JSON"],
    "images": [
      {
        "filename": "test.jpg",
        "content_type": "image/jpeg",
        "data": "data:image/jpeg;base64,/9j/4AAQSkZJRg..."
      }
    ]
  }'
```

**预期结果**:
- ✅ 返回200状态码
- ✅ 返回帖子详情（包含image_count=1）
- ✅ 图片成功保存到服务器
- ✅ 日志显示Base64解码成功

**测试用例2: 多图片上传（JSON+Base64）**

```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "多图片JSON测试",
    "description": "使用JSON上传3张图片",
    "tags": ["测试"],
    "images": [
      {"filename": "1.jpg", "content_type": "image/jpeg", "data": "..."},
      {"filename": "2.png", "content_type": "image/png", "data": "..."},
      {"filename": "3.jpg", "content_type": "image/jpeg", "data": "..."}
    ]
  }'
```

**预期结果**:
- ✅ 返回200状态码
- ✅ image_count=3
- ✅ 所有图片按顺序保存

#### 任务3.2: 向后兼容性测试 ⏳

**测试用例3: multipart/form-data格式（保证不破坏现有功能）**

```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -F "title=Multipart测试" \
  -F "description=验证向后兼容性" \
  -F "tags=测试" \
  -F "imageFiles=@test.jpg"
```

**预期结果**:
- ✅ 功能正常，与修改前一致
- ✅ 返回200状态码
- ✅ 图片成功上传

#### 任务3.3: 异常情况测试 ⏳

**测试用例4: 无效的Content-Type**

```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: text/plain" \
  -d "invalid data"
```

**预期结果**:
- ✅ 返回400错误
- ✅ 错误消息: "请求必须使用multipart/form-data或application/json格式"

**测试用例5: JSON格式错误**

```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{invalid json'
```

**预期结果**:
- ✅ 返回400错误
- ✅ 错误消息: "无效的JSON格式"

**测试用例6: 缺少images字段**

```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "缺少图片",
    "description": "测试缺少images字段"
  }'
```

**预期结果**:
- ✅ 返回400错误
- ✅ 错误消息: "缺少images字段"

**测试用例7: 图片数量超限**

```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "图片过多",
    "images": [/* 10张图片 */]
  }'
```

**预期结果**:
- ✅ 返回400错误
- ✅ 错误消息: "图片数量必须在1-9张之间"

**测试用例8: Base64数据为空**

```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "空数据",
    "images": [{"filename": "test.jpg", "data": ""}]
  }'
```

**预期结果**:
- ✅ 返回400错误
- ✅ 错误消息包含"data字段为空"

**测试用例9: 无效的Base64数据**

```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -H "Content-Type: application/json" \
  -d '{
    "title": "无效Base64",
    "images": [{"filename": "test.jpg", "data": "invalid!!!base64"}]
  }'
```

**预期结果**:
- ✅ 返回500错误
- ✅ 错误消息: "保存图片文件失败"
- ✅ 日志显示Base64解码失败

---

### 阶段4: 日志验证

#### 任务4.1: 检查日志输出 ⏳

**期望日志示例（JSON格式成功）**:

```log
[INFO] === [CREATE POST] Request received ===
[INFO] [CREATE POST] User authenticated - UserID: 5
[INFO] [CREATE POST] Request format: application/json
[INFO] [CREATE POST] Processing 2 images from JSON
[INFO] [CREATE POST] Processing image 1/2 - Filename: test1.jpg, ContentType: image/jpeg, Base64Size: 12345 bytes
[INFO] [SAVE FILE] Starting to process file - Name: test1.jpg, Type: image/jpeg, OriginalSize: 12345 bytes
[INFO] [SAVE FILE] ⚠ Base64 encoded data detected!
[INFO] [SAVE FILE]   → Data format: Data URI
[INFO] [SAVE FILE]   → Encoded size: 12345 bytes
[INFO] [SAVE FILE] ✓ Base64 decode successful!
[INFO] [SAVE FILE]   → Decoded size: 9133 bytes
[INFO] [SAVE FILE]   → Size reduction: 26%
[INFO] [SAVE FILE]   → Decode time: 5 ms
[INFO] [SAVE FILE]   → Image format verified: JPEG
[INFO] [SAVE FILE] ✓ File saved successfully!
[INFO] [CREATE POST] Image 1 saved successfully
[INFO] [CREATE POST] Processing image 2/2 - ...
[INFO] [CREATE POST] All images validated successfully, total: 2
[INFO] [CREATE POST] Creating post in database...
[INFO] [CREATE POST] ✓ Post created successfully - PostID: POST_2025Q4_xxx, UserID: 5, Images: 2
```

**期望日志示例（multipart格式成功，向后兼容）**:

```log
[INFO] === [CREATE POST] Request received ===
[INFO] [CREATE POST] User authenticated - UserID: 5
[INFO] [CREATE POST] Request format: multipart/form-data
[INFO] [CREATE POST] Form data - Title: 'Multipart测试', Tags: 1
[DEBUG] [CREATE POST] Debugging form fields: ...
[INFO] [CREATE POST] Received 1 image file(s)
[INFO] [CREATE POST] Processing image 1/1 - Filename: test.jpg, ContentType: image/jpeg, Size: 9133 bytes
[INFO] [SAVE FILE] Binary data detected (not Base64)
[INFO] [SAVE FILE]   → Processing as direct binary upload
[INFO] [SAVE FILE] ✓ File saved successfully!
[INFO] [CREATE POST] ✓ Post created successfully - ...
```

---

### 阶段5: 文档更新

#### 任务5.1: 更新API文档 ⏳

**文件**: `project_document/[000]API文档.md`

**更新内容**:
- 创建帖子接口说明（第52-XX行）
- 添加JSON格式请求示例
- 说明两种格式都支持
- 更新请求参数表格

**新增内容示例**:

```markdown
### 1. 创建帖子

**功能介绍**: 创建新帖子，支持1-9张图片

**请求方式**: `POST`

**请求路径**: `/api/v1/posts`

**支持的Content-Type**:
- ✅ `multipart/form-data` - 适用于Web浏览器、原生应用
- ✅ `application/json` - 适用于API调用、Base64图片上传 **🆕 v2.4.1**

---

#### 格式1: multipart/form-data（推荐用于文件上传）

**请求头**:
```
Authorization: Bearer <access_token>
Content-Type: multipart/form-data
```

**请求参数**:
| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| title | string | 是 | 帖子标题（不超过200字符） |
| description | string | 否 | 帖子描述 |
| tags | string[] | 否 | 标签数组（可选） |
| imageFiles | file[] | 是 | 图片文件（1-9张，每张最大5MB） |

**请求示例**:
```bash
curl -X POST "http://localhost:8080/api/v1/posts" \
  -H "Authorization: Bearer <token>" \
  -F "title=美丽的风景" \
  -F "description=今天拍的照片" \
  -F "tags=风景" \
  -F "tags=旅行" \
  -F "imageFiles=@photo1.jpg" \
  -F "imageFiles=@photo2.jpg"
```

---

#### 格式2: application/json（适用于Base64图片） **🆕 v2.4.1**

**请求头**:
```
Authorization: Bearer <access_token>
Content-Type: application/json
```

**请求参数**:
| 参数名 | 类型 | 必填 | 说明 |
|--------|------|------|------|
| title | string | 是 | 帖子标题 |
| description | string | 否 | 帖子描述 |
| tags | string[] | 否 | 标签数组 |
| images | object[] | 是 | 图片数组（1-9张） |
| images[].filename | string | 否 | 文件名（默认: image.jpg） |
| images[].content_type | string | 否 | MIME类型（默认: image/jpeg） |
| images[].data | string | 是 | Base64编码的图片数据 |

**Base64数据格式**:
- 支持Data URI格式: `data:image/jpeg;base64,/9j/4AAQSkZJRg...`
- 支持纯Base64格式: `/9j/4AAQSkZJRg...`

**请求示例**:
```json
{
  "title": "美丽的风景",
  "description": "今天拍的照片",
  "tags": ["风景", "旅行"],
  "images": [
    {
      "filename": "photo1.jpg",
      "content_type": "image/jpeg",
      "data": "data:image/jpeg;base64,/9j/4AAQSkZJRgABAQEASABIAAD..."
    },
    {
      "filename": "photo2.png",
      "content_type": "image/png",
      "data": "iVBORw0KGgoAAAANSUhEUgAA..."
    }
  ]
}
```

---

**返回参数**: （两种格式返回相同）
...
```

#### 任务5.2: 更新CLAUDE.md ⏳

**文件**: `CLAUDE.md`

**更新内容**:
- API版本历史（v2.4.1）
- 添加JSON+Base64支持说明

#### 任务5.3: 更新README.md ⏳

**文件**: `/backend-service/README.md`

**更新内容**:
- 版本号更新为v2.4.1
- 功能特性中添加JSON+Base64支持

---

## 测试计划

### 单元测试

**测试文件**: `/home/kun/projects/SharePix/backend-service/test/test_json_post.cpp`

**测试内容**:
1. ✅ JSON解析正确性
2. ✅ Base64解码正确性
3. ✅ 图片数量验证
4. ✅ 字段验证（title, images等）
5. ✅ 错误处理

### 集成测试

**使用工具**: Apifox / curl

**测试用例**: 见[阶段3: 功能测试](#阶段3-功能测试)

### 性能测试

**测试指标**:
- Base64解码时间
- 整体请求响应时间
- 内存占用情况

**测试方法**:
```bash
# 并发测试（JSON格式）
ab -n 100 -c 10 -H "Authorization: Bearer <token>" \
   -H "Content-Type: application/json" \
   -p post_data.json \
   http://localhost:8080/api/v1/posts
```

---

## 风险评估

### 技术风险

| 风险项 | 风险等级 | 影响 | 应对措施 |
|--------|---------|------|---------|
| Base64解码性能 | 🟡 中 | 大图片解码可能耗时较长 | 已有性能日志监控 |
| 内存占用增加 | 🟡 中 | Base64数据在内存中占用更多空间 | 限制单张图片5MB |
| 向后兼容性破坏 | 🟢 低 | 可能影响现有multipart客户端 | 两种格式都支持，不破坏现有功能 |
| JSON解析错误 | 🟢 低 | 无效JSON导致服务异常 | 完善的错误处理和日志 |

### 业务风险

| 风险项 | 风险等级 | 影响 | 应对措施 |
|--------|---------|------|---------|
| 客户端不兼容 | 🟢 低 | 部分客户端可能不支持Base64 | 提供两种格式供选择 |
| 文档不同步 | 🟡 中 | 开发者理解错误 | 及时更新API文档 |

---

## 实施进度

### 进度跟踪

| 阶段 | 任务 | 状态 | 完成时间 | 备注 |
|------|------|------|---------|------|
| **阶段1: 代码修改** | | | | |
| 1.1 | 修改Content-Type检查逻辑 | ✅ 已完成 | 2025-10-15 | 支持multipart和JSON |
| 1.2 | 添加JSON格式解析分支 | ✅ 已完成 | 2025-10-15 | 完整的Base64处理逻辑 |
| 1.3 | 验证saveUploadedFile的Base64支持 | ✅ 已完成 | 2025-10-14 | 无需修改 |
| **阶段2: 编译和部署** | | | | |
| 2.1 | 编译项目 | ✅ 已完成 | 2025-10-15 | 编译成功无错误 |
| 2.2 | 检查编译结果 | ✅ 已完成 | 2025-10-15 | 无警告 |
| 2.3 | 重启服务 | ✅ 已完成 | 2025-10-15 | 服务正常运行 |
| **阶段3: 功能测试** | | | | |
| 3.1 | JSON格式测试 | ✅ 已完成 | 2025-10-15 | 单图片和多图片均测试通过 |
| 3.2 | 向后兼容性测试 | ✅ 已完成 | 2025-10-15 | multipart格式正常工作 |
| 3.3 | 异常情况测试 | ✅ 已完成 | 2025-10-15 | 错误处理符合预期 |
| **阶段4: 日志验证** | | | | |
| 4.1 | 检查日志输出 | ✅ 已完成 | 2025-10-15 | Base64解码日志完整 |
| **阶段5: 文档更新** | | | | |
| 5.1 | 更新API文档 | ✅ 已完成 | 2025-10-15 | 添加JSON格式说明 |
| 5.2 | 更新CLAUDE.md | ✅ 已完成 | 2025-10-15 | 添加v2.4.1版本记录 |
| 5.3 | 更新README.md | ✅ 已完成 | 2025-10-15 | 更新版本和功能列表 |

### 里程碑

- [x] **Milestone 1**: 代码修改完成 (完成: 2025-10-15)
- [x] **Milestone 2**: 功能测试通过 (完成: 2025-10-15)
- [x] **Milestone 3**: 文档更新完成 (完成: 2025-10-15)
- [x] **Milestone 4**: 版本发布 v2.4.1 (完成: 2025-10-15)

---

## 关键决策记录

### 决策1: 选择方案A而非方案B

**时间**: 2025-10-14  
**决策者**: 开发团队  
**理由**:
1. Base64解码逻辑已实现，无需重复开发
2. 前端代码不用改，减少工作量
3. 向后兼容，不影响现有客户端
4. 符合RESTful规范

### 决策2: 同时支持两种格式而非替换

**时间**: 2025-10-14  
**决策者**: 开发团队  
**理由**:
1. 保证向后兼容性
2. 给客户端更多选择
3. multipart更适合大文件，JSON更适合API调用

### 决策3: 保持图片大小限制5MB不变

**时间**: 2025-10-14  
**决策者**: 开发团队  
**理由**:
1. Base64编码后约6.65MB，仍在可接受范围
2. 避免过大请求影响性能
3. 建议前端压缩图片

---

## 参考资料

### 相关文档

- `[112]Base64图片上传问题分析与解决方案.md` - Base64问题分析
- `[113]日志监控指南-图片上传排查手册.md` - 日志监控指南
- `[114]多图片功能修复和实现报告.md` - 多图片功能修复
- `[000]API文档.md` - API接口规范

### 相关代码

- `src/api/post_handler.cpp` - 帖子API处理器
- `src/utils/base64_decoder.{h,cpp}` - Base64解码工具
- `src/core/post_service.cpp` - 帖子业务逻辑

---

## 附录

### 附录A: Base64编码原理

Base64是一种用64个字符（A-Z, a-z, 0-9, +, /）表示二进制数据的编码方式。

**编码过程**:
1. 将3个字节（24位）拆分为4组，每组6位
2. 每组6位转换为0-63的数字
3. 根据Base64字符表转换为对应字符

**大小变化**: 编码后数据大小约为原始数据的 **133.33%**（增加约33%）

**示例**:
```
原始数据: Hello (5字节 = 40位)
二进制:   01001000 01100101 01101100 01101100 01101111
Base64:   SGVsbG8= (8字符)
```

### 附录B: 支持的图片格式

| 格式 | MIME类型 | 文件签名 | 是否支持 |
|------|---------|---------|---------|
| JPEG | image/jpeg | FF D8 FF | ✅ 是 |
| PNG | image/png | 89 50 4E 47 0D 0A 1A 0A | ✅ 是 |
| GIF | image/gif | 47 49 46 38 | ✅ 是 |
| WebP | image/webp | 52 49 46 46 ... 57 45 42 50 | ✅ 是 |

### 附录C: 错误码说明

| HTTP状态码 | 错误消息 | 原因 | 解决方法 |
|-----------|---------|------|---------|
| 400 | 请求必须使用multipart/form-data或application/json格式 | Content-Type不支持 | 使用正确的Content-Type |
| 400 | 无效的JSON格式 | JSON解析失败 | 检查JSON格式是否正确 |
| 400 | 标题不能为空 | 缺少title字段 | 添加title字段 |
| 400 | 缺少images字段 | JSON中没有images | 添加images数组 |
| 400 | 图片数量必须在1-9张之间 | 图片数量不符合要求 | 调整图片数量 |
| 400 | 图片X的data字段为空 | Base64数据为空 | 检查图片数据 |
| 400 | 只能上传图片文件 | content_type不是image/* | 使用正确的MIME类型 |
| 500 | 保存图片文件失败 | Base64解码或文件保存失败 | 检查Base64数据是否有效 |

---

**文档结束**

**下一步行动**: 开始执行阶段1的代码修改任务

