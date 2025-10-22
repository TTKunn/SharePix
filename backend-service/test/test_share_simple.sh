#!/bin/bash
# 测试文件: test_share_simple.sh  
# 测试目的: 简单测试分享功能核心问题
# 创建时间: 2025-10-22

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

DB_HOST="8.138.115.164"
DB_USER="root"
DB_PASS="Xzk200411."
DB_NAME="knot_image_sharing"

echo -e "${BLUE}╔═══════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║      应用内分享功能 - 问题诊断测试                      ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════╝${NC}"

# 测试1: 验证数据库表结构
echo ""
echo -e "${YELLOW}=== 测试1: 验证数据库表结构 ===${NC}"
mysql -h $DB_HOST -u $DB_USER -p"$DB_PASS" -e "USE $DB_NAME; DESCRIBE shares;" 2>/dev/null
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ shares表结构正常${NC}"
else
    echo -e "${RED}❌ shares表不存在或无法访问${NC}"
fi

# 测试2: 查询现有分享记录
echo ""
echo -e "${YELLOW}=== 测试2: 查询现有分享记录 ===${NC}"
SHARE_COUNT=$(mysql -h $DB_HOST -u $DB_USER -p"$DB_PASS" -N -e "USE $DB_NAME; SELECT COUNT(*) FROM shares;" 2>/dev/null)
echo "分享记录数: $SHARE_COUNT"

if [ "$SHARE_COUNT" -gt 0 ]; then
    echo ""
    echo "最近的分享记录:"
    mysql -h $DB_HOST -u $DB_USER -p"$DB_PASS" -e "
        USE $DB_NAME;
        SELECT s.share_id, s.post_id, s.sender_id, s.receiver_id, s.share_message, s.create_time
        FROM shares s
        ORDER BY s.create_time DESC
        LIMIT 3;
    " 2>/dev/null
fi

# 测试3: 验证帖子批量查询SQL
echo ""
echo -e "${YELLOW}=== 测试3: 验证帖子批量查询SQL (问题1修复验证) ===${NC}"
echo "执行SQL: SELECT p.id, p.post_id, p.title, i.thumbnail_url FROM posts p"
echo "         LEFT JOIN (SELECT post_id, thumbnail_url FROM images WHERE display_order = 0) i"
echo "         ON p.id = i.post_id WHERE p.id IN (65)"

mysql -h $DB_HOST -u $DB_USER -p"$DB_PASS" -e "
    USE $DB_NAME;
    SELECT p.id, p.post_id, p.title, p.description, p.like_count, p.favorite_count, i.thumbnail_url
    FROM posts p
    LEFT JOIN (SELECT post_id, thumbnail_url FROM images WHERE display_order = 0) i
    ON p.id = i.post_id
    WHERE p.id IN (65);
" 2>/dev/null

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ 帖子批量查询SQL执行成功${NC}"
else
    echo -e "${RED}❌ 帖子批量查询SQL执行失败${NC}"
fi

# 测试4: 验证用户信息查询
echo ""
echo -e "${YELLOW}=== 测试4: 验证用户信息查询 ===${NC}"
mysql -h $DB_HOST -u $DB_USER -p"$DB_PASS" -e "
    USE $DB_NAME;
    SELECT id, user_id, username, avatar_url
    FROM users
    WHERE id IN (2, 35);
" 2>/dev/null

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ 用户信息查询成功${NC}"
else
    echo -e "${RED}❌ 用户信息查询失败${NC}"
fi

# 测试5: 验证互关关系
echo ""
echo -e "${YELLOW}=== 测试5: 验证互关关系 ===${NC}"
FOLLOW_COUNT=$(mysql -h $DB_HOST -u $DB_USER -p"$DB_PASS" -N -e "
    USE $DB_NAME;
    SELECT COUNT(*) FROM follows
    WHERE (follower_id = 2 AND followee_id = 35)
       OR (follower_id = 35 AND followee_id = 2);
" 2>/dev/null)

echo "互关记录数: $FOLLOW_COUNT"
if [ "$FOLLOW_COUNT" = "2" ]; then
    echo -e "${GREEN}✅ 用户2和35互相关注${NC}"
else
    echo -e "${YELLOW}⚠️  互关关系不完整 (应为2条记录)${NC}"
fi

# 测试6: 模拟完整的分享列表查询
echo ""
echo -e "${YELLOW}=== 测试6: 模拟完整的分享列表查询 (用户35收到的分享) ===${NC}"
mysql -h $DB_HOST -u $DB_USER -p"$DB_PASS" -e "
    USE $DB_NAME;
    SELECT s.id, s.share_id, s.post_id, s.sender_id, s.share_message, s.create_time,
           p.title as post_title, p.description, p.like_count, p.favorite_count,
           i.thumbnail_url,
           u.username as sender_name, u.avatar_url as sender_avatar
    FROM shares s
    LEFT JOIN posts p ON s.post_id = p.id
    LEFT JOIN (SELECT post_id, thumbnail_url FROM images WHERE display_order = 0) i ON p.id = i.post_id
    LEFT JOIN users u ON s.sender_id = u.id
    WHERE s.receiver_id = 35
    ORDER BY s.create_time DESC
    LIMIT 5;
" 2>/dev/null

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✅ 完整查询SQL执行成功${NC}"
else
    echo -e "${RED}❌ 完整查询SQL执行失败${NC}"
fi

# 测试7: 检查images表字段名
echo ""
echo -e "${YELLOW}=== 测试7: 检查images表字段 (问题1根源) ===${NC}"
mysql -h $DB_HOST -u $DB_USER -p"$DB_PASS" -e "USE $DB_NAME; DESCRIBE images;" 2>/dev/null | grep -E "(Field|display_order|position)"

HAS_DISPLAY_ORDER=$(mysql -h $DB_HOST -u $DB_USER -p"$DB_PASS" -N -e "
    USE $DB_NAME;
    SHOW COLUMNS FROM images LIKE 'display_order';
" 2>/dev/null | wc -l)

if [ "$HAS_DISPLAY_ORDER" -gt 0 ]; then
    echo -e "${GREEN}✅ images表有display_order字段${NC}"
else
    echo -e "${RED}❌ images表缺少display_order字段${NC}"
fi

echo ""
echo -e "${BLUE}╔═══════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║  测试完成 - 请检查上述结果                             ║${NC}"
echo -e "${BLUE}╚═══════════════════════════════════════════════════════╝${NC}"

