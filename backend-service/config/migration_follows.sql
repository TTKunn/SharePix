-- ============================================================================
-- 关注系统数据库迁移脚本
-- ============================================================================
-- 文件: migration_follows.sql
-- 创建时间: 2025-10-16
-- 版本: v2.3.0
-- 描述: 创建关注关系表（follows），扩展users表添加关注计数字段
-- ============================================================================

USE knot_image_sharing;

-- ============================================================================
-- 第1步：创建follows表（关注关系表）
-- ============================================================================

CREATE TABLE IF NOT EXISTS follows (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    follower_id BIGINT NOT NULL COMMENT '关注者ID（A关注B，A是follower）',
    followee_id BIGINT NOT NULL COMMENT '被关注者ID（A关注B，B是followee）',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '关注时间',

    -- 外键约束（级联删除：用户删除时自动清理关注关系）
    CONSTRAINT fk_follows_follower FOREIGN KEY (follower_id) REFERENCES users(id) ON DELETE CASCADE,
    CONSTRAINT fk_follows_followee FOREIGN KEY (followee_id) REFERENCES users(id) ON DELETE CASCADE,

    -- 唯一约束（防止同一用户重复关注同一用户）
    UNIQUE KEY uk_follower_followee (follower_id, followee_id),

    -- 索引优化
    INDEX idx_follower_create (follower_id, create_time DESC) COMMENT '查询"我关注的人"列表',
    INDEX idx_followee_create (followee_id, create_time DESC) COMMENT '查询"关注我的人"列表'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='关注关系表';

-- ============================================================================
-- 第2步：扩展users表（添加关注计数冗余字段）
-- ============================================================================

-- 添加关注数字段（我关注的人数）
ALTER TABLE users 
ADD COLUMN IF NOT EXISTS following_count INT NOT NULL DEFAULT 0 
COMMENT '关注数（我关注的人数，冗余字段）' AFTER bio;

-- 添加粉丝数字段（关注我的人数）
ALTER TABLE users 
ADD COLUMN IF NOT EXISTS follower_count INT NOT NULL DEFAULT 0 
COMMENT '粉丝数（关注我的人数，冗余字段）' AFTER following_count;

-- ============================================================================
-- 第3步：验证表结构
-- ============================================================================

-- 查看follows表结构
SHOW CREATE TABLE follows\G

-- 查看users表结构（验证新增字段）
DESCRIBE users;

-- ============================================================================
-- 完成
-- ============================================================================
-- 迁移脚本执行完成！
-- 
-- 后续步骤：
-- 1. 实现Follow模型类（src/models/follow.{h,cpp}）
-- 2. 实现FollowRepository（src/database/follow_repository.{h,cpp}）
-- 3. 扩展UserRepository（添加关注计数更新方法）
-- 4. 实现FollowService（src/core/follow_service.{h,cpp}）
-- 5. 实现FollowHandler（src/api/follow_handler.{h,cpp}）
-- 6. 注册路由并测试
-- ============================================================================



