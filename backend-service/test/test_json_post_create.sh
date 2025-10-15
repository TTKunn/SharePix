#!/bin/bash
# 
# 测试文件: test_json_post_create.sh
# 测试目的: 测试JSON格式创建帖子API（支持Base64图片）
# 创建时间: 2025-10-14
# 测试完成后删除
#

BASE_URL="http://localhost:8080/api/v1"

echo "========================================="
echo "测试JSON格式创建帖子API"
echo "========================================="
echo

# 步骤1: 注册测试用户（如果不存在）
echo "步骤1: 注册测试用户（如果已存在会失败，不影响）..."
REGISTER_RESPONSE=$(curl -s -X POST "${BASE_URL}/auth/register" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "jsontest",
    "password": "password123",
    "real_name": "JSON测试用户",
    "phone": "13900000001"
  }')

echo "注册响应: $REGISTER_RESPONSE"
echo

# 步骤2: 登录获取token
echo "步骤2: 登录获取access token..."
LOGIN_RESPONSE=$(curl -s -X POST "${BASE_URL}/auth/login" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "jsontest",
    "password": "password123"
  }')

echo "登录响应: $LOGIN_RESPONSE"
echo

TOKEN=$(echo $LOGIN_RESPONSE | python3 -c "import sys, json; print(json.load(sys.stdin)['data']['access_token'])" 2>/dev/null)

if [ -z "$TOKEN" ]; then
    echo "❌ 登录失败，无法获取token"
    exit 1
fi

echo "✓ 获取到token: ${TOKEN:0:50}..."
echo

# 步骤3: 准备Base64编码的测试图片
echo "步骤3: 准备Base64编码的测试图片..."

# 使用一个简单的1x1像素PNG图片的Base64编码
# 这是一个透明的PNG图片
BASE64_PNG="iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mNkYPhfDwAChwGA60e6kgAAAABJRU5ErkJggg=="

# 或者从实际文件读取并编码
if [ -f "/home/kun/projects/SharePix/backend-service/test/pictures/mysql.png" ]; then
    echo "  从文件读取并编码..."
    BASE64_IMAGE=$(base64 -w 0 /home/kun/projects/SharePix/backend-service/test/pictures/mysql.png)
    echo "  ✓ 图片编码完成，长度: ${#BASE64_IMAGE} 字符"
else
    echo "  使用预定义的测试图片"
    BASE64_IMAGE=$BASE64_PNG
fi

echo

# 步骤4: 测试JSON格式创建帖子
echo "步骤4: 使用JSON格式创建帖子..."
echo

JSON_PAYLOAD=$(cat <<EOF
{
  "title": "JSON格式测试帖子",
  "description": "使用JSON+Base64上传图片的测试",
  "tags": ["测试", "JSON", "Base64"],
  "images": [
    {
      "filename": "test_image.png",
      "content_type": "image/png",
      "data": "${BASE64_IMAGE}"
    }
  ]
}
EOF
)

echo "发送JSON请求..."
CREATE_RESPONSE=$(curl -s -X POST "${BASE_URL}/posts" \
  -H "Authorization: Bearer ${TOKEN}" \
  -H "Content-Type: application/json" \
  -d "$JSON_PAYLOAD")

echo "创建帖子响应:"
echo "$CREATE_RESPONSE" | python3 -m json.tool 2>/dev/null || echo "$CREATE_RESPONSE"
echo

# 检查响应
if echo "$CREATE_RESPONSE" | grep -q '"success".*:.*true'; then
    echo "✅ 测试成功：JSON格式创建帖子成功！"
    POST_ID=$(echo $CREATE_RESPONSE | python3 -c "import sys, json; print(json.load(sys.stdin)['data']['post']['post_id'])" 2>/dev/null)
    IMAGE_COUNT=$(echo $CREATE_RESPONSE | python3 -c "import sys, json; print(json.load(sys.stdin)['data']['post']['image_count'])" 2>/dev/null)
    echo "   帖子ID: $POST_ID"
    echo "   图片数量: $IMAGE_COUNT"
else
    echo "❌ 测试失败：JSON格式创建帖子失败"
    exit 1
fi

echo
echo "========================================="
echo "测试完成"
echo "========================================="

