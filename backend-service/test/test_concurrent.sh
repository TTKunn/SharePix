#!/bin/bash
# 并发测试脚本
# 用途：测试并发请求时的行为

TOKEN="eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3NTkzOTU2NjUsImlhdCI6MTc1OTM5MjA2NSwiaXNzIjoic2hhcmVkLXBhcmtpbmctYXV0aCIsInN1YiI6IjIiLCJ1c2VybmFtZSI6InRlc3R1c2VyX3BhcmtpbmcifQ.sVkv8GxPucEeKn-jFvND6z_IQLYBfsQqgE3N7awBUNI"
BASE_URL="http://192.168.32.128:8080"

echo "========================================="
echo "并发测试：同时发送5个请求"
echo "========================================="

# 创建临时目录存储结果
TEMP_DIR="/home/kun/projects/shared-parking/test/temp_results"
mkdir -p "$TEMP_DIR"
rm -f "$TEMP_DIR"/*

# 并发发送5个请求
for i in {1..5}; do
  (
    RESULT=$(curl -s -w "\nHTTP_CODE:%{http_code}" -X POST "$BASE_URL/api/v1/my-spaces" \
      -H "Authorization: Bearer $TOKEN" \
      -H "Content-Type: application/json" \
      -d "{\"space_number\":\"CONCURRENT-$i\",\"location\":\"并发测试$i\",\"description\":\"并发测试停车位$i\"}")
    
    echo "$RESULT" > "$TEMP_DIR/result_$i.txt"
    echo "请求 $i 完成"
  ) &
done

# 等待所有后台任务完成
wait

echo ""
echo "========================================="
echo "结果分析："
echo "========================================="

SUCCESS_COUNT=0
FAIL_COUNT=0

for i in {1..5}; do
  echo "--- 请求 $i 的结果 ---"
  if [ -f "$TEMP_DIR/result_$i.txt" ]; then
    HTTP_CODE=$(grep "HTTP_CODE:" "$TEMP_DIR/result_$i.txt" | cut -d: -f2)
    BODY=$(grep -v "HTTP_CODE:" "$TEMP_DIR/result_$i.txt")
    
    echo "HTTP状态码: $HTTP_CODE"
    echo "$BODY" | python3 -m json.tool 2>/dev/null || echo "$BODY"
    
    if [ "$HTTP_CODE" = "200" ]; then
      SUCCESS_COUNT=$((SUCCESS_COUNT + 1))
    else
      FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
  else
    echo "结果文件不存在"
    FAIL_COUNT=$((FAIL_COUNT + 1))
  fi
  echo ""
done

echo "========================================="
echo "统计："
echo "成功: $SUCCESS_COUNT"
echo "失败: $FAIL_COUNT"
echo "========================================="

# 清理临时文件
rm -rf "$TEMP_DIR"

