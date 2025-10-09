/**
 * @file tag_repository.h
 * @brief 标签数据访问层定义
 * @author Knot Team
 * @date 2025-10-07
 */

#pragma once

#include "models/tag.h"
#include <optional>
#include <vector>
#include <string>

/**
 * @brief 标签数据访问类
 * 
 * 负责标签数据的CRUD操作
 */
class TagRepository {
public:
    /**
     * @brief 构造函数
     */
    TagRepository();
    
    /**
     * @brief 析构函数
     */
    ~TagRepository() = default;
    
    /**
     * @brief 根据名称查找标签
     * @param name 标签名称
     * @return 如果找到返回Tag对象，否则返回std::nullopt
     */
    std::optional<Tag> findByName(const std::string& name);
    
    /**
     * @brief 创建标签
     * @param tag 标签对象
     * @return 成功返回true，失败返回false
     */
    bool createTag(const Tag& tag);
    
    /**
     * @brief 关联帖子和标签
     * @param postId 帖子物理ID
     * @param tagId 标签物理ID
     * @return 成功返回true，失败返回false
     */
    bool linkPostTag(int postId, int tagId);

    /**
     * @brief 获取帖子的所有标签
     * @param postId 帖子物理ID
     * @return 标签列表
     */
    std::vector<Tag> getPostTags(int postId);

    /**
     * @brief 关联图片和标签（已废弃，保留用于兼容）
     * @param imageId 图片物理ID
     * @param tagId 标签物理ID
     * @return 成功返回true，失败返回false
     * @deprecated 使用linkPostTag代替
     */
    bool linkImageTag(int imageId, int tagId);

    /**
     * @brief 获取图片的所有标签（已废弃，保留用于兼容）
     * @param imageId 图片物理ID
     * @return 标签列表
     * @deprecated 使用getPostTags代替
     */
    std::vector<Tag> getImageTags(int imageId);
    
    /**
     * @brief 增加标签使用次数
     * @param tagId 标签物理ID
     * @return 成功返回true，失败返回false
     */
    bool incrementUseCount(int tagId);

private:
    /**
     * @brief 从预编译语句构建Tag对象
     * @param stmtPtr MYSQL_STMT指针
     * @return Tag对象
     */
    Tag buildTagFromStatement(void* stmtPtr);
};

