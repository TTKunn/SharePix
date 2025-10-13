-- Knot Image Sharing Service Database Migration
-- Fix Images Table Schema for Multi-Image Posts Support
-- Created: 2025-10-13
-- Purpose: Add missing post_id and display_order fields to images table

USE knot_image_sharing;

-- Check if the table already has the new columns
SET @column_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND COLUMN_NAME = 'post_id'
);

-- Add post_id column if it doesn't exist
SET @sql = IF(@column_exists = 0,
    'ALTER TABLE images ADD COLUMN post_id BIGINT NOT NULL DEFAULT 0 COMMENT ''所属帖子ID'' AFTER user_id',
    'SELECT ''post_id column already exists'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Check if display_order column exists
SET @column_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND COLUMN_NAME = 'display_order'
);

-- Add display_order column if it doesn't exist
SET @sql = IF(@column_exists = 0,
    'ALTER TABLE images ADD COLUMN display_order INT NOT NULL DEFAULT 0 COMMENT ''显示顺序（0-8）'' AFTER post_id',
    'SELECT ''display_order column already exists'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Remove old columns that are no longer needed (from single image system)
-- Check if title column exists (from old single image system)
SET @column_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND COLUMN_NAME = 'title'
);

SET @sql = IF(@column_exists > 0,
    'ALTER TABLE images DROP COLUMN title',
    'SELECT ''title column already removed'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Check if description column exists (from old single image system)
SET @column_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND COLUMN_NAME = 'description'
);

SET @sql = IF(@column_exists > 0,
    'ALTER TABLE images DROP COLUMN description',
    'SELECT ''description column already removed'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Check if like_count column exists (moved to posts table)
SET @column_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND COLUMN_NAME = 'like_count'
);

SET @sql = IF(@column_exists > 0,
    'ALTER TABLE images DROP COLUMN like_count',
    'SELECT ''like_count column already removed'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Check if favorite_count column exists (moved to posts table)
SET @column_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND COLUMN_NAME = 'favorite_count'
);

SET @sql = IF(@column_exists > 0,
    'ALTER TABLE images DROP COLUMN favorite_count',
    'SELECT ''favorite_count column already removed'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Check if view_count column exists (moved to posts table)
SET @column_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND COLUMN_NAME = 'view_count'
);

SET @sql = IF(@column_exists > 0,
    'ALTER TABLE images DROP COLUMN view_count',
    'SELECT ''view_count column already removed'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Check if status column exists (moved to posts table)
SET @column_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.COLUMNS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND COLUMN_NAME = 'status'
);

SET @sql = IF(@column_exists > 0,
    'ALTER TABLE images DROP COLUMN status',
    'SELECT ''status column already removed'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Add new indexes for performance
SET @index_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.STATISTICS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND INDEX_NAME = 'idx_post_id'
);

SET @sql = IF(@index_exists = 0,
    'CREATE INDEX idx_post_id ON images(post_id)',
    'SELECT ''idx_post_id index already exists'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @index_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.STATISTICS
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND INDEX_NAME = 'idx_post_order'
);

SET @sql = IF(@index_exists = 0,
    'CREATE INDEX idx_post_order ON images(post_id, display_order)',
    'SELECT ''idx_post_order index already exists'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Add foreign key constraint
SET @constraint_exists = (
    SELECT COUNT(*)
    FROM INFORMATION_SCHEMA.KEY_COLUMN_USAGE
    WHERE TABLE_SCHEMA = 'knot_image_sharing'
    AND TABLE_NAME = 'images'
    AND COLUMN_NAME = 'post_id'
    AND REFERENCED_TABLE_NAME = 'posts'
);

SET @sql = IF(@constraint_exists = 0,
    'ALTER TABLE images ADD CONSTRAINT fk_images_post FOREIGN KEY (post_id) REFERENCES posts(id) ON DELETE CASCADE',
    'SELECT ''fk_images_post constraint already exists'' as message'
);
PREPARE stmt FROM @sql;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

-- Show final table structure
SELECT 'Images table schema after migration:' as message;
DESCRIBE images;

SELECT 'Migration completed successfully!' as message;