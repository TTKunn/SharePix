# Follow路由问题诊断方案

**问题ID**: FOLLOW-ROUTE-001  
**严重级别**: 🔴 高危  
**影响**: POST `/api/v1/users/:user_id/follow` 完全无法工作  

---

## 问题现象

### 症状
- ❌ POST `/api/v1/users/31/follow` → 400 Bad Request, 耗时5秒, 无响应体
- ✅ GET `/api/v1/users/28/followers` → 200 OK, 正常工作
- ✅ GET `/api/v1/users/28` → 200 OK, 正则路由正常工作
- ❌ Lambda函数完全未被调用（日志未出现）

### 日志证据
```
[13:49:59.635] FollowHandler: Registering POST /api/v1/users/:user_id/follow  ✅
[13:50:19.488] Request: POST /api/v1/users/31/follow                           ✅
[13:50:24.488] Response: 400 for POST /api/v1/users/31/follow                  ❌
（无Lambda内部日志）
```

---

## 环境信息

- **cpp-httplib版本**: 0.26.0（非预期的0.11.0）
- **超时设置**: 
  - `CPPHTTPLIB_KEEPALIVE_TIMEOUT_SECOND = 5`
  - `CPPHTTPLIB_SERVER_READ_TIMEOUT_SECOND = 5`

---

## 可能原因分析

### 原因1: 路由冲突 - 正则路由干扰
**可能性**: ⭐⭐⭐⭐☆ (80%)

**理论**:
1. AuthHandler注册正则路由: `GET R"(/api/v1/users/([^/]+)$)"`
2. 虽然添加了`$`锚点，但cpp-httplib可能在路由表层面优先检查正则路由
3. 当POST请求`/api/v1/users/31/follow`到达时：
   - 正则引擎可能先检查路径前缀`/api/v1/users/31`
   - 发现路径继续有`/follow`，与`$`锚点不匹配
   - 但已经标记该路径树为"正则匹配域"
   - 导致后续的普通路径参数路由被跳过
4. 最终找不到匹配的路由，返回400

**证据**:
- 正则路由注册在最后（第7步）
- 所有POST `/api/v1/users/:user_id/*` 路由都注册在之前（第5步）
- GET路由正常工作可能是因为HTTP方法不同的匹配逻辑

**验证方法**:
```bash
# 1. 完全注释掉AuthHandler::registerWildcardRoutes()
# 2. 重新编译并测试
# 3. 如果POST正常工作，则证明是正则路由干扰
```

---

### 原因2: 路径参数命名冲突
**可能性**: ⭐⭐☆☆☆ (40%)

**理论**:
多个Handler使用了`/api/v1/users/:xxx/*`路径：
- FollowHandler: `:user_id`
- PostHandler: `:user_id` 
- ImageHandler: `:id`

可能存在路径参数名称或路由表内部冲突。

**证据**:
- PostHandler的 `GET /api/v1/users/:user_id/posts` 能工作
- 说明路径参数名称本身不是问题

**验证方法**:
```bash
# 1. 将Follow路由改为 /api/v1/follow/:user_id
# 2. 测试是否工作
```

---

### 原因3: HTTP方法注册bug
**可能性**: ⭐⭐⭐☆☆ (60%)

**理论**:
cpp-httplib 0.26.0可能存在POST方法与路径参数结合的bug。

**证据**:
- LikeHandler的 `POST /api/v1/posts/:post_id/like` 能工作
- FollowHandler的 `POST /api/v1/users/:user_id/follow` 不工作
- 区别：路径前缀不同（`/posts/` vs `/users/`）

**验证方法**:
```bash
# 测试简单POST路由
server.Post("/api/v1/testpost", [](...) { res.set_content("OK", "text/plain"); });
server.Post("/api/v1/users/testpost", [](...) { res.set_content("OK", "text/plain"); });
```

---

### 原因4: 路由注册顺序问题
**可能性**: ⭐⭐☆☆☆ (40%)

**理论**:
AuthHandler先注册了其他`/api/v1/users/*`路由，占据了路径树的根节点。

**当前注册顺序**:
```
1. AuthHandler::registerRoutes()
   - GET /api/v1/users/profile
   - PUT /api/v1/users/profile
   - GET /api/v1/users/check-username

2. [其他Handler...]

3. FollowHandler::registerRoutes()
   - POST /api/v1/users/:user_id/follow

4. AuthHandler::registerWildcardRoutes()
   - GET R"(/api/v1/users/([^/]+)$)"
```

**验证方法**:
```bash
# 将FollowHandler的注册移到AuthHandler之前
# 或者创建独立的 /api/v1/follow/ 路径
```

---

## 建议的修复方案

### 方案A: 移除或限制正则路由（推荐）⭐⭐⭐⭐⭐

**操作**:
1. 注释掉 `http_server.cpp` 第147行: 
   ```cpp
   // authHandler_->registerWildcardRoutes(*server_);
   ```

2. 将获取用户公开信息的功能改为精确路由：
   ```cpp
   // 在 auth_handler.cpp 的 registerRoutes() 中
   server.Get("/api/v1/users/:user_id/info", [this](...) {
       handleGetUserPublicInfo(...);
   });
   ```

3. 更新API文档和客户端

**优点**:
- 彻底解决路由冲突问题
- 路由逻辑更清晰
- 性能更好（无需正则匹配）

**缺点**:
- 需要修改现有API路径
- 需要更新客户端代码

---

### 方案B: 调整路由注册顺序⭐⭐⭐☆☆

**操作**:
1. 在 `http_server.cpp` 中将FollowHandler移到AuthHandler之前：
   ```cpp
   void HttpServer::setupRoutes() {
       followHandler_->registerRoutes(*server_);  // ← 提前
       authHandler_->registerRoutes(*server_);
       imageHandler_->registerRoutes(*server_);
       // ...
       authHandler_->registerWildcardRoutes(*server_);
   }
   ```

**优点**:
- 不需要修改API路径
- 改动最小

**缺点**:
- 可能只是临时解决
- 可能引入新的冲突

---

### 方案C: 更改Follow路由路径前缀⭐⭐⭐⭐☆

**操作**:
将所有Follow相关路由从 `/api/v1/users/` 改为 `/api/v1/follow/`:

```cpp
// follow_handler.cpp
server.Post("/api/v1/follow/:user_id", [this](...) {
    handleFollow(req, res);
});

server.Delete("/api/v1/unfollow/:user_id", [this](...) {
    handleUnfollow(req, res);
});

server.Get("/api/v1/follow/:user_id/status", [this](...) {
    handleCheckFollowStatus(req, res);
});

// 列表查询仍然保留在 /api/v1/users/:user_id/following
server.Get("/api/v1/users/:user_id/following", [this](...) { ... });
server.Get("/api/v1/users/:user_id/followers", [this](...) { ... });
```

**优点**:
- 彻底避开冲突区域
- RESTful语义更清晰（`/follow/` vs `/users/`)
- 不影响现有GET路由

**缺点**:
- 需要修改客户端（但还未发布，影响小）

---

### 方案D: 升级或降级cpp-httplib版本⭐⭐☆☆☆

**操作**:
1. 测试其他版本的cpp-httplib（如0.15.0、0.20.0）
2. 查看是否存在已知的路由匹配bug

**优点**:
- 如果是库的bug，升级可以解决

**缺点**:
- 不确定性高
- 可能引入其他兼容性问题

---

## 推荐执行顺序

### 第一步: 快速验证（5分钟）
```bash
# 1. 注释掉正则路由
vim src/server/http_server.cpp
# 第147行改为: // authHandler_->registerWildcardRoutes(*server_);

# 2. 重新编译
cd build && make -j4

# 3. 重启服务并测试
LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH ./knot_image_sharing ../config/config.json &

# 4. 测试POST /follow
TOKEN="..."  # 填入你的token
curl -X POST http://localhost:8080/api/v1/users/31/follow \
     -H "Authorization: Bearer $TOKEN" \
     -v
```

**预期结果**:
- 如果返回200/201/409（业务响应），则证明是正则路由冲突
- 如果仍然返回400且耗时5秒，则继续下一步

---

### 第二步: 路径前缀方案（15分钟）
如果第一步验证失败，实施方案C：

```bash
# 1. 修改 follow_handler.cpp 的路由路径
# 2. 修改 [000]API文档.md
# 3. 编译测试
```

---

### 第三步: 深度调试（30分钟）
如果前两步都失败，进行深度调试：

```bash
# 1. 编译测试程序
g++ -std=c++17 test/test_follow_route_debug.cpp -lcurl -ljsoncpp -o test_follow

# 2. 运行测试
cd test
./test_follow

# 3. 分析测试结果，定位具体原因
```

---

## 测试检查清单

完成修复后，执行以下测试：

- [ ] POST `/api/v1/users/:user_id/follow` → 200/201
- [ ] DELETE `/api/v1/users/:user_id/follow` → 200
- [ ] GET `/api/v1/users/:user_id/follow/status` → 200
- [ ] GET `/api/v1/users/:user_id/following` → 200
- [ ] GET `/api/v1/users/:user_id/followers` → 200
- [ ] GET `/api/v1/users/:user_id/stats` → 200
- [ ] POST `/api/v1/users/follow/batch-status` → 200
- [ ] GET `/api/v1/users/:user_id` (用户公开信息) → 200

---

## 参考资料

- cpp-httplib文档: https://github.com/yhirose/cpp-httplib
- 测试问题汇总: `[122]关注功能测试问题汇总.md`
- 关注功能实现计划: `[108]阶段D-2-互动系统关注功能实现计划.md`


