/**
 * @file share_repository.h
 * @brief 分享数据访问层定义
 * @author Claude Code Assistant
 * @date 2025-10-22
 * @version v2.10.0
 */

#pragma once

#include "models/share.h"
#include <mysql/mysql.h>
#include <optional>
#include <vector>
#include <string>

/**
 * @brief 分享数据访问类
 *
 * 负责分享记录的CRUD操作
 * 设计原则：仅操作shares表，不处理关联数据的查询
 */
class ShareRepository {
public:
    /**
     * @brief 构造函数
     */
    ShareRepository() = default;

    /**
     * @brief 析构函数
     */
    ~ShareRepository() = default;

    /**
     * @brief 创建分享记录
     * @param conn MySQL连接
     * @param share 分享对象（包含shareId、postId、senderId、receiverId、shareMessage）
     * @return 成功返回插入的分享记录ID（物理ID），失败返回0
     */
    int create(MYSQL* conn, const Share& share);

    /**
     * @brief 根据ID查询分享记录
     * @param conn MySQL连接
     * @param shareId 分享物理ID
     * @return 找到返回Share对象，否则返回nullopt
     */
    std::optional<Share> findById(MYSQL* conn, int shareId);

    /**
     * @brief 根据业务ID查询分享记录
     * @param conn MySQL连接
     * @param shareId 分享业务ID（SHR_xxx格式）
     * @return 找到返回Share对象，否则返回nullopt
     */
    std::optional<Share> findByShareId(MYSQL* conn, const std::string& shareId);

    /**
     * @brief 检查分享记录是否已存在（防重复分享）
     * @param conn MySQL连接
     * @param senderId 发送者ID（物理ID）
     * @param receiverId 接收者ID（物理ID）
     * @param postId 帖子ID（物理ID）
     * @return 存在返回true，否则返回false
     */
    bool exists(MYSQL* conn, int senderId, int receiverId, int postId);

    /**
     * @brief 查询接收到的分享列表
     * @param conn MySQL连接
     * @param receiverId 接收者ID（物理ID）
     * @param limit 每页数量
     * @param offset 偏移量
     * @return 分享列表（仅包含基础Share对象，不含帖子和用户详情）
     */
    std::vector<Share> findReceivedShares(MYSQL* conn, int receiverId, int limit, int offset);

    /**
     * @brief 查询发送出的分享列表
     * @param conn MySQL连接
     * @param senderId 发送者ID（物理ID）
     * @param limit 每页数量
     * @param offset 偏移量
     * @return 分享列表（仅包含基础Share对象，不含帖子和用户详情）
     */
    std::vector<Share> findSentShares(MYSQL* conn, int senderId, int limit, int offset);

    /**
     * @brief 统计接收到的分享数量
     * @param conn MySQL连接
     * @param receiverId 接收者ID（物理ID）
     * @return 分享数量
     */
    int countReceivedShares(MYSQL* conn, int receiverId);

    /**
     * @brief 统计发送出的分享数量
     * @param conn MySQL连接
     * @param senderId 发送者ID（物理ID）
     * @return 分享数量
     */
    int countSentShares(MYSQL* conn, int senderId);

    /**
     * @brief 删除分享记录
     * @param conn MySQL连接
     * @param shareId 分享物理ID
     * @return 成功返回true，失败返回false
     */
    bool deleteById(MYSQL* conn, int shareId);

    /**
     * @brief 统计帖子的分享次数
     * @param conn MySQL连接
     * @param postId 帖子ID（物理ID）
     * @return 分享次数
     */
    int countPostShares(MYSQL* conn, int postId);

private:
    /**
     * @brief 从MYSQL_ROW构建Share对象
     * @param row 数据库查询结果行
     * @return Share对象
     */
    Share buildShareFromRow(MYSQL_ROW row);
};
