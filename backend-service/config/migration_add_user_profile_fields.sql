-- 用户信息管理功能完善 - 数据库迁移脚本
-- 创建时间: 2025-10-09
-- 说明: 为users表添加个人简介、性别、所在地字段

USE knot_image_sharing;

-- 添加新字段
ALTER TABLE users ADD COLUMN bio VARCHAR(500) NULL COMMENT '个人简介' AFTER avatar_url;
ALTER TABLE users ADD COLUMN gender ENUM('male', 'female', 'other', 'prefer_not_to_say') NULL COMMENT '性别' AFTER bio;
ALTER TABLE users ADD COLUMN location VARCHAR(100) NULL COMMENT '所在地' AFTER gender;

-- 验证字段添加成功
DESC users;

