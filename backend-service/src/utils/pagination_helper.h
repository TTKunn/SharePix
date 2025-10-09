/**
 * @file pagination_helper.h
 * @brief 分页工具类
 * @author Knot Team
 * @date 2025-10-08
 */

#pragma once

#include <json/json.h>
#include <vector>
#include <cmath>

/**
 * @brief 分页信息结构体
 */
struct PaginationInfo {
    int page;           ///< 当前页码（从1开始）
    int pageSize;       ///< 每页数量
    int totalItems;     ///< 总记录数
    int totalPages;     ///< 总页数
    bool hasNext;       ///< 是否有下一页
    bool hasPrev;       ///< 是否有上一页
    
    /**
     * @brief 构造函数
     */
    PaginationInfo() 
        : page(1), pageSize(20), totalItems(0), totalPages(0), 
          hasNext(false), hasPrev(false) {}
    
    /**
     * @brief 计算总页数和分页状态
     */
    void calculate() {
        if (pageSize <= 0) {
            totalPages = 0;
            hasNext = false;
            hasPrev = false;
            return;
        }
        
        totalPages = (totalItems + pageSize - 1) / pageSize;  // 向上取整
        hasNext = page < totalPages;
        hasPrev = page > 1;
    }
    
    /**
     * @brief 转换为JSON对象
     * @return JSON对象
     */
    Json::Value toJson() const {
        Json::Value json;
        json["page"] = page;
        json["page_size"] = pageSize;
        json["total_items"] = totalItems;
        json["total_pages"] = totalPages;
        json["has_next"] = hasNext;
        json["has_prev"] = hasPrev;
        return json;
    }
};

/**
 * @brief 分页工具类
 */
class PaginationHelper {
public:
    /**
     * @brief 验证分页参数
     * @param page 页码
     * @param pageSize 每页数量
     * @param maxPageSize 最大每页数量（默认100）
     * @return 成功返回true，失败返回false
     */
    static bool validate(int page, int pageSize, int maxPageSize = 100) {
        return page >= 1 && pageSize >= 1 && pageSize <= maxPageSize;
    }
    
    /**
     * @brief 计算OFFSET值
     * @param page 页码（从1开始）
     * @param pageSize 每页数量
     * @return OFFSET值
     */
    static int calculateOffset(int page, int pageSize) {
        return (page - 1) * pageSize;
    }
    
    /**
     * @brief 创建分页信息
     * @param page 当前页码
     * @param pageSize 每页数量
     * @param totalItems 总记录数
     * @return 分页信息对象
     */
    static PaginationInfo createInfo(int page, int pageSize, int totalItems) {
        PaginationInfo info;
        info.page = page;
        info.pageSize = pageSize;
        info.totalItems = totalItems;
        info.calculate();
        return info;
    }
    
    /**
     * @brief 创建分页响应JSON
     * @param items 数据列表（JSON数组）
     * @param pagination 分页信息
     * @return 包含数据和分页信息的JSON对象
     */
    static Json::Value createResponse(const Json::Value& items, 
                                      const PaginationInfo& pagination) {
        Json::Value response;
        response["items"] = items;
        response["pagination"] = pagination.toJson();
        return response;
    }
    
    /**
     * @brief 规范化页码（确保在有效范围内）
     * @param page 页码
     * @param totalPages 总页数
     * @return 规范化后的页码
     */
    static int normalizePage(int page, int totalPages) {
        if (page < 1) return 1;
        if (totalPages > 0 && page > totalPages) return totalPages;
        return page;
    }
    
    /**
     * @brief 规范化每页数量（确保在有效范围内）
     * @param pageSize 每页数量
     * @param minPageSize 最小每页数量（默认1）
     * @param maxPageSize 最大每页数量（默认100）
     * @return 规范化后的每页数量
     */
    static int normalizePageSize(int pageSize, 
                                 int minPageSize = 1, 
                                 int maxPageSize = 100) {
        if (pageSize < minPageSize) return minPageSize;
        if (pageSize > maxPageSize) return maxPageSize;
        return pageSize;
    }
};

