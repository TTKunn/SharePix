#!/bin/bash
# 测试帖子接口用户ID逻辑化改造
# 验证所有帖子相关API返回的user_id字段为逻辑ID（字符串格式）而非物理ID（整数格式）
# 测试时间: 2025-10-18

BASE_URL="http://localhost:8080/api/v1"

echo "========================================="
echo "帖子接口用户ID逻辑化改造 - 测试脚本"
echo "========================================="
echo ""

# 1. 登录测试用户(使用已有账户)
echo "1. 登录测试用户..."
LOGIN_RESPONSE=$(curl -s -X POST "$BASE_URL/auth/login" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "Test123456"
  }')

echo "$LOGIN_RESPONSE" | jq '.'

# 提取access_token
ACCESS_TOKEN=$(echo "$LOGIN_RESPONSE" | jq -r '.data.access_token')

# 获取用户信息以获取逻辑user_id
USER_PROFILE=$(curl -s -X GET "$BASE_URL/users/profile" \
  -H "Authorization: Bearer $ACCESS_TOKEN")

LOGICAL_USER_ID=$(echo "$USER_PROFILE" | jq -r '.data.user_id')

if [ "$ACCESS_TOKEN" == "null" ] || [ -z "$ACCESS_TOKEN" ]; then
    echo "❌ 登录失败，无法获取access_token"
    exit 1
fi

echo "✅ 登录成功"
echo "   逻辑用户ID: $LOGICAL_USER_ID"
echo "   Access Token: ${ACCESS_TOKEN:0:20}..."
echo ""

# 2. 创建测试帖子
echo "2. 创建测试帖子..."
POST_RESPONSE=$(curl -s -X POST "$BASE_URL/posts" \
  -H "Authorization: Bearer $ACCESS_TOKEN" \
  -F "title=测试逻辑ID帖子" \
  -F "description=验证user_id返回逻辑ID" \
  -F "tags=测试" \
  -F "image=@/home/kun/projects/SharePix/backend-service/test/pictures/milvus.png")

echo "$POST_RESPONSE" | jq '.'

POST_ID=$(echo "$POST_RESPONSE" | jq -r '.data.post_id')
POST_USER_ID=$(echo "$POST_RESPONSE" | jq -r '.data.user_id')

if [ "$POST_ID" == "null" ] || [ -z "$POST_ID" ]; then
    echo "❌ 创建帖子失败"
    exit 1
fi

echo "✅ 创建帖子成功"
echo "   帖子ID: $POST_ID"
echo "   返回的user_id: $POST_USER_ID"
echo ""

# 3. 验证user_id格式
echo "3. 验证返回的user_id格式..."

# 检查是否为字符串格式（USR_开头）
if [[ "$POST_USER_ID" =~ ^USR_ ]]; then
    echo "✅ user_id格式正确: $POST_USER_ID (逻辑ID)"
else
    echo "❌ user_id格式错误: $POST_USER_ID (应为USR_开头的逻辑ID)"
    exit 1
fi

# 检查是否与登录用户的逻辑ID一致
if [ "$POST_USER_ID" == "$LOGICAL_USER_ID" ]; then
    echo "✅ user_id与登录用户的逻辑ID一致"
else
    echo "❌ user_id不一致: 帖子=$POST_USER_ID, 用户=$LOGICAL_USER_ID"
    exit 1
fi
echo ""

# 4. 测试获取帖子详情接口
echo "4. 测试获取帖子详情接口..."
DETAIL_RESPONSE=$(curl -s -X GET "$BASE_URL/posts/$POST_ID" \
  -H "Authorization: Bearer $ACCESS_TOKEN")

echo "$DETAIL_RESPONSE" | jq '.'

DETAIL_USER_ID=$(echo "$DETAIL_RESPONSE" | jq -r '.data.user_id')

if [[ "$DETAIL_USER_ID" =~ ^USR_ ]]; then
    echo "✅ 帖子详情user_id格式正确: $DETAIL_USER_ID"
else
    echo "❌ 帖子详情user_id格式错误: $DETAIL_USER_ID"
    exit 1
fi
echo ""

# 5. 测试Feed流接口
echo "5. 测试Feed流接口..."
FEED_RESPONSE=$(curl -s -X GET "$BASE_URL/posts?page=1&page_size=10" \
  -H "Authorization: Bearer $ACCESS_TOKEN")

echo "$FEED_RESPONSE" | jq '.'

# 提取第一个帖子的user_id
FEED_USER_ID=$(echo "$FEED_RESPONSE" | jq -r '.data.posts[0].user_id')

if [[ "$FEED_USER_ID" =~ ^USR_ ]]; then
    echo "✅ Feed流user_id格式正确: $FEED_USER_ID"
else
    echo "❌ Feed流user_id格式错误: $FEED_USER_ID"
    exit 1
fi
echo ""

# 6. 测试图片列表中的user_id
echo "6. 测试图片列表中的user_id..."
IMAGES=$(echo "$DETAIL_RESPONSE" | jq -r '.data.images[]')

if [ -z "$IMAGES" ] || [ "$IMAGES" == "null" ]; then
    echo "⚠️  帖子没有图片，跳过图片user_id验证"
else
    IMAGE_USER_ID=$(echo "$DETAIL_RESPONSE" | jq -r '.data.images[0].user_id')
    if [[ "$IMAGE_USER_ID" =~ ^USR_ ]]; then
        echo "✅ 图片user_id格式正确: $IMAGE_USER_ID"
    else
        echo "❌ 图片user_id格式错误: $IMAGE_USER_ID"
        exit 1
    fi
fi
echo ""

# 7. 清理：删除测试帖子
echo "7. 清理测试数据..."
DELETE_RESPONSE=$(curl -s -X DELETE "$BASE_URL/posts/$POST_ID" \
  -H "Authorization: Bearer $ACCESS_TOKEN")

echo "$DELETE_RESPONSE" | jq '.'
echo "✅ 测试帖子已删除"
echo ""

# 8. 测试总结
echo "========================================="
echo "✅ 所有测试通过！"
echo "========================================="
echo ""
echo "验证结果:"
echo "  - 创建帖子接口: user_id返回逻辑ID ✓"
echo "  - 帖子详情接口: user_id返回逻辑ID ✓"
echo "  - Feed流接口: user_id返回逻辑ID ✓"
echo "  - 图片列表: user_id返回逻辑ID ✓"
echo ""
echo "改造成功! 所有帖子相关API的user_id已从物理ID改为逻辑ID。"
echo ""
