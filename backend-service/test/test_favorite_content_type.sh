#!/bin/bash

# 测试文件: test_favorite_content_type.sh
# 测试目的: 复现前端400错误 - Content-Type与body不匹配问题
# 创建时间: 2025-10-18
# 测试完成后删除

BASE_URL="http://localhost:8080"

# 登录获取token
echo "登录获取token..."
LOGIN_RESPONSE=$(curl -s -X POST "${BASE_URL}/api/v1/auth/login" \
  -H "Content-Type: application/json" \
  -d '{
    "email": "testuser5@example.com",
    "password": "Test123456"
  }')

TOKEN=$(echo $LOGIN_RESPONSE | grep -o '"access_token":"[^"]*' | cut -d'"' -f4)

if [ -z "$TOKEN" ]; then
  echo "❌ 获取令牌失败"
  echo "响应: $LOGIN_RESPONSE"
  exit 1
fi

echo "✓ 令牌获取成功"

POST_ID="POST_2025Q4_8qYFhp"

echo ""
echo "========================================" 
echo "测试: 设置Content-Type但发送空body (这是最可能导致400的情况)"
echo "========================================"

# 使用nc或telnet手动发送HTTP请求以精确控制
echo -ne "POST /api/v1/posts/${POST_ID}/favorite HTTP/1.1\r\n\
Host: localhost:8080\r\n\
Authorization: Bearer ${TOKEN}\r\n\
Content-Type: application/json\r\n\
Content-Length: 0\r\n\
\r\n" | timeout 6 nc localhost 8080

echo ""
echo ""
echo "如果上面返回400且等待了5秒，说明这就是问题所在！"
echo ""
echo "解决方案："
echo "1. 前端不要设置Content-Type（对于无body的POST请求）"
echo "2. 或者设置Content-Length: 0"
echo "3. 或者发送空JSON: {}"

