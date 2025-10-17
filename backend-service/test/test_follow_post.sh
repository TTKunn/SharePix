#!/bin/bash
# 测试关注功能POST请求调试脚本
# 创建时间: 2025-10-17
# 用途: 详细测试POST /api/v1/users/:user_id/follow路由问题

echo "========================================="
echo "关注功能POST路由调试测试"
echo "========================================="
echo ""

# 配置
API_URL="http://localhost:8080"
TEST_USER="user_c"
TEST_PASSWORD="password123"
TARGET_USER_ID="28"

echo "步骤1: 登录获取token"
LOGIN_RESPONSE=$(curl -s -X POST \
  "${API_URL}/api/v1/auth/login" \
  -H "Content-Type: application/json" \
  -d "{\"username\":\"${TEST_USER}\",\"password\":\"${TEST_PASSWORD}\"}")

echo "登录响应: ${LOGIN_RESPONSE}"
echo ""

# 提取token
TOKEN=$(echo ${LOGIN_RESPONSE} | grep -o '"access_token":"[^"]*"' | cut -d'"' -f4)

if [ -z "$TOKEN" ]; then
    echo "❌ 登录失败，无法获取token"
    exit 1
fi

echo "✅ Token获取成功: ${TOKEN:0:50}..."
echo ""

echo "========================================="
echo "步骤2: 测试POST /api/v1/users/${TARGET_USER_ID}/follow"
echo "========================================="
echo ""

echo "发送时间: $(date '+%Y-%m-%d %H:%M:%S')"
START_TIME=$(date +%s)

# 使用-v显示详细信息
FOLLOW_RESPONSE=$(curl -v -X POST \
  "${API_URL}/api/v1/users/${TARGET_USER_ID}/follow" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  2>&1)

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo ""
echo "响应时间: $(date '+%Y-%m-%d %H:%M:%S')"
echo "耗时: ${ELAPSED}秒"
echo ""
echo "完整响应:"
echo "${FOLLOW_RESPONSE}"
echo ""

# 检查HTTP状态码
HTTP_CODE=$(echo "${FOLLOW_RESPONSE}" | grep "< HTTP" | awk '{print $3}')
echo "HTTP状态码: ${HTTP_CODE}"

if [ "$HTTP_CODE" = "200" ] || [ "$HTTP_CODE" = "201" ]; then
    echo "✅ 关注成功"
else
    echo "❌ 关注失败 (HTTP ${HTTP_CODE})"
fi

echo ""
echo "========================================="
echo "步骤3: 测试GET /api/v1/users/${TARGET_USER_ID}/followers（对比）"
echo "========================================="
echo ""

FOLLOWERS_RESPONSE=$(curl -s -X GET \
  "${API_URL}/api/v1/users/${TARGET_USER_ID}/followers" \
  -H "Authorization: Bearer ${TOKEN}")

echo "GET请求响应: ${FOLLOWERS_RESPONSE}"
echo ""

echo "========================================="
echo "测试完成"
echo "========================================="

