#!/bin/bash
# 获取测试用户的token

# 注册一个新用户并登录
TIMESTAMP=$(date +%s)
USERNAME="testuser_${TIMESTAMP}"
PHONE="138${TIMESTAMP:(-8)}"

echo "注册新用户: $USERNAME, 手机: $PHONE"

# 注册
REGISTER_RESP=$(curl -s -X POST "http://localhost:8080/api/v1/auth/register" \
  -H "Content-Type: application/json" \
  -d "{
    \"username\": \"$USERNAME\",
    \"password\": \"test123456\",
    \"phone\": \"$PHONE\"
  }")

echo "注册响应: $REGISTER_RESP"

# 登录获取token
LOGIN_RESP=$(curl -s -X POST "http://localhost:8080/api/v1/auth/login" \
  -H "Content-Type: application/json" \
  -d "{
    \"phone\": \"$PHONE\",
    \"password\": \"test123456\"
  }")

echo "登录响应: $LOGIN_RESP"

# 提取token
TOKEN=$(echo "$LOGIN_RESP" | grep -o '"access_token":"[^"]*"' | cut -d'"' -f4)

if [ -n "$TOKEN" ]; then
    echo ""
    echo "✅ Token获取成功:"
    echo "$TOKEN"
    echo ""
    echo "export TOKEN='$TOKEN'"
else
    echo "❌ Token获取失败"
fi
