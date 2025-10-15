# 用户信息更新接口BUG修复总结

## 🐛 发现的问题

你报告的三个问题：
1. ❌ 上传头像URL会错误返回"手机号格式不对"
2. ❌ bio（个人简介）、gender（性别）、location（位置）字段无法更新
3. ❌ 即使数据库有这些字段，发送请求也无法更改，一直为空

## ✅ 修复完成

### 问题根源

发现了**三个关键BUG**：

#### BUG 1: Handler层参数顺序错误
```cpp
// 错误：avatar_url 和 phone 位置颠倒
authService_->updateUserProfile(
    userId, realName, email, avatarUrl, phone, ...
);

// 正确：
authService_->updateUserProfile(
    userId, realName, email, phone, avatarUrl, ...
);
```
**影响**：avatar_url被当作phone验证，导致"手机号格式不对"错误

#### BUG 2: Service层调用了错误的Repository方法
```cpp
// 错误：调用 updateUser()，SQL不包含bio/gender/location
userRepo_->updateUser(*existingUser);

// 正确：调用 updateUserProfile()，包含所有字段
userRepo_->updateUserProfile(userId, realName, email, phone, 
                            avatarUrl, bio, gender, location);
```
**影响**：bio、gender、location永远无法更新

#### BUG 3: Repository层参数顺序与Service层不一致
**影响**：如果只修复BUG2，会导致phone和avatar_url值互换

### 修复的文件

| 文件 | 修改内容 |
|------|---------|
| `src/api/auth_handler.cpp` | 修正参数传递顺序 |
| `src/core/auth_service.cpp` | 调用正确的Repository方法 |
| `src/database/user_repository.h` | 统一参数顺序 |
| `src/database/user_repository.cpp` | 调整SQL和参数绑定 |

## 🧪 如何测试

### 方式1：使用测试脚本（推荐）

```bash
cd /home/kun/projects/SharePix/backend-service/test
./compile_and_test_fix.sh
```

### 方式2：使用Apifox或curl

**步骤1: 登录获取token**
```bash
curl -X POST http://localhost:8080/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser001",
    "password": "Test123456"
  }'
```

**步骤2: 测试更新用户信息**
```bash
curl -X PUT http://localhost:8080/api/v1/users/profile \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer <你的token>" \
  -d '{
    "real_name": "测试用户",
    "phone": "13900000001",
    "email": "test@example.com",
    "avatar_url": "https://example.com/avatar.jpg",
    "bio": "这是我的个人简介",
    "gender": "male",
    "location": "北京市朝阳区"
  }'
```

**预期结果**：
- ✅ 返回 `success: true`
- ✅ bio、gender、location 字段已更新
- ✅ avatar_url 正确保存，不再报"手机号格式不对"
- ✅ phone 字段正确更新

**步骤3: 验证更新结果**
```bash
curl -X GET http://localhost:8080/api/v1/users/profile \
  -H "Authorization: Bearer <你的token>"
```

检查返回的数据中：
- `bio` 应为 "这是我的个人简介"
- `gender` 应为 "male"
- `location` 应为 "北京市朝阳区"
- `avatar_url` 应包含 "avatar.jpg"

## 📊 修复前后对比

| 字段 | 修复前 | 修复后 |
|-----|-------|-------|
| avatar_url | ❌ 被当作phone验证，报错 | ✅ 正常保存 |
| bio | ❌ 无法更新 | ✅ 正常更新 |
| gender | ❌ 无法更新 | ✅ 正常更新 |
| location | ❌ 无法更新 | ✅ 正常更新 |
| phone | ❌ 与avatar_url位置错乱 | ✅ 正常更新 |

## 📝 数据库验证

数据库表 `users` 已经包含所有字段：
```sql
avatar_url VARCHAR(255) NULL        -- ✓ 存在
bio VARCHAR(500) NULL                -- ✓ 存在
gender ENUM('male', 'female', ...) NULL  -- ✓ 存在
location VARCHAR(100) NULL           -- ✓ 存在
```

之前无法更新是因为**代码BUG**，不是数据库问题。

## 🔧 如何重新编译服务

```bash
cd /home/kun/projects/SharePix/backend-service
rm -rf build && mkdir build
cd build
cmake ..
make -j4

# 运行服务
cd ..
./build/knot_image_sharing config/config.json
```

## 📖 详细文档

完整的BUG分析和修复报告见：
- `project_document/[107]用户信息更新接口BUG修复报告.md`

该文档包含：
- 详细的问题分析
- 代码对比
- 修复方案说明
- 测试用例
- 后续改进建议

## ✨ 修复状态

- ✅ 代码已修复
- ✅ 编译无错误
- ✅ 测试脚本已创建
- ⏳ 等待你的测试验证

---

**修复时间**: 2025-10-15  
**修复版本**: v2.1.1  
**影响范围**: 用户信息管理模块


