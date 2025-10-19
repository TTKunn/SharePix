-- Knot Image Sharing Service Database Schema
-- Created: 2024-01-01
-- Version: 1.0.0

-- Create database if not exists
CREATE DATABASE IF NOT EXISTS knot_image_sharing
CHARACTER SET utf8mb4
COLLATE utf8mb4_unicode_ci;

USE knot_image_sharing;

-- Drop old tables if exists
DROP TABLE IF EXISTS login_attempts;
DROP TABLE IF EXISTS user_profiles;
DROP TABLE IF EXISTS user_sessions;
DROP TABLE IF EXISTS users;

-- Users table for authentication
CREATE TABLE IF NOT EXISTS users (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    user_id VARCHAR(36) NOT NULL UNIQUE COMMENT '逻辑ID（业务生成，例：USR_2025Q1_01H8S2）',
    username VARCHAR(50) NOT NULL UNIQUE COMMENT '用户名',
    password VARCHAR(255) NOT NULL COMMENT '加密密码（存储 password_hash）',
    salt VARCHAR(64) NOT NULL COMMENT '密码盐值',
    real_name VARCHAR(50) NOT NULL COMMENT '真实姓名',
    phone VARCHAR(20) NOT NULL UNIQUE COMMENT '手机号',
    email VARCHAR(100) NULL COMMENT '邮箱',
    role ENUM('user', 'admin') NOT NULL DEFAULT 'user' COMMENT '用户角色',
    status ENUM('active', 'inactive', 'banned') NOT NULL DEFAULT 'active' COMMENT '账户状态',
    avatar_url VARCHAR(255) NULL COMMENT '头像URL',
    bio VARCHAR(500) NULL COMMENT '个人简介',
    gender ENUM('male', 'female', 'other', 'prefer_not_to_say') NULL COMMENT '性别',
    location VARCHAR(100) NULL COMMENT '所在地',
    device_count SMALLINT NOT NULL DEFAULT 0 COMMENT '绑定设备数',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    update_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    -- Indexes for performance
    INDEX idx_user_id (user_id),
    INDEX idx_username (username),
    INDEX idx_phone (phone),
    INDEX idx_email (email),
    INDEX idx_status (status),
    INDEX idx_role (role),
    INDEX idx_create_time (create_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='用户信息表';



-- Insert default admin user (password: admin123)
-- Note: This is for development only, change in production
INSERT IGNORE INTO users (user_id, username, password, salt, real_name, phone, email, role, status) VALUES
('USR_ADMIN_001', 'admin',
 'pbkdf2_sha256$100000$default_salt_change_this$hashed_password_placeholder',
 'default_salt_change_this',
 '系统管理员',
 '13800000000',
 'admin@example.com',
 'admin',
 'active');

-- Create composite indexes for better performance
CREATE INDEX idx_users_composite ON users(username, status);
CREATE INDEX idx_users_phone_status ON users(phone, status);

-- Images table for multi-image posts (v2.0.0+)
CREATE TABLE IF NOT EXISTS images (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    image_id VARCHAR(36) NOT NULL UNIQUE COMMENT '业务逻辑ID（例：IMG_2025Q4_ABC123）',
    user_id BIGINT NOT NULL COMMENT '上传用户ID',
    post_id BIGINT NOT NULL COMMENT '所属帖子ID',
    display_order INT NOT NULL DEFAULT 0 COMMENT '显示顺序（0-8）',
    file_url VARCHAR(500) NOT NULL COMMENT '原图URL',
    thumbnail_url VARCHAR(500) NOT NULL COMMENT '缩略图URL',
    file_size BIGINT NOT NULL COMMENT '文件大小（字节）',
    width INT COMMENT '图片宽度',
    height INT COMMENT '图片高度',
    mime_type VARCHAR(50) NOT NULL COMMENT 'MIME类型（image/jpeg, image/png, image/webp）',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    update_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    -- Foreign keys
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,

    -- Indexes for performance
    INDEX idx_image_id (image_id),
    INDEX idx_user_id (user_id),
    INDEX idx_post_id (post_id),
    INDEX idx_post_order (post_id, display_order),
    INDEX idx_create_time (create_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='图片信息表（多图片帖子支持）';

-- Tags table for image tagging
CREATE TABLE IF NOT EXISTS tags (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    name VARCHAR(50) NOT NULL UNIQUE COMMENT '标签名称',
    use_count INT NOT NULL DEFAULT 0 COMMENT '使用次数',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',

    -- Indexes for performance
    INDEX idx_name (name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='标签表';

-- Image-Tag relationship table
CREATE TABLE IF NOT EXISTS image_tags (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    image_id BIGINT NOT NULL COMMENT '图片ID',
    tag_id BIGINT NOT NULL COMMENT '标签ID',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',

    -- Foreign keys
    FOREIGN KEY (image_id) REFERENCES images(id) ON DELETE CASCADE,
    FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE,

    -- Unique constraint to prevent duplicate tags on same image
    UNIQUE KEY uk_image_tag (image_id, tag_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='图片标签关联表';

-- Posts table for multi-image posts (v2.0.0)
CREATE TABLE IF NOT EXISTS posts (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    post_id VARCHAR(36) NOT NULL UNIQUE COMMENT '业务逻辑ID（例：POST_2025Q4_ABC123）',
    user_id BIGINT NOT NULL COMMENT '发布用户ID',
    title VARCHAR(255) NOT NULL COMMENT '帖子标题',
    description TEXT COMMENT '帖子配文',
    image_count INT NOT NULL DEFAULT 0 COMMENT '图片数量（1-9张）',
    like_count INT NOT NULL DEFAULT 0 COMMENT '点赞数（冗余字段）',
    favorite_count INT NOT NULL DEFAULT 0 COMMENT '收藏数（冗余字段）',
    comment_count INT NOT NULL DEFAULT 0 COMMENT '评论数（冗余字段）',
    view_count INT NOT NULL DEFAULT 0 COMMENT '浏览数',
    status ENUM('PENDING', 'APPROVED', 'REJECTED') NOT NULL DEFAULT 'APPROVED' COMMENT '审核状态',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    update_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',

    -- Foreign key
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,

    -- Indexes for performance
    INDEX idx_post_id (post_id),
    INDEX idx_user_id (user_id),
    INDEX idx_create_time (create_time),
    INDEX idx_status (status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='帖子表';

-- Likes table for post likes (v2.2.0)
CREATE TABLE IF NOT EXISTS likes (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    user_id BIGINT NOT NULL COMMENT '点赞用户ID',
    post_id BIGINT NOT NULL COMMENT '被点赞帖子ID（关联posts表的物理ID）',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '点赞时间',

    -- Foreign keys
    CONSTRAINT fk_likes_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    CONSTRAINT fk_likes_post FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,

    -- Unique constraint to prevent duplicate likes
    UNIQUE KEY uk_user_post (user_id, post_id),

    -- Indexes for performance
    INDEX idx_post_id (post_id),
    INDEX idx_user_create (user_id, create_time DESC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='点赞表';

-- Favorites table for post favorites (v2.2.0)
CREATE TABLE IF NOT EXISTS favorites (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    user_id BIGINT NOT NULL COMMENT '收藏用户ID',
    post_id BIGINT NOT NULL COMMENT '被收藏帖子ID（关联posts表的物理ID）',
    create_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '收藏时间',

    -- Foreign keys
    CONSTRAINT fk_favorites_user FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    CONSTRAINT fk_favorites_post FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,

    -- Unique constraint to prevent duplicate favorites
    UNIQUE KEY uk_user_post (user_id, post_id),

    -- Indexes for performance
    INDEX idx_post_id (post_id),
    INDEX idx_user_create (user_id, create_time DESC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='收藏表';

-- Comments table for post comments (v2.8.0)
CREATE TABLE IF NOT EXISTS comments (
    id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '物理ID（自增主键）',
    comment_id VARCHAR(36) NOT NULL COMMENT '业务逻辑ID（例：CMT_2025Q4_ABC123）',
    post_id BIGINT NOT NULL COMMENT '所属帖子ID',
    user_id BIGINT NOT NULL COMMENT '评论用户ID',
    content TEXT NOT NULL COMMENT '评论内容（最多1000字符）',
    create_time TIMESTAMP NULL DEFAULT CURRENT_TIMESTAMP COMMENT '评论时间',

    -- Foreign keys
    CONSTRAINT comments_ibfk_1 FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    CONSTRAINT comments_ibfk_2 FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE,

    -- Indexes for performance
    UNIQUE KEY uk_comment_id (comment_id),
    INDEX idx_post_id (post_id),
    INDEX idx_user_id (user_id),
    INDEX idx_post_create (post_id, create_time DESC)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='评论表';

-- Show table structure
SHOW TABLES;
DESCRIBE users;
DESCRIBE images;
DESCRIBE tags;
DESCRIBE image_tags;
DESCRIBE posts;
DESCRIBE likes;
DESCRIBE favorites;
DESCRIBE comments;
