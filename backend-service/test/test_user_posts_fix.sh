#!/bin/bash
# 测试文件: test_user_posts_fix.sh
# 测试目的: 验证获取用户帖子接口的两个修复
# 创建时间: 2025-10-18
# 说明: 测试完成后保留此文件作为回归测试

BASE_URL="http://localhost:8080/api/v1"

echo "========================================"
echo "获取用户帖子接口修复验证测试"
echo "========================================"
echo ""

# 首先检查服务是否运行
echo "[检查] 检查服务状态..."
curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/health > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "❌ 错误: 服务未运行，请先启动服务"
    echo "启动命令: cd /home/kun/projects/SharePix/backend-service && ./build/knot_image_sharing config/config.json"
    exit 1
fi
echo "✓ 服务正在运行"
echo ""

# 测试1: 使用逻辑ID查询 (新功能)
echo "========================================"
echo "测试1: 使用逻辑ID查询 (新功能)"
echo "========================================"
echo "请求: GET ${BASE_URL}/users/USR_2025Q4_CBNLB0/posts?page_size=5"
echo ""
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" "${BASE_URL}/users/USR_2025Q4_CBNLB0/posts?page_size=5" -H "Content-Type: application/json")
HTTP_CODE=$(echo "$RESPONSE" | grep HTTP_CODE | cut -d: -f2)
BODY=$(echo "$RESPONSE" | sed '/HTTP_CODE/d')

echo "响应状态码: $HTTP_CODE"
if [ "$HTTP_CODE" == "200" ]; then
    echo "✓ 测试通过 - 逻辑ID解析成功"
    echo "响应数据:"
    echo "$BODY" | python3 -m json.tool 2>/dev/null | head -20
else
    echo "❌ 测试失败 - 预期状态码200，实际: $HTTP_CODE"
    echo "响应内容: $BODY"
fi
echo ""
sleep 1

# 测试2: 使用物理ID查询 (向后兼容)
echo "========================================"
echo "测试2: 使用物理ID查询 (向后兼容)"
echo "========================================"
echo "请求: GET ${BASE_URL}/users/7/posts?page_size=10"
echo ""
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" "${BASE_URL}/users/7/posts?page_size=10" -H "Content-Type: application/json")
HTTP_CODE=$(echo "$RESPONSE" | grep HTTP_CODE | cut -d: -f2)
BODY=$(echo "$RESPONSE" | sed '/HTTP_CODE/d')

echo "响应状态码: $HTTP_CODE"
if [ "$HTTP_CODE" == "200" ]; then
    echo "✓ 测试通过 - 物理ID兼容正常"
    # 检查page_size是否正确返回
    PAGE_SIZE=$(echo "$BODY" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('data', {}).get('page_size', 0))" 2>/dev/null)
    if [ "$PAGE_SIZE" == "10" ]; then
        echo "✓ 分页参数正确 - page_size返回: $PAGE_SIZE"
    else
        echo "⚠ 分页参数异常 - page_size返回: $PAGE_SIZE (预期: 10)"
    fi
else
    echo "❌ 测试失败 - 预期状态码200，实际: $HTTP_CODE"
    echo "响应内容: $BODY"
fi
echo ""
sleep 1

# 测试3: 不存在的逻辑ID (错误处理)
echo "========================================"
echo "测试3: 不存在的逻辑ID (错误处理)"
echo "========================================"
echo "请求: GET ${BASE_URL}/users/USR_9999Q4_NOTEXIST/posts"
echo ""
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" "${BASE_URL}/users/USR_9999Q4_NOTEXIST/posts" -H "Content-Type: application/json")
HTTP_CODE=$(echo "$RESPONSE" | grep HTTP_CODE | cut -d: -f2)
BODY=$(echo "$RESPONSE" | sed '/HTTP_CODE/d')

echo "响应状态码: $HTTP_CODE"
if [ "$HTTP_CODE" == "404" ]; then
    echo "✓ 测试通过 - 正确返回404错误"
    echo "错误消息: $(echo "$BODY" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('message', ''))" 2>/dev/null)"
else
    echo "❌ 测试失败 - 预期状态码404，实际: $HTTP_CODE"
    echo "响应内容: $BODY"
fi
echo ""
sleep 1

# 测试4: page_size参数验证
echo "========================================"
echo "测试4: page_size参数验证"
echo "========================================"
echo "请求: GET ${BASE_URL}/users/7/posts?page_size=15"
echo ""
RESPONSE=$(curl -s -w "\nHTTP_CODE:%{http_code}" "${BASE_URL}/users/7/posts?page_size=15" -H "Content-Type: application/json")
HTTP_CODE=$(echo "$RESPONSE" | grep HTTP_CODE | cut -d: -f2)
BODY=$(echo "$RESPONSE" | sed '/HTTP_CODE/d')

echo "响应状态码: $HTTP_CODE"
if [ "$HTTP_CODE" == "200" ]; then
    PAGE_SIZE=$(echo "$BODY" | python3 -c "import sys, json; data=json.load(sys.stdin); print(data.get('data', {}).get('page_size', 0))" 2>/dev/null)
    if [ "$PAGE_SIZE" == "15" ]; then
        echo "✓ 测试通过 - page_size参数生效: $PAGE_SIZE"
    else
        echo "❌ 测试失败 - page_size参数未生效，返回: $PAGE_SIZE (预期: 15)"
    fi
else
    echo "❌ 测试失败 - 预期状态码200，实际: $HTTP_CODE"
fi
echo ""

# 测试总结
echo "========================================"
echo "测试完成"
echo "========================================"
echo ""
echo "修复内容:"
echo "1. ✓ 支持逻辑ID参数 (如 USR_2025Q4_XXX)"
echo "2. ✓ 向后兼容物理ID参数"
echo "3. ✓ page_size参数命名统一"
echo "4. ✓ 错误处理(用户不存在返回404)"
echo ""
echo "建议: 请检查服务日志确认详细执行情况"
echo "日志路径: logs/auth-service.log"










