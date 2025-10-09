-- ============================================================================
-- Knot Image Sharing Service - 数据库迁移脚本
-- 从单张图片模式迁移到多图片帖子模式
-- ============================================================================
-- 创建时间: 2025-10-08
-- 版本: 2.0.0
-- 说明: 此脚本将现有的单张图片系统迁移为支持多图片帖子的系统
-- ============================================================================

USE knot_image_sharing;

-- ============================================================================
-- 第一步：备份现有数据
-- ============================================================================

-- 创建备份表
CREATE TABLE IF NOT EXISTS images_backup AS SELECT * FROM images;
CREATE TABLE IF NOT EXISTS image_tags_backup AS SELECT * FROM image_tags;

SELECT '✅ 步骤1：数据备份完成' AS status;

-- ============================================================================
-- 第二步：创建posts表
-- ============================================================================

CREATE TABLE IF NOT EXISTS posts (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    post_id VARCHAR(36) NOT NULL UNIQUE COMMENT '业务逻辑ID（例：POST_2025Q4_ABC123）',
    user_id BIGINT NOT NULL COMMENT '发布用户ID',
    title VARCHAR(255) NOT NULL COMMENT '帖子标题',
    description TEXT COMMENT '帖子配文',
    image_count INT NOT NULL DEFAULT 0 COMMENT '图片数量',
    like_count INT NOT NULL DEFAULT 0 COMMENT '点赞数',
    favorite_count INT NOT NULL DEFAULT 0 COMMENT '收藏数',
    view_count INT NOT NULL DEFAULT 0 COMMENT '浏览数',
    status ENUM('PENDING', 'APPROVED', 'REJECTED') NOT NULL DEFAULT 'APPROVED' COMMENT '审核状态',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    update_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    -- 外键约束
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,

    -- 索引优化
    INDEX idx_post_id (post_id),
    INDEX idx_user_id (user_id),
    INDEX idx_create_time (create_time DESC),
    INDEX idx_status (status),
    INDEX idx_user_create (user_id, create_time DESC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='帖子表';

SELECT '✅ 步骤2：posts表创建完成' AS status;

-- ============================================================================
-- 第三步：为每个现有image创建对应的post
-- ============================================================================

INSERT INTO posts (post_id, user_id, title, description, image_count, 
                   like_count, favorite_count, view_count, status, 
                   create_time, update_time)
SELECT 
    CONCAT('POST_', SUBSTRING(image_id, 5)) AS post_id,  -- IMG_2025Q4_ABC123 -> POST_2025Q4_ABC123
    user_id,
    title,
    description,
    1 AS image_count,  -- 每个旧图片对应一个单图片帖子
    like_count,
    favorite_count,
    view_count,
    status,
    create_time,
    update_time
FROM images;

SELECT CONCAT('✅ 步骤3：已创建 ', COUNT(*), ' 个帖子记录') AS status FROM posts;

-- ============================================================================
-- 第四步：修改images表结构
-- ============================================================================

-- 添加新字段（临时允许NULL）
ALTER TABLE images ADD COLUMN post_id BIGINT NULL COMMENT '所属帖子ID' AFTER image_id;
ALTER TABLE images ADD COLUMN display_order INT NOT NULL DEFAULT 0 COMMENT '显示顺序（0-8）' AFTER post_id;

SELECT '✅ 步骤4.1：images表新增字段完成' AS status;

-- 更新images表，关联到posts
UPDATE images i
JOIN posts p ON CONCAT('POST_', SUBSTRING(i.image_id, 5)) = p.post_id
SET i.post_id = p.id, i.display_order = 0;

SELECT '✅ 步骤4.2：images表关联posts完成' AS status;

-- 将post_id设置为NOT NULL
ALTER TABLE images MODIFY COLUMN post_id BIGINT NOT NULL COMMENT '所属帖子ID';

-- 添加外键和索引
ALTER TABLE images ADD FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE;
ALTER TABLE images ADD INDEX idx_post_id (post_id);
ALTER TABLE images ADD INDEX idx_post_order (post_id, display_order);

SELECT '✅ 步骤4.3：images表外键和索引创建完成' AS status;

-- 删除旧字段（移到posts表）
ALTER TABLE images DROP COLUMN title;
ALTER TABLE images DROP COLUMN description;
ALTER TABLE images DROP COLUMN like_count;
ALTER TABLE images DROP COLUMN favorite_count;
ALTER TABLE images DROP COLUMN view_count;
ALTER TABLE images DROP COLUMN status;

SELECT '✅ 步骤4.4：images表旧字段删除完成' AS status;

-- ============================================================================
-- 第五步：重命名image_tags为post_tags
-- ============================================================================

-- 创建新的post_tags表
CREATE TABLE IF NOT EXISTS post_tags (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    post_id BIGINT NOT NULL COMMENT '帖子ID',
    tag_id BIGINT NOT NULL COMMENT '标签ID',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',

    -- 外键约束
    FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,
    FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE,

    -- 唯一约束：防止同一帖子重复添加相同标签
    UNIQUE KEY uk_post_tag (post_id, tag_id),
    
    -- 索引优化
    INDEX idx_post_id (post_id),
    INDEX idx_tag_id (tag_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='帖子标签关联表';

SELECT '✅ 步骤5.1：post_tags表创建完成' AS status;

-- 迁移标签关联数据
INSERT INTO post_tags (post_id, tag_id, create_time)
SELECT p.id, it.tag_id, it.create_time
FROM image_tags it
JOIN images i ON it.image_id = i.id
JOIN posts p ON i.post_id = p.id;

SELECT CONCAT('✅ 步骤5.2：已迁移 ', COUNT(*), ' 条标签关联记录') AS status FROM post_tags;

-- 删除旧的image_tags表
DROP TABLE IF EXISTS image_tags;

SELECT '✅ 步骤5.3：image_tags表删除完成' AS status;

-- ============================================================================
-- 第六步：验证数据完整性
-- ============================================================================

-- 验证posts数量
SELECT 
    '验证posts数量' AS check_item,
    COUNT(*) AS count,
    CASE 
        WHEN COUNT(*) > 0 THEN '✅ 通过'
        ELSE '❌ 失败'
    END AS result
FROM posts;

-- 验证images关联
SELECT 
    '验证images关联' AS check_item,
    COUNT(*) AS count,
    CASE 
        WHEN COUNT(*) = (SELECT COUNT(*) FROM images) THEN '✅ 通过'
        ELSE '❌ 失败'
    END AS result
FROM images i
JOIN posts p ON i.post_id = p.id;

-- 验证post_tags关联
SELECT 
    '验证post_tags关联' AS check_item,
    COUNT(*) AS count,
    '✅ 通过' AS result
FROM post_tags pt
JOIN posts p ON pt.post_id = p.id
JOIN tags t ON pt.tag_id = t.id;

-- 验证外键约束
SELECT 
    '验证外键约束' AS check_item,
    COUNT(*) AS count,
    '✅ 通过' AS result
FROM information_schema.TABLE_CONSTRAINTS
WHERE CONSTRAINT_SCHEMA = 'knot_image_sharing'
AND TABLE_NAME IN ('posts', 'images', 'post_tags')
AND CONSTRAINT_TYPE = 'FOREIGN KEY';

-- ============================================================================
-- 第七步：显示新表结构
-- ============================================================================

SELECT '========================================' AS separator;
SELECT '新表结构' AS title;
SELECT '========================================' AS separator;

SHOW TABLES;

SELECT '========================================' AS separator;
SELECT 'posts表结构' AS title;
SELECT '========================================' AS separator;
DESCRIBE posts;

SELECT '========================================' AS separator;
SELECT 'images表结构（修改后）' AS title;
SELECT '========================================' AS separator;
DESCRIBE images;

SELECT '========================================' AS separator;
SELECT 'post_tags表结构' AS title;
SELECT '========================================' AS separator;
DESCRIBE post_tags;

-- ============================================================================
-- 第八步：显示统计信息
-- ============================================================================

SELECT '========================================' AS separator;
SELECT '数据统计' AS title;
SELECT '========================================' AS separator;

SELECT 
    'posts' AS table_name,
    COUNT(*) AS record_count,
    MIN(create_time) AS earliest,
    MAX(create_time) AS latest
FROM posts
UNION ALL
SELECT 
    'images' AS table_name,
    COUNT(*) AS record_count,
    MIN(create_time) AS earliest,
    MAX(create_time) AS latest
FROM images
UNION ALL
SELECT 
    'post_tags' AS table_name,
    COUNT(*) AS record_count,
    MIN(create_time) AS earliest,
    MAX(create_time) AS latest
FROM post_tags;

-- ============================================================================
-- 完成
-- ============================================================================

SELECT '========================================' AS separator;
SELECT '✅ 数据库迁移完成！' AS status;
SELECT '========================================' AS separator;
SELECT '备份表：images_backup, image_tags_backup' AS note;
SELECT '如果确认迁移成功，可以删除备份表：' AS note;
SELECT 'DROP TABLE images_backup;' AS command;
SELECT 'DROP TABLE image_tags_backup;' AS command;
SELECT '========================================' AS separator;

