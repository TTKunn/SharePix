#!/bin/bash

# 评论API集成测试脚本
# 测试目的: 测试评论功能的创建、查询、删除接口
# 创建时间: 2025-10-19

BASE_URL="http://localhost:8080/api/v1"

echo "========================================="
echo "评论功能集成测试"
echo "========================================="
echo ""

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# 1. 用户登录获取token
echo "1. 用户登录..."
LOGIN_RESPONSE=$(curl -s -X POST "${BASE_URL}/auth/login" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "Test123456"
  }')

TOKEN=$(echo $LOGIN_RESPONSE | jq -r '.data.access_token')

if [ "$TOKEN" == "null" ] || [ -z "$TOKEN" ]; then
    echo -e "${RED}❌ 登录失败${NC}"
    echo "Response: $LOGIN_RESPONSE"
    exit 1
fi

echo -e "${GREEN}✓ 登录成功${NC}"
echo "Token: ${TOKEN:0:20}..."
echo ""

# 2. 获取第一个帖子ID
echo "2. 获取帖子列表..."
POSTS_RESPONSE=$(curl -s -X GET "${BASE_URL}/posts?page=1&page_size=1")
POST_ID=$(echo $POSTS_RESPONSE | jq -r '.data.posts[0].post_id')

if [ "$POST_ID" == "null" ] || [ -z "$POST_ID" ]; then
    echo -e "${RED}❌ 获取帖子失败${NC}"
    echo "Response: $POSTS_RESPONSE"
    exit 1
fi

echo -e "${GREEN}✓ 获取帖子成功${NC}"
echo "Post ID: $POST_ID"
echo ""

# 3. 创建评论
echo "3. 创建评论..."
CREATE_COMMENT_RESPONSE=$(curl -s -X POST "${BASE_URL}/posts/${POST_ID}/comments" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "content": "这是一条测试评论，功能测试中..."
  }')

COMMENT_ID=$(echo $CREATE_COMMENT_RESPONSE | jq -r '.data.comment_id')
SUCCESS=$(echo $CREATE_COMMENT_RESPONSE | jq -r '.success')

if [ "$SUCCESS" != "true" ] || [ "$COMMENT_ID" == "null" ]; then
    echo -e "${RED}❌ 创建评论失败${NC}"
    echo "Response: $CREATE_COMMENT_RESPONSE"
    exit 1
fi

echo -e "${GREEN}✓ 创建评论成功${NC}"
echo "Comment ID: $COMMENT_ID"
echo "评论数: $(echo $CREATE_COMMENT_RESPONSE | jq -r '.data.comment_count')"
echo ""

# 4. 获取评论列表
echo "4. 获取评论列表..."
GET_COMMENTS_RESPONSE=$(curl -s -X GET "${BASE_URL}/posts/${POST_ID}/comments?page=1&page_size=20")

COMMENTS_COUNT=$(echo $GET_COMMENTS_RESPONSE | jq -r '.data.comments | length')
TOTAL=$(echo $GET_COMMENTS_RESPONSE | jq -r '.data.total')
SUCCESS=$(echo $GET_COMMENTS_RESPONSE | jq -r '.success')

if [ "$SUCCESS" != "true" ]; then
    echo -e "${RED}❌ 获取评论列表失败${NC}"
    echo "Response: $GET_COMMENTS_RESPONSE"
    exit 1
fi

echo -e "${GREEN}✓ 获取评论列表成功${NC}"
echo "查询到 $COMMENTS_COUNT 条评论 (总计: $TOTAL 条)"

# 显示第一条评论
if [ "$COMMENTS_COUNT" -gt 0 ]; then
    FIRST_COMMENT=$(echo $GET_COMMENTS_RESPONSE | jq -r '.data.comments[0]')
    echo "第一条评论:"
    echo "  - Comment ID: $(echo $FIRST_COMMENT | jq -r '.comment_id')"
    echo "  - Content: $(echo $FIRST_COMMENT | jq -r '.content')"
    echo "  - Author: $(echo $FIRST_COMMENT | jq -r '.author.username')"
    echo "  - Create Time: $(echo $FIRST_COMMENT | jq -r '.create_time')"
fi
echo ""

# 5. 删除评论
echo "5. 删除评论..."
DELETE_COMMENT_RESPONSE=$(curl -s -X DELETE "${BASE_URL}/posts/${POST_ID}/comments/${COMMENT_ID}" \
  -H "Authorization: Bearer ${TOKEN}")

SUCCESS=$(echo $DELETE_COMMENT_RESPONSE | jq -r '.success')

if [ "$SUCCESS" != "true" ]; then
    echo -e "${RED}❌ 删除评论失败${NC}"
    echo "Response: $DELETE_COMMENT_RESPONSE"
    exit 1
fi

echo -e "${GREEN}✓ 删除评论成功${NC}"
echo "删除后评论数: $(echo $DELETE_COMMENT_RESPONSE | jq -r '.data.comment_count')"
echo ""

# 6. 验证评论已删除
echo "6. 验证评论已删除..."
VERIFY_RESPONSE=$(curl -s -X GET "${BASE_URL}/posts/${POST_ID}/comments?page=1&page_size=20")
FOUND_DELETED=$(echo $VERIFY_RESPONSE | jq -r ".data.comments[] | select(.comment_id == \"${COMMENT_ID}\")")

if [ -n "$FOUND_DELETED" ]; then
    echo -e "${RED}❌ 验证失败: 评论仍然存在${NC}"
    exit 1
fi

echo -e "${GREEN}✓ 验证成功: 评论已被删除${NC}"
echo ""

# 7. 测试无效评论内容
echo "7. 测试无效评论内容..."
INVALID_COMMENT_RESPONSE=$(curl -s -X POST "${BASE_URL}/posts/${POST_ID}/comments" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -d '{
    "content": ""
  }')

SUCCESS=$(echo $INVALID_COMMENT_RESPONSE | jq -r '.success')

if [ "$SUCCESS" == "true" ]; then
    echo -e "${RED}❌ 应该拒绝空评论${NC}"
    echo "Response: $INVALID_COMMENT_RESPONSE"
    exit 1
fi

echo -e "${GREEN}✓ 正确拒绝了空评论${NC}"
echo "错误信息: $(echo $INVALID_COMMENT_RESPONSE | jq -r '.message')"
echo ""

# 8. 测试未认证访问
echo "8. 测试游客访问评论列表..."
GUEST_RESPONSE=$(curl -s -X GET "${BASE_URL}/posts/${POST_ID}/comments")
SUCCESS=$(echo $GUEST_RESPONSE | jq -r '.success')

if [ "$SUCCESS" != "true" ]; then
    echo -e "${RED}❌ 游客应该能够查看评论列表${NC}"
    echo "Response: $GUEST_RESPONSE"
    exit 1
fi

echo -e "${GREEN}✓ 游客可以查看评论列表${NC}"
echo ""

echo "========================================="
echo -e "${GREEN}所有测试通过! ✓${NC}"
echo "========================================="
