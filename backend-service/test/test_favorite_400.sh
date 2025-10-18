#!/bin/bash

# 测试文件: test_favorite_400.sh
# 测试目的: 诊断前端收藏接口返回400的问题
# 创建时间: 2025-10-18
# 测试完成后删除

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 服务器地址
BASE_URL="http://localhost:8080"

# 测试用的JWT令牌（需要替换为真实令牌）
# 可以通过登录接口获取
echo -e "${YELLOW}步骤1: 获取JWT令牌${NC}"
LOGIN_RESPONSE=$(curl -s -X POST "${BASE_URL}/api/v1/auth/login" \
  -H "Content-Type: application/json" \
  -d '{
    "email": "test@example.com",
    "password": "Test123456"
  }')

TOKEN=$(echo $LOGIN_RESPONSE | grep -o '"access_token":"[^"]*' | cut -d'"' -f4)

if [ -z "$TOKEN" ]; then
  echo -e "${RED}❌ 获取令牌失败，请确保测试用户存在${NC}"
  echo "响应: $LOGIN_RESPONSE"
  exit 1
fi

echo -e "${GREEN}✓ 令牌获取成功${NC}"
echo "Token: ${TOKEN:0:50}..."

# 测试用的帖子ID
POST_ID="POST_2025Q4_8qYFhp"

echo -e "\n${YELLOW}======================================${NC}"
echo -e "${YELLOW}开始测试各种请求格式${NC}"
echo -e "${YELLOW}======================================${NC}"

# 测试1: 正确的请求（无请求体）
echo -e "\n${YELLOW}测试1: 正确的请求（无Content-Type，无请求体）${NC}"
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "${BASE_URL}/api/v1/posts/${POST_ID}/favorite" \
  -H "Authorization: Bearer ${TOKEN}")
echo "$RESPONSE"
HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE" | cut -d':' -f2)
if [ "$HTTP_CODE" = "200" ] || [ "$HTTP_CODE" = "409" ]; then
  echo -e "${GREEN}✓ 测试1通过 (状态码: $HTTP_CODE)${NC}"
else
  echo -e "${RED}❌ 测试1失败 (状态码: $HTTP_CODE)${NC}"
fi

# 测试2: 带Content-Type但无请求体
echo -e "\n${YELLOW}测试2: 带Content-Type: application/json，但无请求体${NC}"
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "${BASE_URL}/api/v1/posts/${POST_ID}/favorite" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json")
echo "$RESPONSE"
HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE" | cut -d':' -f2)
if [ "$HTTP_CODE" = "200" ] || [ "$HTTP_CODE" = "409" ]; then
  echo -e "${GREEN}✓ 测试2通过 (状态码: $HTTP_CODE)${NC}"
else
  echo -e "${RED}❌ 测试2失败 (状态码: $HTTP_CODE) - 这可能是问题所在！${NC}"
fi

# 测试3: 带Content-Type和空JSON请求体
echo -e "\n${YELLOW}测试3: 带Content-Type: application/json 和空JSON请求体 {}${NC}"
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "${BASE_URL}/api/v1/posts/${POST_ID}/favorite" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{}')
echo "$RESPONSE"
HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE" | cut -d':' -f2)
if [ "$HTTP_CODE" = "200" ] || [ "$HTTP_CODE" = "409" ]; then
  echo -e "${GREEN}✓ 测试3通过 (状态码: $HTTP_CODE)${NC}"
else
  echo -e "${RED}❌ 测试3失败 (状态码: $HTTP_CODE)${NC}"
fi

# 测试4: 带Content-Length: 0
echo -e "\n${YELLOW}测试4: 带Content-Length: 0${NC}"
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "${BASE_URL}/api/v1/posts/${POST_ID}/favorite" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Length: 0")
echo "$RESPONSE"
HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE" | cut -d':' -f2)
if [ "$HTTP_CODE" = "200" ] || [ "$HTTP_CODE" = "409" ]; then
  echo -e "${GREEN}✓ 测试4通过 (状态码: $HTTP_CODE)${NC}"
else
  echo -e "${RED}❌ 测试4失败 (状态码: $HTTP_CODE)${NC}"
fi

# 测试5: Authorization头格式错误（缺少Bearer前缀）
echo -e "\n${YELLOW}测试5: Authorization头格式错误（缺少Bearer前缀）${NC}"
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "${BASE_URL}/api/v1/posts/${POST_ID}/favorite" \
  -H "Authorization: ${TOKEN}")
echo "$RESPONSE"
HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE" | cut -d':' -f2)
if [ "$HTTP_CODE" = "401" ]; then
  echo -e "${GREEN}✓ 测试5通过 (正确返回401)${NC}"
else
  echo -e "${RED}❌ 测试5失败 (状态码: $HTTP_CODE，应该是401)${NC}"
fi

# 测试6: 完全缺少Authorization头
echo -e "\n${YELLOW}测试6: 完全缺少Authorization头${NC}"
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "${BASE_URL}/api/v1/posts/${POST_ID}/favorite")
echo "$RESPONSE"
HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE" | cut -d':' -f2)
if [ "$HTTP_CODE" = "401" ]; then
  echo -e "${GREEN}✓ 测试6通过 (正确返回401)${NC}"
else
  echo -e "${RED}❌ 测试6失败 (状态码: $HTTP_CODE，应该是401)${NC}"
fi

# 测试7: 模拟前端常见的fetch请求（带所有常见头）
echo -e "\n${YELLOW}测试7: 模拟前端fetch请求（带常见头）${NC}"
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "${BASE_URL}/api/v1/posts/${POST_ID}/favorite" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -H "Accept: application/json" \
  -H "Origin: http://localhost:3000" \
  -d '')
echo "$RESPONSE"
HTTP_CODE=$(echo "$RESPONSE" | grep "HTTP_CODE" | cut -d':' -f2)
if [ "$HTTP_CODE" = "200" ] || [ "$HTTP_CODE" = "409" ]; then
  echo -e "${GREEN}✓ 测试7通过 (状态码: $HTTP_CODE)${NC}"
else
  echo -e "${RED}❌ 测试7失败 (状态码: $HTTP_CODE)${NC}"
fi

echo -e "\n${YELLOW}======================================${NC}"
echo -e "${YELLOW}测试完成${NC}"
echo -e "${YELLOW}======================================${NC}"
echo -e "\n${YELLOW}请查看日志文件确认哪个测试触发了400错误${NC}"
echo -e "日志文件: logs/auth-service.log"

