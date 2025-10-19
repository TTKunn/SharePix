#!/bin/bash
# 测试头像上传内存泄漏问题修复
# 日期: 2025-10-19

TOKEN="eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3NjA4NjE4NDIsImlhdCI6MTc2MDg1ODI0MiwiaXNzIjoic2hhcmVkLXBhcmtpbmctYXV0aCIsInN1YiI6IjIiLCJ1c2VybmFtZSI6InRlc3R1c2VyIn0.kzsaXgPE1b4I-M_Y9EQy3l_yQGfBER5_fVpq1F5oc6c"

echo "====== 头像上传内存泄漏测试 ======"
echo ""
echo "测试前内存使用:"
ps aux | grep knot_image_sharing | grep -v grep | awk '{print "RSS: " $6/1024 " MB"}'
echo ""

echo "开始连续上传20次正方形头像(这是之前会泄漏的场景)..."
for i in {1..20}; do
  curl -s -X POST "http://localhost:8080/api/v1/users/avatar" \
    -H "Authorization: Bearer $TOKEN" \
    -F "avatar=@test/pictures/test_square_200x200.png" > /dev/null
  echo -n "."
  sleep 0.1
done

echo ""
echo ""
echo "测试后内存使用:"
ps aux | grep knot_image_sharing | grep -v grep | awk '{print "RSS: " $6/1024 " MB"}'
echo ""

echo "====== 测试完成 ======"
echo "说明: 如果修复成功,内存应该保持稳定(增长<5MB)"
echo "      如果未修复,每次泄漏约3MB,20次应增长约60MB"
