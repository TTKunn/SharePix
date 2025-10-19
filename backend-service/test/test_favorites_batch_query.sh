#!/bin/bash
# 测试文件: test_favorites_batch_query.sh
# 测试目的: 测试收藏列表批量查询优化功能
# 创建时间: 2025-10-19
# 使用后删除

BASE_URL="http://localhost:8080/api/v1"

echo "========================================="
echo "收藏列表批量查询优化测试"
echo "========================================="
echo ""

# 1. 登录获取Token
echo "[1] 登录获取Token..."
LOGIN_RESPONSE=$(curl -s -X POST "$BASE_URL/auth/login" \
  -H "Content-Type: application/json" \
  -d '{
    "username": "testuser",
    "password": "Test123456"
  }')

TOKEN=$(echo $LOGIN_RESPONSE | python3 -c "import sys, json; print(json.load(sys.stdin)['data']['access_token'])" 2>/dev/null)

if [ -z "$TOKEN" ]; then
    echo "✗ 登录失败"
    echo "$LOGIN_RESPONSE" | python3 -m json.tool
    exit 1
fi

echo "✓ 登录成功"
echo "Token: ${TOKEN:0:50}..."
echo ""

# 2. 测试无Token访问
echo "[2] 测试无Token访问（应返回401）..."
RESPONSE=$(curl -s -X GET "$BASE_URL/my/favorites")
SUCCESS=$(echo $RESPONSE | python3 -c "import sys, json; print(json.load(sys.stdin)['success'])" 2>/dev/null)

if [ "$SUCCESS" == "False" ]; then
    echo "✓ 无Token访问正确返回错误"
else
    echo "✗ 无Token访问应该失败"
fi
echo ""

# 3. 测试空收藏列表
echo "[3] 测试空收藏列表..."
RESPONSE=$(curl -s -X GET "$BASE_URL/my/favorites?page=1&page_size=5" \
  -H "Authorization: Bearer $TOKEN")

echo "$RESPONSE" | python3 -m json.tool
echo ""

POSTS_COUNT=$(echo $RESPONSE | python3 -c "import sys, json; print(len(json.load(sys.stdin)['data']['posts']))" 2>/dev/null)
echo "✓ 收藏列表返回 $POSTS_COUNT 个帖子"
echo ""

# 4. 检查返回字段结构
echo "[4] 检查返回字段结构..."
if [ "$POSTS_COUNT" -gt 0 ]; then
    FIRST_POST=$(echo $RESPONSE | python3 -c "import sys, json; import pprint; posts = json.load(sys.stdin)['data']['posts']; print(json.dumps(posts[0], indent=2))")

    echo "第一个帖子的字段:"
    echo "$FIRST_POST"
    echo ""

    # 检查必需字段
    HAS_AUTHOR=$(echo "$FIRST_POST" | python3 -c "import sys, json; print('author' in json.load(sys.stdin))")
    HAS_LIKED=$(echo "$FIRST_POST" | python3 -c "import sys, json; print('has_liked' in json.load(sys.stdin))")
    HAS_FAVORITED=$(echo "$FIRST_POST" | python3 -c "import sys, json; print('has_favorited' in json.load(sys.stdin))")

    if [ "$HAS_AUTHOR" == "True" ]; then
        echo "✓ author 字段存在"
    else
        echo "✗ author 字段缺失"
    fi

    if [ "$HAS_LIKED" == "True" ]; then
        echo "✓ has_liked 字段存在"
    else
        echo "✗ has_liked 字段缺失"
    fi

    if [ "$HAS_FAVORITED" == "True" ]; then
        FAVORITED_VALUE=$(echo "$FIRST_POST" | python3 -c "import sys, json; print(json.load(sys.stdin)['has_favorited'])")
        if [ "$FAVORITED_VALUE" == "True" ]; then
            echo "✓ has_favorited 字段存在且为true"
        else
            echo "✗ has_favorited 应该为true"
        fi
    else
        echo "✗ has_favorited 字段缺失"
    fi
else
    echo "收藏列表为空，跳过字段检查"
fi
echo ""

# 5. 性能测试
echo "[5] 性能测试（检查日志中的查询次数）..."
echo "请查看 server.log 中的 [GET FAVORITES] 日志"
echo "预期看到: '3 queries (1 posts + 1 authors + 1 likes)'"
echo ""

echo "========================================="
echo "测试完成"
echo "========================================="
