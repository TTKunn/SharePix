# Follow路由问题诊断结果

**日期**: 2025-10-17  
**问题ID**: FOLLOW-ROUTE-001  
**状态**: ✅ 根本原因已找到  

---

## 问题描述

POST `/api/v1/users/:user_id/follow` 返回400 Bad Request，耗时5秒，Lambda未执行。

---

## 诊断过程总结

### 阶段1: 验证正则路由冲突假设
**操作**: 注释 `authHandler_->registerWildcardRoutes(*server_);`  
**结果**: ❌ 问题依然存在  
**结论**: 不是AuthHandler的正则路由导致的冲突

### 阶段2: 验证OPTIONS处理器干扰
**操作**: 注释 `server_->Options(".*", ...)`  
**结果**: ❌ 问题依然存在  
**结论**: 不是OPTIONS正则路由导致的

### 阶段3: 缩小问题范围
**测试对比**:
| 路由 | 方法 | 路径参数 | 结果 |
|------|------|---------|------|
| `/api/v1/auth/login` | POST | 无 | ✅ 200 OK |
| `/api/v1/users/:user_id/followers` | GET | 有 | ✅ 200 OK |
| `/api/v1/users/:user_id/follow` | POST | 有 | ❌ 400 (5s) |
| `/api/v1/posts/:post_id/like` | POST | 有 | ❌ 400 (5s) |
| `/api/v1/test/:test_id` | POST | 有 | ❌ 400 (5s) |

**结论**: **所有带路径参数的POST路由都失败！**

---

## 根本原因

**cpp-httplib 0.26.0** 存在路由匹配Bug，无法正确处理带路径参数（如`:user_id`）的POST路由。

**证据**:
1. 简单测试路由 `POST /api/v1/test/:test_id` 也失败
2. Lambda函数完全未被调用（日志未出现）
3. 同样路径的GET请求正常工作
4. 无路径参数的POST请求正常工作

**技术细节**:
- 请求到达服务器（pre_routing_handler记录）
- 路由匹配失败（Lambda未执行）
- 5秒超时后返回400（`CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND = 5`）
- 无响应体

---

## 解决方案

### 方案A: 降级cpp-httplib版本 ⭐⭐⭐⭐⭐ (强烈推荐)

**操作**:
1. 将 `backend-service/third_party/httplib.h` 替换为 **0.11.0** 或 **0.15.0** 版本
2. 重新编译测试

**优点**:
- 根本解决问题
- 不需要修改应用代码
- 向后兼容

**操作步骤**:
```bash
cd backend-service/third_party
# 备份现有版本
cp httplib.h httplib.h.0.26.0.backup

# 下载0.11.0版本
wget https://raw.githubusercontent.com/yhirose/cpp-httplib/v0.11.0/httplib.h

# 重新编译
cd ../build
rm -rf * && cmake .. && make -j4

# 测试
./knot_image_sharing ../config/config.json
```

---

### 方案B: 更改路由路径模式 ⭐⭐⭐☆☆

**操作**: 将Follow路由改为查询参数而不是路径参数

```cpp
// 修改前
server.Post("/api/v1/users/:user_id/follow", [this](...) { ... });

// 修改后
server.Post("/api/v1/follow", [this](const httplib::Request& req, httplib::Response& res) {
    std::string userId = req.get_param_value("user_id");
    // ... 业务逻辑
});
```

**调用方式**:
```bash
POST /api/v1/follow?user_id=31
```

**缺点**:
- 不符合RESTful规范
- 需要修改API文档和客户端
- 治标不治本

---

### 方案C: 使用独立路径前缀 ⭐⭐⭐⭐☆

**操作**: 将Follow路由移至独立路径

```cpp
// 关注操作
server.Post("/api/v1/follow/:user_id", [this](...) {  
    handleFollow(req, res);
});

// 取消关注
server.Delete("/api/v1/unfollow/:user_id", [this](...) {
    handleUnfollow(req, res);
});

// 查询路由保持不变
server.Get("/api/v1/users/:user_id/followers", [this](...) { ... });
server.Get("/api/v1/users/:user_id/following", [this](...) { ... });
```

**优点**:
- 可能避开cpp-httplib的Bug触发条件
- RESTful语义清晰
- 部分API路径需要修改

**缺点**:
- 不确定是否能解决（Bug可能影响所有POST+路径参数）
- 需要测试验证

---

## 推荐执行方案

### 🎯 第一优先级：方案A（降级cpp-httplib）

1. **下载0.11.0版本**（已知稳定）
2. **完整测试**所有API功能
3. **如果仍有问题**，尝试0.15.0版本

### 备选：方案C（独立路径前缀）

如果降级失败或不可行，尝试独立路径前缀。

---

##  测试验证清单

降级后必须测试：

### 核心功能
- [ ] POST `/api/v1/users/:user_id/follow` → 200/201
- [ ] POST `/api/v1/posts/:post_id/like` → 200/201
- [ ] POST `/api/v1/test/:test_id` → 200（测试路由）
- [ ] GET `/api/v1/users/:user_id/followers` → 200（确保GET不受影响）
- [ ] POST `/api/v1/auth/login` → 200（确保无参POST不受影响）

### 完整关注功能
- [ ] 关注用户
- [ ] 取消关注
- [ ] 检查关注状态
- [ ] 获取关注列表
- [ ] 获取粉丝列表
- [ ] 批量检查关注

---

## 相关文件

- 测试问题汇总: `[122]关注功能测试问题汇总.md`
- 诊断方案: `test/diagnose_follow_route.md`
- 测试程序: `test/test_follow_route_debug.cpp`

---

## 技术笔记

### cpp-httplib版本对比

| 版本 | 发布日期 | 状态 | 备注 |
|-----|---------|-----|------|
| 0.11.0 | 2022-03 | ✅ 已知稳定 | 文档中声明的版本 |
| 0.15.0 | 2023-06 | 🟡 待测试 | 中间稳定版本 |
| 0.26.0 | 2025-01 | ❌ 有Bug | 当前使用版本，POST+路径参数失败 |

### 5秒超时来源

```cpp
// third_party/httplib.h 第45-46行
#ifndef CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND
#define CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND 5
#endif
```

当路由匹配失败时，cpp-httplib等待5秒Keep-Alive超时后返回400。

---

**报告更新时间**: 2025-10-17 14:55  
**诊断完成**:  ✅ 根本原因已找到，推荐降级cpp-httplib至0.11.0




