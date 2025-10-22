-- ============================================================================
-- 应用内分享系统数据库迁移脚本
-- ============================================================================
-- 文件: migration_shares.sql
-- 创建时间: 2025-10-22
-- 版本: v2.10.0
-- 描述: 创建分享记录表（shares），实现应用内帖子分享功能
-- 修改说明: 移除冗余字段，使用批量查询策略保证数据一致性
-- ============================================================================

USE knot_image_sharing;

-- ============================================================================
-- 第1步：创建shares表（分享记录表）
-- ============================================================================

CREATE TABLE IF NOT EXISTS shares (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    share_id VARCHAR(36) NOT NULL UNIQUE COMMENT '业务逻辑ID（例：SHR_2025Q4_ABC123）',
    post_id BIGINT NOT NULL COMMENT '被分享的帖子ID（物理ID，关联posts表）',
    sender_id BIGINT NOT NULL COMMENT '分享者ID（物理ID，关联users表）',
    receiver_id BIGINT NOT NULL COMMENT '接收者ID（物理ID，关联users表）',
    share_message TEXT COMMENT '分享附言（可选，最多500字符）',

    -- 时间字段
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '分享时间',

    -- 外键约束（级联删除：删除帖子/用户时自动清理分享记录）
    CONSTRAINT fk_shares_post FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,
    CONSTRAINT fk_shares_sender FOREIGN KEY (sender_id) REFERENCES users(id) ON DELETE CASCADE,
    CONSTRAINT fk_shares_receiver FOREIGN KEY (receiver_id) REFERENCES users(id) ON DELETE CASCADE,

    -- 唯一约束（防止同一用户重复分享同一帖子给同一用户）
    UNIQUE KEY uk_sender_receiver_post (sender_id, receiver_id, post_id),

    -- 索引优化
    INDEX idx_receiver_time (receiver_id, create_time DESC) COMMENT '接收者查看分享列表',
    INDEX idx_sender_time (sender_id, create_time DESC) COMMENT '发送者查看分享记录',
    INDEX idx_post_shares (post_id, create_time DESC) COMMENT '帖子分享统计',
    INDEX idx_create_time (create_time DESC) COMMENT '时间排序'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='分享记录表';

-- ============================================================================
-- 第2步：验证表结构
-- ============================================================================

-- 查看shares表结构
SHOW CREATE TABLE shares\G

-- 查看shares表字段详情
DESCRIBE shares;

-- ============================================================================
-- 完成
-- ============================================================================
-- 迁移脚本执行完成！
--
-- 设计要点：
-- 1. 无冗余字段设计 - 使用批量查询获取帖子和用户信息
-- 2. 唯一约束防重复 - 同一帖子不能重复分享给同一用户
-- 3. 级联删除保证一致性 - 删除帖子/用户时自动清理分享记录
-- 4. 索引优化查询性能 - 接收者列表、发送者列表、时间排序
--
-- 后续步骤：
-- 1. 实现Share模型类（src/models/share.{h,cpp}）
-- 2. 实现ShareRepository（src/database/share_repository.{h,cpp}）
-- 3. 实现ShareService（src/core/share_service.{h,cpp}）
-- 4. 实现ShareHandler（src/api/share_handler.{h,cpp}）
-- 5. 注册路由并测试
-- ============================================================================
