#!/bin/bash
# 测试文件: test_static_file_upload.sh
# 测试目的: 测试图片上传并验证静态文件服务访问
# 创建时间: 2025-10-15
# 测试完成后删除

set -e

echo "========================================="
echo "📸 图片上传与静态文件服务测试"
echo "========================================="

# 配置
API_BASE="http://localhost:8080/api/v1"
TEST_USER="test_static_$(date +%s)"
TEST_PASSWORD="Test123456"
TEST_IMAGE="/home/kun/projects/SharePix/测试图片1.png"

echo ""
echo "📋 测试配置:"
echo "  - 用户名: $TEST_USER"
echo "  - 测试图片: $TEST_IMAGE"

# 检查图片是否存在
if [ ! -f "$TEST_IMAGE" ]; then
    echo "❌ 测试图片不存在: $TEST_IMAGE"
    exit 1
fi

# 获取图片信息
IMAGE_SIZE=$(stat -c "%s" "$TEST_IMAGE")
IMAGE_SIZE_KB=$((IMAGE_SIZE / 1024))
echo "  - 图片大小: $IMAGE_SIZE bytes (${IMAGE_SIZE_KB}KB)"

echo ""
echo "========================================="
echo "步骤1: 注册测试用户"
echo "========================================="
REGISTER_RESPONSE=$(curl -s -X POST "$API_BASE/auth/register" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$TEST_USER\",\"password\":\"$TEST_PASSWORD\",\"real_name\":\"静态文件测试\",\"phone\":\"1380000$(date +%s | tail -c 5)\"}")

if echo "$REGISTER_RESPONSE" | grep -q '"success".*:.*true'; then
    echo "✅ 注册成功"
else
    echo "ℹ️  注册响应: $(echo "$REGISTER_RESPONSE" | python3 -m json.tool 2>/dev/null)"
fi

echo ""
echo "========================================="
echo "步骤2: 登录获取Token"
echo "========================================="
LOGIN_RESPONSE=$(curl -s -X POST "$API_BASE/auth/login" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$TEST_USER\",\"password\":\"$TEST_PASSWORD\"}")

TOKEN=$(echo "$LOGIN_RESPONSE" | python3 -c "import sys, json; print(json.load(sys.stdin)['data']['access_token'])" 2>/dev/null)

if [ -z "$TOKEN" ]; then
    echo "❌ 登录失败"
    exit 1
fi

echo "✅ 登录成功"

echo ""
echo "========================================="
echo "步骤3: 转换图片为Base64"
echo "========================================="
echo "⏳ 正在编码图片..."

IMAGE_BASE64=$(base64 -w 0 "$TEST_IMAGE")
BASE64_SIZE=${#IMAGE_BASE64}
BASE64_SIZE_KB=$((BASE64_SIZE / 1024))

echo "✅ Base64编码完成"
echo "  - Base64大小: $BASE64_SIZE bytes (${BASE64_SIZE_KB}KB)"

# 将JSON写入临时文件
TEMP_JSON_FILE="/tmp/test_static_upload_$$.json"

cat > "$TEMP_JSON_FILE" <<EOF
{
  "title": "静态文件服务测试",
  "description": "测试图片上传后的静态文件访问功能",
  "tags": ["测试", "静态文件"],
  "images": [
    {
      "filename": "测试图片1.png",
      "content_type": "image/png",
      "data": "$IMAGE_BASE64"
    }
  ]
}
EOF

echo ""
echo "========================================="
echo "步骤4: 上传图片创建帖子"
echo "========================================="
echo "⏳ 正在上传..."

CREATE_RESPONSE=$(curl -s -X POST "$API_BASE/posts" \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d @"$TEMP_JSON_FILE")

# 清理临时文件
rm -f "$TEMP_JSON_FILE"

echo ""
echo "📊 响应结果:"
echo "$CREATE_RESPONSE" | python3 -m json.tool

echo ""
echo "========================================="
echo "🎯 静态文件URL提取"
echo "========================================="

if echo "$CREATE_RESPONSE" | grep -q '"success".*:.*true'; then
    echo "✅ 上传成功！"
    echo ""
    
    # 提取关键信息
    POST_ID=$(echo "$CREATE_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data['data']['post']['post_id'])" 2>/dev/null)
    FILE_URL=$(echo "$CREATE_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data['data']['images'][0]['file_url'])" 2>/dev/null)
    THUMBNAIL_URL=$(echo "$CREATE_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data['data']['images'][0]['thumbnail_url'])" 2>/dev/null)
    IMAGE_ID=$(echo "$CREATE_RESPONSE" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data['data']['images'][0]['image_id'])" 2>/dev/null)
    
    echo "📌 帖子信息:"
    echo "  - 帖子ID: $POST_ID"
    echo "  - 图片ID: $IMAGE_ID"
    echo ""
    echo "🌐 静态文件URL（您可以在浏览器中访问）:"
    echo ""
    echo "┌─────────────────────────────────────────┐"
    echo "│ 原图URL:                                 │"
    echo "│ $FILE_URL"
    echo "└─────────────────────────────────────────┘"
    echo ""
    echo "┌─────────────────────────────────────────┐"
    echo "│ 缩略图URL:                               │"
    echo "│ $THUMBNAIL_URL"
    echo "└─────────────────────────────────────────┘"
    echo ""
    echo "✨ 测试方法:"
    echo "  1. 在浏览器中打开上述URL"
    echo "  2. 或使用命令: curl -I \"$FILE_URL\""
    echo "  3. 应该能看到图片内容"
    echo ""
    
    # 测试URL是否可访问
    echo "========================================="
    echo "📡 自动测试URL可访问性"
    echo "========================================="
    
    echo "测试原图URL..."
    HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "$FILE_URL")
    if [ "$HTTP_CODE" = "200" ]; then
        echo "✅ 原图URL可访问 (HTTP $HTTP_CODE)"
    else
        echo "⚠️  原图URL返回 HTTP $HTTP_CODE"
    fi
    
    echo "测试缩略图URL..."
    HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" "$THUMBNAIL_URL")
    if [ "$HTTP_CODE" = "200" ]; then
        echo "✅ 缩略图URL可访问 (HTTP $HTTP_CODE)"
    else
        echo "⚠️  缩略图URL返回 HTTP $HTTP_CODE"
    fi
    
else
    echo "❌ 上传失败"
    ERROR_MSG=$(echo "$CREATE_RESPONSE" | python3 -c "import sys, json; print(json.load(sys.stdin).get('message', '未知错误'))" 2>/dev/null || echo "解析响应失败")
    echo "  - 错误信息: $ERROR_MSG"
    exit 1
fi

echo ""
echo "========================================="
echo "🏁 测试完成"
echo "========================================="

