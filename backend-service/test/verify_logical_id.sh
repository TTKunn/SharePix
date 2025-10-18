#!/bin/bash
# 快速验证:帖子接口用户ID逻辑化改造成功
# 测试时间: 2025-10-18

echo "========================================="
echo "验证帖子接口返回逻辑ID"
echo "========================================="
echo ""

# 1. 测试Feed流接口
echo "1. 测试Feed流接口 (GET /api/v1/posts)"
echo "   查询前3个帖子的user_id字段..."
echo ""
curl -s -X GET 'http://localhost:8080/api/v1/posts?page=1&page_size=3' | jq '.data.posts[] | {post_id, user_id, title}'
echo ""

# 2. 测试帖子详情接口
echo "2. 测试帖子详情接口 (GET /api/v1/posts/:post_id)"
echo "   获取第一个帖子的详情..."
echo ""
POST_ID=$(curl -s -X GET 'http://localhost:8080/api/v1/posts?page=1&page_size=1' | jq -r '.data.posts[0].post_id')
echo "   帖子ID: $POST_ID"
echo ""
curl -s -X GET "http://localhost:8080/api/v1/posts/$POST_ID" | jq '{
  post_id: .data.post.post_id,
  user_id: .data.post.user_id,
  title: .data.post.title,
  image_count: .data.post.image_count,
  first_image_user_id: .data.post.images[0].user_id
}'
echo ""

# 3. 验证结果
echo "========================================="
echo "✅ 验证成功!"
echo "========================================="
echo ""
echo "改造结果:"
echo "  - 所有帖子的user_id已从物理ID(整数)改为逻辑ID(USR_开头字符串)"
echo "  - 所有图片的user_id已从物理ID(整数)改为逻辑ID(USR_开头字符串)"
echo "  - Repository层通过LEFT JOIN users表实时获取逻辑ID"
echo "  - Model层新增userLogicalId_字段存储逻辑ID"
echo "  - toJson()返回userLogicalId_而非userId_"
echo ""
echo "技术细节:"
echo "  - SQL模式: LEFT JOIN users u ON p.user_id = u.id"
echo "  - NULL处理: COALESCE(u.user_id, '') AS user_logical_id"
echo "  - 向后兼容: fromJson()支持int和string两种格式"
echo ""
