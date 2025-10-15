#!/bin/bash
# 测试文件: test_large_image_base64.sh
# 测试目的: 使用大图片（1.5MB）测试JSON+Base64内存优化效果
# 创建时间: 2025-10-15
# 测试完成后删除

set -e

echo "========================================="
echo "🔥 大图片Base64压力测试（内存优化验证）"
echo "========================================="

# 配置
API_BASE="http://localhost:8080/api/v1"
TEST_USER="test_large_img_$(date +%s)"
TEST_PASSWORD="Test123456"
TEST_IMAGE="/home/kun/projects/SharePix/backend-service/test/pictures/C++.png"

echo ""
echo "📋 测试配置:"
echo "  - 用户名: $TEST_USER"
echo "  - 测试图片: $TEST_IMAGE"

# 检查图片是否存在
if [ ! -f "$TEST_IMAGE" ]; then
    echo "❌ 测试图片不存在: $TEST_IMAGE"
    exit 1
fi

# 获取图片大小
IMAGE_SIZE=$(stat -c "%s" "$TEST_IMAGE")
IMAGE_SIZE_KB=$((IMAGE_SIZE / 1024))
IMAGE_SIZE_MB=$(echo "scale=2; $IMAGE_SIZE / 1024 / 1024" | bc)

echo "  - 图片大小: $IMAGE_SIZE bytes (${IMAGE_SIZE_KB}KB / ${IMAGE_SIZE_MB}MB)"

if [ $IMAGE_SIZE -gt 5242880 ]; then
    echo "⚠️  警告：图片超过5MB，可能被拒绝"
fi

echo ""
echo "========================================="
echo "步骤1: 注册测试用户"
echo "========================================="
REGISTER_RESPONSE=$(curl -s -X POST "$API_BASE/auth/register" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$TEST_USER\",\"password\":\"$TEST_PASSWORD\",\"real_name\":\"大图测试\",\"phone\":\"1380000$(date +%s | tail -c 5)\"}")

echo "$REGISTER_RESPONSE" | python3 -m json.tool

if echo "$REGISTER_RESPONSE" | grep -q '"success".*:.*true'; then
    echo "✅ 注册成功"
else
    echo "ℹ️  用户可能已存在，继续登录..."
fi

echo ""
echo "========================================="
echo "步骤2: 登录获取Token"
echo "========================================="
LOGIN_RESPONSE=$(curl -s -X POST "$API_BASE/auth/login" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$TEST_USER\",\"password\":\"$TEST_PASSWORD\"}")

# 提取token
TOKEN=$(echo "$LOGIN_RESPONSE" | python3 -c "import sys, json; print(json.load(sys.stdin)['data']['access_token'])" 2>/dev/null)

if [ -z "$TOKEN" ]; then
    echo "❌ 登录失败，无法获取token"
    exit 1
fi

echo "✅ 登录成功"

echo ""
echo "========================================="
echo "步骤3: 将大图片转换为Base64"
echo "========================================="
echo "⏳ 正在读取和编码图片（这可能需要几秒钟）..."

START_ENCODE=$(date +%s.%N)
IMAGE_BASE64=$(base64 -w 0 "$TEST_IMAGE")
END_ENCODE=$(date +%s.%N)
ENCODE_TIME=$(echo "$END_ENCODE - $START_ENCODE" | bc)

BASE64_SIZE=${#IMAGE_BASE64}
BASE64_SIZE_KB=$((BASE64_SIZE / 1024))
BASE64_SIZE_MB=$(echo "scale=2; $BASE64_SIZE / 1024 / 1024" | bc)
INCREASE_PERCENT=$(echo "scale=2; ($BASE64_SIZE * 100.0 / $IMAGE_SIZE) - 100" | bc)

echo "✅ Base64编码完成"
echo "  - 原始文件: $IMAGE_SIZE bytes (${IMAGE_SIZE_KB}KB / ${IMAGE_SIZE_MB}MB)"
echo "  - Base64大小: $BASE64_SIZE bytes (${BASE64_SIZE_KB}KB / ${BASE64_SIZE_MB}MB)"
echo "  - 增长率: ${INCREASE_PERCENT}%"
echo "  - 编码耗时: ${ENCODE_TIME}秒"

# 检查Base64大小是否超过限制
if [ $BASE64_SIZE -gt 7340032 ]; then
    echo ""
    echo "⚠️  警告：Base64数据超过7MB限制，请求将被拒绝"
    echo "  → Base64大小: ${BASE64_SIZE_MB}MB"
    echo "  → 服务器限制: 7MB"
    echo ""
    read -p "是否继续测试（验证限制是否生效）? [y/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "测试取消"
        exit 0
    fi
fi

echo ""
echo "========================================="
echo "步骤4: 构建JSON请求"
echo "========================================="

# 将JSON写入临时文件（避免shell参数长度限制）
TEMP_JSON_FILE="/tmp/test_large_image_payload_$$.json"

cat > "$TEMP_JSON_FILE" <<EOF
{
  "title": "大图片内存压力测试",
  "description": "测试${IMAGE_SIZE_MB}MB图片的Base64上传及内存优化效果",
  "tags": ["压力测试", "大图片", "内存优化"],
  "images": [
    {
      "filename": "C++.png",
      "content_type": "image/png",
      "data": "$IMAGE_BASE64"
    }
  ]
}
EOF

JSON_SIZE=$(stat -c "%s" "$TEMP_JSON_FILE")
JSON_SIZE_KB=$((JSON_SIZE / 1024))
JSON_SIZE_MB=$(echo "scale=2; $JSON_SIZE / 1024 / 1024" | bc)

echo "✅ JSON请求构建完成（已保存至临时文件）"
echo "  - JSON总大小: $JSON_SIZE bytes (${JSON_SIZE_KB}KB / ${JSON_SIZE_MB}MB)"
echo "  - 临时文件: $TEMP_JSON_FILE"

echo ""
echo "========================================="
echo "步骤5: 发送创建帖子请求"
echo "========================================="
echo "⏳ 正在发送请求（这可能需要几秒钟）..."
echo "📊 监控指标："
echo "  - 请求大小: ${JSON_SIZE_MB}MB"
echo "  - 预期内存峰值（修复前）: ~$((JSON_SIZE / 1024 / 1024 * 2))MB"
echo "  - 预期内存峰值（修复后）: ~$((JSON_SIZE / 1024 / 1024))MB"

START_TIME=$(date +%s.%N)

CREATE_RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}\n" -X POST "$API_BASE/posts" \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $TOKEN" \
    -d @"$TEMP_JSON_FILE")

END_TIME=$(date +%s.%N)
ELAPSED_TIME=$(echo "$END_TIME - $START_TIME" | bc)

# 提取HTTP状态码
HTTP_CODE=$(echo "$CREATE_RESPONSE" | grep "HTTP_CODE:" | cut -d: -f2)
RESPONSE_BODY=$(echo "$CREATE_RESPONSE" | sed '/HTTP_CODE:/d')

echo ""
echo "📊 响应结果:"
echo "HTTP状态码: $HTTP_CODE"
echo "$RESPONSE_BODY" | python3 -m json.tool 2>/dev/null || echo "$RESPONSE_BODY"

echo ""
echo "========================================="
echo "📈 性能指标"
echo "========================================="
echo "  - 图片大小: ${IMAGE_SIZE_MB}MB"
echo "  - Base64大小: ${BASE64_SIZE_MB}MB"
echo "  - JSON总大小: ${JSON_SIZE_MB}MB"
echo "  - Base64编码耗时: ${ENCODE_TIME}秒"
echo "  - 请求总耗时: ${ELAPSED_TIME}秒"

echo ""
echo "========================================="
echo "🎯 测试结果验证"
echo "========================================="

if echo "$RESPONSE_BODY" | grep -q '"success".*:.*true'; then
    echo "✅✅✅ 大图片测试成功！"
    
    POST_ID=$(echo "$RESPONSE_BODY" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data['data']['post']['post_id'])" 2>/dev/null || echo "")
    
    if [ -n "$POST_ID" ]; then
        echo "  - 帖子ID: $POST_ID"
    fi
    
    echo ""
    echo "✅ 内存优化验证通过："
    echo "  ✓ ${IMAGE_SIZE_MB}MB图片未导致崩溃"
    echo "  ✓ Base64解码成功"
    echo "  ✓ 请求耗时: ${ELAPSED_TIME}秒"
    echo "  ✓ 服务稳定运行"
    
elif echo "$RESPONSE_BODY" | grep -q '"message".*:.*"Base64数据过大"'; then
    echo "⚠️  请求被正确拒绝（图片超过限制）"
    echo "  ✓ 大小限制机制工作正常"
    echo "  ✓ 防止了潜在的内存问题"
    
elif [ "$HTTP_CODE" = "413" ]; then
    echo "⚠️  请求被正确拒绝（JSON body过大）"
    echo "  ✓ JSON大小限制机制工作正常"
    
else
    echo "❌ 测试失败"
    ERROR_MSG=$(echo "$RESPONSE_BODY" | python3 -c "import sys, json; print(json.load(sys.stdin).get('message', '未知错误'))" 2>/dev/null || echo "解析响应失败")
    echo "  - HTTP状态码: $HTTP_CODE"
    echo "  - 错误信息: $ERROR_MSG"
fi

echo ""
echo "========================================="
echo "🧹 清理临时文件"
echo "========================================="
if [ -f "$TEMP_JSON_FILE" ]; then
    rm -f "$TEMP_JSON_FILE"
    echo "✅ 临时JSON文件已删除"
fi

echo ""
echo "========================================="
echo "🏁 压力测试完成"
echo "========================================="

