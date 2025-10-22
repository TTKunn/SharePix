#!/bin/bash
# 测试文件: test_share_direct.sh
# 测试目的: 直接测试分享API,不依赖登录
# 创建时间: 2025-10-22

echo "=== 测试: 获取收到的分享列表(用户35) ==="
echo "请求: GET /api/v1/shares/received?page=1&page_size=10"
echo "说明: 用户ID 35 (testuser2) 查看收到的分享"
echo ""

# 使用已知的Token或生成测试Token
# 这里我们直接测试服务是否能处理请求
curl -v -X GET "http://localhost:8080/api/v1/shares/received?page=1&page_size=10" \
  -H "Authorization: Bearer test_token_placeholder" \
  2>&1 | grep -E "(HTTP|{.*})"

echo ""
echo ""
echo "=== 查看服务器日志(最后20行) ==="
tail -20 /home/kun/projects/SharePix/backend-service/logs/auth-service.log

