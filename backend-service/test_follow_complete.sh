#!/bin/bash
# 关注功能完整测试脚本

BASE_URL="http://localhost:8080"
REPORT_FILE="project_document/[121]关注功能完整测试报告.md"

# 创建测试报告
cat > "$REPORT_FILE" << 'REPORTHEADER'
# 关注功能完整测试报告

**测试日期**: 2025-10-17  
**测试环境**: 开发环境  
**服务地址**: http://localhost:8080  

---

## 📋 测试准备

### 1. 服务健康检查
REPORTHEADER

echo '```bash' >> "$REPORT_FILE"
echo 'curl http://localhost:8080/health' >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"
echo '**响应**:' >> "$REPORT_FILE"
echo '```json' >> "$REPORT_FILE"
curl -s http://localhost:8080/health | jq . >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
echo '✅ 服务正常运行' >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

# 注册用户
echo '### 2. 注册测试用户' >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

echo '#### 2.1 注册用户A' >> "$REPORT_FILE"
echo '```bash' >> "$REPORT_FILE"
echo 'curl -X POST http://localhost:8080/api/v1/auth/register ...' >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
echo '**响应**:' >> "$REPORT_FILE"
echo '```json' >> "$REPORT_FILE"
USER_A=$(curl -s -X POST $BASE_URL/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "ft_user_a",
    "email": "fta@test.com",
    "phone": "13901000001",
    "password": "Test123456",
    "real_name": "测试用户A"
  }')
echo "$USER_A" | jq . >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

echo '#### 2.2 注册用户B' >> "$REPORT_FILE"
echo '**响应**:' >> "$REPORT_FILE"
echo '```json' >> "$REPORT_FILE"
USER_B=$(curl -s -X POST $BASE_URL/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "ft_user_b",
    "email": "ftb@test.com",
    "phone": "13901000002",
    "password": "Test123456",
    "real_name": "测试用户B"
  }')
echo "$USER_B" | jq . >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

echo '#### 2.3 注册用户C' >> "$REPORT_FILE"
echo '**响应**:' >> "$REPORT_FILE"
echo '```json' >> "$REPORT_FILE"
USER_C=$(curl -s -X POST $BASE_URL/api/v1/auth/register \
  -H "Content-Type: application/json" \
  -d '{
    "username": "ft_user_c",
    "email": "ftc@test.com",
    "phone": "13901000003",
    "password": "Test123456",
    "real_name": "测试用户C"
  }')
echo "$USER_C" | jq . >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

# 登录获取Token
echo '### 3. 登录获取Token' >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

echo '#### 3.1 用户A登录' >> "$REPORT_FILE"
echo '**响应**:' >> "$REPORT_FILE"
echo '```json' >> "$REPORT_FILE"
LOGIN_A=$(curl -s -X POST $BASE_URL/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username": "ft_user_a", "password": "Test123456"}')
echo "$LOGIN_A" | jq . >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
TOKEN_A=$(echo "$LOGIN_A" | jq -r '.data.access_token')
USER_ID_A=$(echo "$LOGIN_A" | jq -r '.data.user_id')
echo "- Token: \`$TOKEN_A\`" >> "$REPORT_FILE"
echo "- User ID: \`$USER_ID_A\`" >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

echo '#### 3.2 用户B登录' >> "$REPORT_FILE"
echo '**响应**:' >> "$REPORT_FILE"
echo '```json' >> "$REPORT_FILE"
LOGIN_B=$(curl -s -X POST $BASE_URL/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username": "ft_user_b", "password": "Test123456"}')
echo "$LOGIN_B" | jq . >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
TOKEN_B=$(echo "$LOGIN_B" | jq -r '.data.access_token')
USER_ID_B=$(echo "$LOGIN_B" | jq -r '.data.user_id')
echo "- Token: \`$TOKEN_B\`" >> "$REPORT_FILE"
echo "- User ID: \`$USER_ID_B\`" >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

echo '#### 3.3 用户C登录' >> "$REPORT_FILE"
echo '**响应**:' >> "$REPORT_FILE"
echo '```json' >> "$REPORT_FILE"
LOGIN_C=$(curl -s -X POST $BASE_URL/api/v1/auth/login \
  -H "Content-Type: application/json" \
  -d '{"username": "ft_user_c", "password": "Test123456"}')
echo "$LOGIN_C" | jq . >> "$REPORT_FILE"
echo '```' >> "$REPORT_FILE"
TOKEN_C=$(echo "$LOGIN_C" | jq -r '.data.access_token')
USER_ID_C=$(echo "$LOGIN_C" | jq -r '.data.user_id')
echo "- Token: \`$TOKEN_C\`" >> "$REPORT_FILE"
echo "- User ID: \`$USER_ID_C\`" >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

echo '---' >> "$REPORT_FILE"
echo '' >> "$REPORT_FILE"

echo "测试准备完成！继续关注功能测试..." >> "$REPORT_FILE"
echo "User A: $USER_ID_A (Token: ${TOKEN_A:0:20}...)" 
echo "User B: $USER_ID_B (Token: ${TOKEN_B:0:20}...)"
echo "User C: $USER_ID_C (Token: ${TOKEN_C:0:20}...)"

