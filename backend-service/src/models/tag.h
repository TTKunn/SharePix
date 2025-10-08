/**
 * @file tag.h
 * @brief 标签模型定义
 * @author Knot Team
 * @date 2025-10-07
 */

#pragma once

#include <string>
#include <ctime>
#include <json/json.h>

/**
 * @brief 标签模型类
 */
class Tag {
public:
    /**
     * @brief 默认构造函数
     */
    Tag();
    
    /**
     * @brief 带参数的构造函数
     * @param id 物理ID（自增主键）
     * @param name 标签名称
     */
    Tag(int id, const std::string& name);
    
    /**
     * @brief 析构函数
     */
    ~Tag() = default;
    
    // Getters
    int getId() const { return id_; }
    const std::string& getName() const { return name_; }
    int getUseCount() const { return useCount_; }
    std::time_t getCreateTime() const { return createTime_; }
    
    // Setters
    void setId(int id) { id_ = id; }
    void setName(const std::string& name) { name_ = name; }
    void setUseCount(int useCount) { useCount_ = useCount; }
    void setCreateTime(std::time_t createTime) { createTime_ = createTime; }
    
    /**
     * @brief 增加使用次数
     */
    void incrementUseCount() { useCount_++; }
    
    /**
     * @brief 将标签对象转换为JSON
     * @return JSON表示
     */
    Json::Value toJson() const;
    
    /**
     * @brief 从JSON创建标签对象
     * @param j JSON对象
     * @return Tag对象
     */
    static Tag fromJson(const Json::Value& j);
    
    /**
     * @brief 验证标签数据
     * @return 验证错误信息（如果有效则为空字符串）
     */
    std::string validate() const;

private:
    int id_;                      // 物理ID（自增主键）
    std::string name_;            // 标签名称
    int useCount_;                // 使用次数
    std::time_t createTime_;      // 创建时间
};

