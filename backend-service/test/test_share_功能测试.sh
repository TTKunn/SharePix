#!/bin/bash
# 测试文件: test_share_功能测试.sh
# 测试目的: 测试应用内分享功能的所有API
# 创建时间: 2025-10-22
# 测试完成后保留供参考

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 服务器地址
SERVER="http://localhost:8080"

# 测试用户凭证
echo -e "${YELLOW}=== 步骤1: 获取测试用户Token ===${NC}"
echo "登录 testuser..."
TOKEN_USER1=$(curl -s -X POST "$SERVER/api/v1/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser","password":"test123"}' \
  | jq -r '.data.access_token')

if [ "$TOKEN_USER1" = "null" ] || [ -z "$TOKEN_USER1" ]; then
    echo -e "${RED}❌ testuser登录失败${NC}"
    exit 1
fi
echo -e "${GREEN}✅ testuser Token获取成功${NC}"

echo ""
echo "登录 testuser2..."
TOKEN_USER2=$(curl -s -X POST "$SERVER/api/v1/auth/login" \
  -H "Content-Type: application/json" \
  -d '{"username":"testuser2","password":"test123"}' \
  | jq -r '.data.access_token')

if [ "$TOKEN_USER2" = "null" ] || [ -z "$TOKEN_USER2" ]; then
    echo -e "${RED}❌ testuser2登录失败${NC}"
    exit 1
fi
echo -e "${GREEN}✅ testuser2 Token获取成功${NC}"

# 测试1: 创建分享
echo ""
echo -e "${YELLOW}=== 测试1: 创建分享 (testuser分享帖子65给testuser2) ===${NC}"
RESPONSE=$(curl -s -X POST "$SERVER/api/v1/shares/posts" \
  -H "Authorization: Bearer $TOKEN_USER1" \
  -H "Content-Type: application/json" \
  -d '{
    "post_id": "65",
    "receiver_id": "35",
    "share_message": "这是一个测试分享,推荐给你看!"
  }')

echo "响应: $RESPONSE" | jq '.'

SUCCESS=$(echo "$RESPONSE" | jq -r '.success')
if [ "$SUCCESS" = "true" ]; then
    SHARE_ID=$(echo "$RESPONSE" | jq -r '.data.share_id')
    echo -e "${GREEN}✅ 创建分享成功, share_id=$SHARE_ID${NC}"
else
    MESSAGE=$(echo "$RESPONSE" | jq -r '.message')
    echo -e "${YELLOW}⚠️ 创建分享: $MESSAGE${NC}"
fi

# 测试2: 获取收到的分享列表 (testuser2)
echo ""
echo -e "${YELLOW}=== 测试2: testuser2查看收到的分享列表 ===${NC}"
RESPONSE=$(curl -s -X GET "$SERVER/api/v1/shares/received?page=1&page_size=10" \
  -H "Authorization: Bearer $TOKEN_USER2")

echo "响应: $RESPONSE" | jq '.'

SUCCESS=$(echo "$RESPONSE" | jq -r '.success')
TOTAL=$(echo "$RESPONSE" | jq -r '.data.total')
SHARES_COUNT=$(echo "$RESPONSE" | jq -r '.data.shares | length')

if [ "$SUCCESS" = "true" ]; then
    echo -e "${GREEN}✅ 查询成功, total=$TOTAL, 返回$SHARES_COUNT条分享${NC}"
    
    if [ "$SHARES_COUNT" -gt 0 ]; then
        echo ""
        echo "分享详情:"
        echo "$RESPONSE" | jq -r '.data.shares[] | "- share_id: \(.share_id)\n  帖子: \(.post.title // "无标题")\n  发送者: \(.sender.username)\n  消息: \(.share_message // "无附言")\n"'
    else
        echo -e "${YELLOW}⚠️ 收到的分享列表为空${NC}"
    fi
else
    MESSAGE=$(echo "$RESPONSE" | jq -r '.message')
    echo -e "${RED}❌ 查询失败: $MESSAGE${NC}"
fi

# 测试3: 获取发出的分享列表 (testuser)
echo ""
echo -e "${YELLOW}=== 测试3: testuser查看发出的分享列表 ===${NC}"
RESPONSE=$(curl -s -X GET "$SERVER/api/v1/shares/sent?page=1&page_size=10" \
  -H "Authorization: Bearer $TOKEN_USER1")

echo "响应: $RESPONSE" | jq '.'

SUCCESS=$(echo "$RESPONSE" | jq -r '.success')
TOTAL=$(echo "$RESPONSE" | jq -r '.data.total')
SHARES_COUNT=$(echo "$RESPONSE" | jq -r '.data.shares | length')

if [ "$SUCCESS" = "true" ]; then
    echo -e "${GREEN}✅ 查询成功, total=$TOTAL, 返回$SHARES_COUNT条分享${NC}"
    
    if [ "$SHARES_COUNT" -gt 0 ]; then
        echo ""
        echo "分享详情:"
        echo "$RESPONSE" | jq -r '.data.shares[] | "- share_id: \(.share_id)\n  帖子: \(.post.title // "无标题")\n  接收者: \(.sender.username)\n  消息: \(.share_message // "无附言")\n"'
    fi
else
    MESSAGE=$(echo "$RESPONSE" | jq -r '.message')
    echo -e "${RED}❌ 查询失败: $MESSAGE${NC}"
fi

# 测试4: 重复分享测试
echo ""
echo -e "${YELLOW}=== 测试4: 重复分享同一帖子给同一用户 ===${NC}"
RESPONSE=$(curl -s -X POST "$SERVER/api/v1/shares/posts" \
  -H "Authorization: Bearer $TOKEN_USER1" \
  -H "Content-Type: application/json" \
  -d '{
    "post_id": "65",
    "receiver_id": "35",
    "share_message": "这是重复分享测试"
  }')

echo "响应: $RESPONSE" | jq '.'

SUCCESS=$(echo "$RESPONSE" | jq -r '.success')
if [ "$SUCCESS" = "false" ]; then
    MESSAGE=$(echo "$RESPONSE" | jq -r '.message')
    echo -e "${GREEN}✅ 正确拒绝重复分享: $MESSAGE${NC}"
else
    echo -e "${RED}❌ 应该拒绝重复分享,但没有拒绝${NC}"
fi

# 测试5: 分享消息长度测试
echo ""
echo -e "${YELLOW}=== 测试5: 分享消息长度限制测试 (>500字符) ===${NC}"
LONG_MESSAGE=$(python3 -c "print('测试' * 300)")  # 600字符
RESPONSE=$(curl -s -X POST "$SERVER/api/v1/shares/posts" \
  -H "Authorization: Bearer $TOKEN_USER1" \
  -H "Content-Type: application/json" \
  -d "{
    \"post_id\": \"65\",
    \"receiver_id\": \"3\",
    \"share_message\": \"$LONG_MESSAGE\"
  }")

echo "响应: $RESPONSE" | jq '.'

SUCCESS=$(echo "$RESPONSE" | jq -r '.success')
if [ "$SUCCESS" = "false" ]; then
    MESSAGE=$(echo "$RESPONSE" | jq -r '.message')
    echo -e "${GREEN}✅ 正确拒绝过长消息: $MESSAGE${NC}"
else
    echo -e "${YELLOW}⚠️ 应该拒绝过长消息,但通过了${NC}"
fi

echo ""
echo -e "${YELLOW}=== 测试总结 ===${NC}"
echo "已完成应用内分享功能的基础测试"
echo "详细日志请查看: /home/kun/projects/SharePix/backend-service/logs/auth-service.log"

