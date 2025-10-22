#!/usr/bin/env python3
"""
测试文件: test_share_comprehensive.py
测试目的: 全面测试应用内分享功能
创建时间: 2025-10-22
测试完成后保留供参考
"""

import requests
import json
import mysql.connector
from datetime import datetime

# 配置
SERVER = "http://localhost:8080"
DB_CONFIG = {
    'host': '8.138.115.164',
    'user': 'root',
    'password': 'Xzk200411.',
    'database': 'knot_image_sharing'
}

# 颜色输出
class Colors:
    GREEN = '\033[0;32m'
    RED = '\033[0;31m'
    YELLOW = '\033[1;33m'
    BLUE = '\033[0;34m'
    NC = '\033[0m'  # No Color

def print_test(title):
    print(f"\n{Colors.YELLOW}=== {title} ==={Colors.NC}")

def print_success(msg):
    print(f"{Colors.GREEN}✅ {msg}{Colors.NC}")

def print_error(msg):
    print(f"{Colors.RED}❌ {msg}{Colors.NC}")

def print_warning(msg):
    print(f"{Colors.YELLOW}⚠️  {msg}{Colors.NC}")

def print_info(msg):
    print(f"{Colors.BLUE}ℹ️  {msg}{Colors.NC}")

def query_database(query):
    """执行数据库查询"""
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor(dictionary=True)
        cursor.execute(query)
        results = cursor.fetchall()
        cursor.close()
        conn.close()
        return results
    except Exception as e:
        print_error(f"数据库查询失败: {e}")
        return []

def test_1_database_structure():
    """测试1: 验证数据库结构"""
    print_test("测试1: 验证数据库结构")
    
    # 检查shares表是否存在
    tables = query_database("SHOW TABLES LIKE 'shares'")
    if tables:
        print_success("shares表存在")
    else:
        print_error("shares表不存在")
        return False
    
    # 检查表结构
    columns = query_database("DESCRIBE shares")
    expected_columns = ['id', 'share_id', 'post_id', 'sender_id', 'receiver_id', 'share_message', 'create_time']
    actual_columns = [col['Field'] for col in columns]
    
    for col in expected_columns:
        if col in actual_columns:
            print_success(f"字段 {col} 存在")
        else:
            print_error(f"字段 {col} 缺失")
    
    return True

def test_2_existing_shares():
    """测试2: 查询现有分享记录"""
    print_test("测试2: 查询现有分享记录")
    
    shares = query_database("""
        SELECT s.id, s.share_id, s.post_id, s.sender_id, s.receiver_id, s.share_message,
               s.create_time, p.title as post_title, 
               u1.username as sender_name, u2.username as receiver_name
        FROM shares s
        LEFT JOIN posts p ON s.post_id = p.id
        LEFT JOIN users u1 ON s.sender_id = u1.id
        LEFT JOIN users u2 ON s.receiver_id = u2.id
        ORDER BY s.create_time DESC
        LIMIT 5
    """)
    
    if shares:
        print_success(f"找到 {len(shares)} 条分享记录:")
        for share in shares:
            print(f"  - ID: {share['share_id']}")
            print(f"    帖子: {share['post_title']} (ID:{share['post_id']})")
            print(f"    {share['sender_name']} (ID:{share['sender_id']}) → {share['receiver_name']} (ID:{share['receiver_id']})")
            print(f"    消息: {share['share_message'] or '无'}")
            print(f"    时间: {share['create_time']}")
            print()
    else:
        print_warning("没有找到分享记录")
    
    return shares

def test_3_post_info():
    """测试3: 验证帖子信息查询"""
    print_test("测试3: 验证帖子信息查询 (帖子ID=65)")
    
    # 测试批量查询SQL
    result = query_database("""
        SELECT p.id, p.post_id, p.title, p.description, p.like_count, p.favorite_count,
               i.thumbnail_url
        FROM posts p
        LEFT JOIN (SELECT post_id, thumbnail_url FROM images WHERE display_order = 0) i
        ON p.id = i.post_id
        WHERE p.id IN (65)
    """)
    
    if result:
        post = result[0]
        print_success("帖子信息查询成功:")
        print(f"  物理ID: {post['id']}")
        print(f"  业务ID: {post['post_id']}")
        print(f"  标题: {post['title']}")
        print(f"  描述: {post['description'][:50]}...")
        print(f"  点赞数: {post['like_count']}")
        print(f"  收藏数: {post['favorite_count']}")
        print(f"  封面图: {post['thumbnail_url']}")
        return True
    else:
        print_error("帖子信息查询失败")
        return False

def test_4_user_info():
    """测试4: 验证用户信息查询"""
    print_test("测试4: 验证用户信息查询 (用户ID=2,35)")
    
    users = query_database("""
        SELECT id, user_id, username, avatar_url, bio
        FROM users
        WHERE id IN (2, 35)
    """)
    
    if users:
        print_success(f"查询到 {len(users)} 个用户:")
        for user in users:
            print(f"  - ID: {user['id']}, username: {user['username']}")
            print(f"    业务ID: {user['user_id']}")
            print(f"    头像: {user['avatar_url'] or '无'}")
            print(f"    简介: {user['bio'] or '无'}")
        return True
    else:
        print_error("用户信息查询失败")
        return False

def test_5_mutual_follow():
    """测试5: 验证互关关系"""
    print_test("测试5: 验证互关关系 (用户2和35)")
    
    follows = query_database("""
        SELECT follower_id, followee_id
        FROM follows
        WHERE (follower_id = 2 AND followee_id = 35)
           OR (follower_id = 35 AND followee_id = 2)
    """)
    
    if len(follows) == 2:
        print_success("用户2和35互相关注")
        return True
    else:
        print_warning(f"互关关系不完整, 找到 {len(follows)} 条记录")
        return False

def test_6_api_health():
    """测试6: API健康检查"""
    print_test("测试6: API健康检查")
    
    try:
        # 测试一个不需要认证的接口
        response = requests.get(f"{SERVER}/health", timeout=5)
        if response.status_code < 500:
            print_success(f"服务器响应正常 (状态码: {response.status_code})")
            return True
        else:
            print_error(f"服务器错误 (状态码: {response.status_code})")
            return False
    except Exception as e:
        print_error(f"无法连接到服务器: {e}")
        return False

def main():
    print(f"{Colors.BLUE}")
    print("╔═══════════════════════════════════════════════════════════╗")
    print("║      应用内分享功能 - 综合测试                              ║")
    print("║      测试时间: " + datetime.now().strftime("%Y-%m-%d %H:%M:%S") + "                         ║")
    print("╚═══════════════════════════════════════════════════════════╝")
    print(f"{Colors.NC}")
    
    # 运行所有测试
    tests = [
        test_1_database_structure,
        test_2_existing_shares,
        test_3_post_info,
        test_4_user_info,
        test_5_mutual_follow,
        test_6_api_health,
    ]
    
    passed = 0
    failed = 0
    
    for test_func in tests:
        try:
            result = test_func()
            if result or result is None:
                passed += 1
            else:
                failed += 1
        except Exception as e:
            print_error(f"测试异常: {e}")
            failed += 1
    
    # 总结
    print(f"\n{Colors.BLUE}╔═══════════════════════════════════════════════════════════╗{Colors.NC}")
    print(f"{Colors.BLUE}║  测试总结{Colors.NC}")
    print(f"{Colors.BLUE}╠═══════════════════════════════════════════════════════════╣{Colors.NC}")
    print(f"{Colors.GREEN}║  通过: {passed} 个测试{Colors.NC}")
    if failed > 0:
        print(f"{Colors.RED}║  失败: {failed} 个测试{Colors.NC}")
    print(f"{Colors.BLUE}╚═══════════════════════════════════════════════════════════╝{Colors.NC}")

if __name__ == "__main__":
    main()

