/**
 * @file follow_service.cpp
 * @brief 关注服务实现
 * @author Knot Team
 * @date 2025-10-16
 */

#include "core/follow_service.h"
#include "database/follow_repository.h"
#include "database/user_repository.h"
#include "database/connection_pool.h"
#include "database/connection_guard.h"
#include "database/transaction_guard.h"
#include "models/user.h"
#include "utils/logger.h"

// 构造函数
FollowService::FollowService()
    : followRepo_(std::make_unique<FollowRepository>())
    , userRepo_(std::make_unique<UserRepository>()) {
}

// 析构函数
FollowService::~FollowService() {
}

// 关注用户
FollowResult FollowService::followUser(int64_t followerId, const std::string& followeeUserId) {
    FollowResult result;
    
    try {
        Logger::info("FollowService::followUser called - followerId=" + std::to_string(followerId) + 
                    ", followeeUserId=" + followeeUserId);
        
        // 1. 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.success = false;
            result.statusCode = 500;
            result.message = "数据库连接失败";
            Logger::error("Failed to get database connection");
            return result;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 2. 查询被关注者是否存在（优先按物理ID查询，失败则按逻辑ID）
        std::optional<User> followeeOpt;
        
        // 尝试作为物理ID解析
        try {
            int64_t followeePhysicalId = std::stoll(followeeUserId);
            Logger::debug("Trying to find user by physical ID: " + std::to_string(followeePhysicalId));
            followeeOpt = userRepo_->findById(followeePhysicalId);
        } catch (const std::exception& e) {
            Logger::debug("Not a valid physical ID, trying logical ID: " + followeeUserId);
        }
        
        // 如果物理ID查询失败，尝试逻辑ID
        if (!followeeOpt.has_value()) {
            Logger::debug("Trying to find user by logical ID: " + followeeUserId);
            followeeOpt = userRepo_->findByUserId(followeeUserId);
        }
        
        if (!followeeOpt.has_value()) {
            result.success = false;
            result.statusCode = 404;
            result.message = "用户不存在";
            Logger::warning("User not found: " + followeeUserId);
            return result;
        }
        
        Logger::info("Followee found: id=" + std::to_string(followeeOpt->getId()) + 
                    ", user_id=" + followeeOpt->getUserId());
        
        int64_t followeeId = followeeOpt->getId();
        
        // 3. 检查是否尝试关注自己
        if (followerId == followeeId) {
            result.success = false;
            result.statusCode = 400;
            result.message = "不能关注自己";
            Logger::warning("User trying to follow themselves: " + std::to_string(followerId));
            return result;
        }
        
        // 4. 检查是否已关注
        if (followRepo_->exists(conn, followerId, followeeId)) {
            result.success = false;
            result.statusCode = 409;
            result.message = "已经关注过该用户";
            result.isFollowing = true;
            // 获取当前粉丝数
            result.followerCount = followRepo_->countFollowers(conn, followeeId);
            Logger::info("Already following: follower=" + std::to_string(followerId) +
                        ", followee=" + std::to_string(followeeId));
            return result;
        }
        
        // 5. 开启事务
        TransactionGuard trans(conn);
        
        // 6. 创建关注记录
        if (!followRepo_->create(conn, followerId, followeeId)) {
            result.success = false;
            result.statusCode = 500;
            result.message = "创建关注记录失败";
            Logger::error("Failed to create follow record");
            return result;
        }
        
        // 7. 更新关注者的following_count
        if (!userRepo_->incrementFollowingCount(conn, followerId)) {
            result.success = false;
            result.statusCode = 500;
            result.message = "更新关注数失败";
            Logger::error("Failed to increment following_count");
            return result;
        }
        
        // 8. 更新被关注者的follower_count
        if (!userRepo_->incrementFollowerCount(conn, followeeId)) {
            result.success = false;
            result.statusCode = 500;
            result.message = "更新粉丝数失败";
            Logger::error("Failed to increment follower_count");
            return result;
        }
        
        // 9. 提交事务
        trans.commit();
        
        // 10. 返回成功结果
        result.success = true;
        result.statusCode = 200;
        result.message = "关注成功";
        result.isFollowing = true;
        result.followerCount = followRepo_->countFollowers(conn, followeeId);
        
        Logger::info("Follow success: follower=" + std::to_string(followerId) +
                    ", followee=" + std::to_string(followeeId));
        return result;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.statusCode = 500;
        result.message = "服务器内部错误";
        Logger::error("Exception in followUser: " + std::string(e.what()));
        return result;
    }
}

// 取消关注
FollowResult FollowService::unfollowUser(int64_t followerId, const std::string& followeeUserId) {
    FollowResult result;
    
    try {
        // 1. 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.success = false;
            result.statusCode = 500;
            result.message = "数据库连接失败";
            Logger::error("Failed to get database connection");
            return result;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 2. 查询被取关用户是否存在
        auto followeeOpt = userRepo_->findByUserId(followeeUserId);
        if (!followeeOpt.has_value()) {
            result.success = false;
            result.statusCode = 404;
            result.message = "用户不存在";
            Logger::warning("User not found: " + followeeUserId);
            return result;
        }
        
        int64_t followeeId = followeeOpt->getId();
        
        // 3. 检查是否已关注
        if (!followRepo_->exists(conn, followerId, followeeId)) {
            result.success = false;
            result.statusCode = 404;
            result.message = "未关注该用户";
            result.isFollowing = false;
            Logger::info("Not following: follower=" + std::to_string(followerId) +
                        ", followee=" + std::to_string(followeeId));
            return result;
        }
        
        // 4. 开启事务
        TransactionGuard trans(conn);
        
        // 5. 删除关注记录
        if (!followRepo_->deleteByFollowerAndFollowee(conn, followerId, followeeId)) {
            result.success = false;
            result.statusCode = 500;
            result.message = "删除关注记录失败";
            Logger::error("Failed to delete follow record");
            return result;
        }
        
        // 6. 更新关注者的following_count
        if (!userRepo_->decrementFollowingCount(conn, followerId)) {
            result.success = false;
            result.statusCode = 500;
            result.message = "更新关注数失败";
            Logger::error("Failed to decrement following_count");
            return result;
        }
        
        // 7. 更新被关注者的follower_count
        if (!userRepo_->decrementFollowerCount(conn, followeeId)) {
            result.success = false;
            result.statusCode = 500;
            result.message = "更新粉丝数失败";
            Logger::error("Failed to decrement follower_count");
            return result;
        }
        
        // 8. 提交事务
        trans.commit();
        
        // 9. 返回成功结果
        result.success = true;
        result.statusCode = 200;
        result.message = "取消关注成功";
        result.isFollowing = false;
        result.followerCount = followRepo_->countFollowers(conn, followeeId);
        
        Logger::info("Unfollow success: follower=" + std::to_string(followerId) +
                    ", followee=" + std::to_string(followeeId));
        return result;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.statusCode = 500;
        result.message = "服务器内部错误";
        Logger::error("Exception in unfollowUser: " + std::string(e.what()));
        return result;
    }
}

// 检查关注关系
FollowStatusResult FollowService::checkFollowStatus(int64_t followerId, const std::string& followeeUserId) {
    FollowStatusResult result;
    
    try {
        // 1. 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            result.success = false;
            result.statusCode = 500;
            result.message = "数据库连接失败";
            Logger::error("Failed to get database connection");
            return result;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 2. 查询目标用户是否存在
        auto followeeOpt = userRepo_->findByUserId(followeeUserId);
        if (!followeeOpt.has_value()) {
            result.success = false;
            result.statusCode = 404;
            result.message = "用户不存在";
            Logger::warning("User not found: " + followeeUserId);
            return result;
        }
        
        int64_t followeeId = followeeOpt->getId();
        
        // 3. 检查我是否关注该用户
        result.isFollowing = followRepo_->exists(conn, followerId, followeeId);
        
        // 4. 检查该用户是否关注我
        result.isFollowedBy = followRepo_->exists(conn, followeeId, followerId);
        
        result.success = true;
        result.statusCode = 200;
        result.message = "查询成功";
        
        return result;
        
    } catch (const std::exception& e) {
        result.success = false;
        result.statusCode = 500;
        result.message = "服务器内部错误";
        Logger::error("Exception in checkFollowStatus: " + std::string(e.what()));
        return result;
    }
}

// 获取关注列表（我关注的人）
std::vector<UserListInfo> FollowService::getFollowingList(const std::string& userId, int64_t currentUserId,
                                                             int page, int pageSize, int& total) {
    std::vector<UserListInfo> userList;
    total = 0;
    
    try {
        // 1. 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return userList;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 2. 查询用户是否存在
        auto userOpt = userRepo_->findByUserId(userId);
        if (!userOpt.has_value()) {
            Logger::warning("User not found: " + userId);
            return userList;
        }
        
        int64_t targetUserId = userOpt->getId();
        
        // 3. 计算分页参数
        int offset = (page - 1) * pageSize;

        // 4. 查询关注列表（第1次数据库查询）
        auto follows = followRepo_->findFollowingByUserId(conn, targetUserId, pageSize, offset);

        // 5. 获取总数
        total = followRepo_->countFollowing(conn, targetUserId);

        // 如果没有关注任何人，直接返回
        if (follows.empty()) {
            Logger::debug("User " + userId + " is not following anyone");
            return userList;
        }

        // ========== 批量查询优化开始 ==========

        // 6. 收集所有被关注用户的ID
        std::vector<int64_t> followeeIds;
        followeeIds.reserve(follows.size());
        for (const auto& follow : follows) {
            followeeIds.push_back(follow.getFolloweeId());
        }

        // 7. 批量查询用户信息（第2次数据库查询）
        // 注意：batchGetUsers返回的User对象包含follower_count冗余字段
        // 转换int64_t到int（物理ID在数据库中是BIGINT，但UserRepository用的是int）
        std::vector<int> followeeIdsInt;
        followeeIdsInt.reserve(followeeIds.size());
        for (int64_t id : followeeIds) {
            followeeIdsInt.push_back(static_cast<int>(id));
        }
        auto userMap = userRepo_->batchGetUsers(conn, followeeIdsInt);

        Logger::debug("Batch fetched " + std::to_string(userMap.size()) + " users");

        // 8. 批量查询关注状态（第3次数据库查询）
        std::map<int64_t, bool> followingMap;
        if (currentUserId > 0) {
            followingMap = followRepo_->batchCheckExists(conn, currentUserId, followeeIds);
            Logger::debug("Batch checked follow status for " +
                         std::to_string(followeeIds.size()) + " users");
        }

        // 9. 组装用户列表（纯内存操作，无数据库查询）
        userList.reserve(follows.size());
        for (const auto& follow : follows) {
            int64_t followeeId = follow.getFolloweeId();

            // 从userMap中查找用户信息（O(1)时间复杂度）
            auto userIt = userMap.find(static_cast<int>(followeeId));
            if (userIt == userMap.end()) {
                Logger::warning("User not found in batch result: " + std::to_string(followeeId));
                continue;
            }

            const User& user = userIt->second;

            UserListInfo info;
            info.userId = user.getUserId();
            info.username = user.getUsername();
            info.realName = user.getRealName();
            info.avatarUrl = user.getAvatarUrl();
            info.bio = user.getBio();

            // 使用冗余字段，无需额外查询
            info.followerCount = user.getFollowerCount();

            info.followedAt = static_cast<int64_t>(follow.getCreateTime());

            // 从followingMap中查找关注状态（O(1)时间复杂度）
            if (currentUserId > 0) {
                auto statusIt = followingMap.find(followeeId);
                info.isFollowing = (statusIt != followingMap.end() && statusIt->second);
            } else {
                info.isFollowing = false;
            }

            userList.push_back(info);
        }

        // ========== 批量查询优化结束 ==========

        Logger::info("Successfully fetched " + std::to_string(userList.size()) +
                    " following users for user " + userId +
                    " (page " + std::to_string(page) + ", total " + std::to_string(total) + ")");
        return userList;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in getFollowingList: " + std::string(e.what()));
        return userList;
    }
}

// 获取粉丝列表（关注我的人）
std::vector<UserListInfo> FollowService::getFollowerList(const std::string& userId, int64_t currentUserId,
                                                           int page, int pageSize, int& total) {
    std::vector<UserListInfo> userList;
    total = 0;
    
    try {
        // 1. 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return userList;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 2. 查询用户是否存在
        auto userOpt = userRepo_->findByUserId(userId);
        if (!userOpt.has_value()) {
            Logger::warning("User not found: " + userId);
            return userList;
        }
        
        int64_t targetUserId = userOpt->getId();
        
        // 3. 计算分页参数
        int offset = (page - 1) * pageSize;

        // 4. 查询粉丝列表（第1次数据库查询）
        auto follows = followRepo_->findFollowersByUserId(conn, targetUserId, pageSize, offset);

        // 5. 获取总数
        total = followRepo_->countFollowers(conn, targetUserId);

        // 如果没有粉丝，直接返回
        if (follows.empty()) {
            Logger::debug("User " + userId + " has no followers");
            return userList;
        }

        // ========== 批量查询优化开始 ==========

        // 6. 收集所有粉丝用户的ID
        std::vector<int64_t> followerIds;
        followerIds.reserve(follows.size());
        for (const auto& follow : follows) {
            followerIds.push_back(follow.getFollowerId());
        }

        // 7. 批量查询用户信息（第2次数据库查询）
        std::vector<int> followerIdsInt;
        followerIdsInt.reserve(followerIds.size());
        for (int64_t id : followerIds) {
            followerIdsInt.push_back(static_cast<int>(id));
        }
        auto userMap = userRepo_->batchGetUsers(conn, followerIdsInt);

        Logger::debug("Batch fetched " + std::to_string(userMap.size()) + " followers");

        // 8. 批量查询关注状态（第3次数据库查询）
        std::map<int64_t, bool> followingMap;
        if (currentUserId > 0) {
            followingMap = followRepo_->batchCheckExists(conn, currentUserId, followerIds);
            Logger::debug("Batch checked follow status for " +
                         std::to_string(followerIds.size()) + " users");
        }

        // 9. 组装用户列表（纯内存操作）
        userList.reserve(follows.size());
        for (const auto& follow : follows) {
            int64_t followerId = follow.getFollowerId();

            auto userIt = userMap.find(static_cast<int>(followerId));
            if (userIt == userMap.end()) {
                Logger::warning("User not found in batch result: " + std::to_string(followerId));
                continue;
            }

            const User& user = userIt->second;

            UserListInfo info;
            info.userId = user.getUserId();
            info.username = user.getUsername();
            info.realName = user.getRealName();
            info.avatarUrl = user.getAvatarUrl();
            info.bio = user.getBio();
            info.followerCount = user.getFollowerCount();
            info.followedAt = static_cast<int64_t>(follow.getCreateTime());

            // 检查我是否关注该粉丝
            if (currentUserId > 0) {
                auto statusIt = followingMap.find(followerId);
                info.isFollowing = (statusIt != followingMap.end() && statusIt->second);
            } else {
                info.isFollowing = false;
            }

            userList.push_back(info);
        }

        // ========== 批量查询优化结束 ==========

        Logger::info("Successfully fetched " + std::to_string(userList.size()) +
                    " followers for user " + userId +
                    " (page " + std::to_string(page) + ", total " + std::to_string(total) + ")");
        return userList;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in getFollowerList: " + std::string(e.what()));
        return userList;
    }
}

// 获取互关列表（互相关注的人）
std::vector<UserListInfo> FollowService::getMutualFollowList(const std::string& userId, int64_t currentUserId,
                                                               int page, int pageSize, int& total) {
    std::vector<UserListInfo> userList;
    total = 0;

    try {
        // 1. 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return userList;
        }

        MYSQL* conn = connGuard.get();

        // 2. 查询用户是否存在
        auto userOpt = userRepo_->findByUserId(userId);
        if (!userOpt.has_value()) {
            Logger::warning("User not found: " + userId);
            return userList;
        }

        int64_t targetUserId = userOpt->getId();

        // 3. 计算分页参数
        int offset = (page - 1) * pageSize;

        // 4. 查询互关用户ID列表（第1次数据库查询）
        auto mutualFollowIds = followRepo_->findMutualFollowIds(conn, targetUserId, pageSize, offset);

        // 5. 获取总数
        total = followRepo_->countMutualFollows(conn, targetUserId);

        // 如果没有互关好友，直接返回
        if (mutualFollowIds.empty()) {
            Logger::debug("User " + userId + " has no mutual follows");
            return userList;
        }

        // ========== 批量查询优化 ==========

        // 6. 批量查询用户信息（第2次数据库查询）
        std::vector<int> mutualFollowIdsInt;
        mutualFollowIdsInt.reserve(mutualFollowIds.size());
        for (int64_t id : mutualFollowIds) {
            mutualFollowIdsInt.push_back(static_cast<int>(id));
        }
        auto userMap = userRepo_->batchGetUsers(conn, mutualFollowIdsInt);

        Logger::debug("Batch fetched " + std::to_string(userMap.size()) + " mutual follow users");

        // 7. 组装用户列表（纯内存操作）
        userList.reserve(mutualFollowIds.size());
        for (int64_t mutualUserId : mutualFollowIds) {
            auto userIt = userMap.find(static_cast<int>(mutualUserId));
            if (userIt == userMap.end()) {
                Logger::warning("User not found in batch result: " + std::to_string(mutualUserId));
                continue;
            }

            const User& user = userIt->second;

            UserListInfo info;
            info.userId = user.getUserId();
            info.username = user.getUsername();
            info.realName = user.getRealName();
            info.avatarUrl = user.getAvatarUrl();
            info.bio = user.getBio();
            info.followerCount = user.getFollowerCount();
            info.followedAt = 0;  // 互关列表不需要关注时间

            // 互关列表中的用户，is_following固定为true（因为是双向关注）
            info.isFollowing = true;

            userList.push_back(info);
        }

        Logger::info("Successfully fetched " + std::to_string(userList.size()) +
                    " mutual follows for user " + userId +
                    " (page " + std::to_string(page) + ", total " + std::to_string(total) + ")");

        return userList;

    } catch (const std::exception& e) {
        Logger::error("Exception in getMutualFollowList: " + std::string(e.what()));
        return userList;
    }
}

// 获取用户统计信息
std::optional<UserStats> FollowService::getUserStats(const std::string& userId) {
    try {
        // 1. 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return std::nullopt;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 2. 调用Repository获取统计信息
        return userRepo_->getUserStats(conn, userId);
        
    } catch (const std::exception& e) {
        Logger::error("Exception in getUserStats: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 批量检查关注关系
std::map<std::string, bool> FollowService::batchCheckFollowStatus(int64_t followerId,
                                                                    const std::vector<std::string>& followeeUserIds) {
    std::map<std::string, bool> result;
    
    try {
        if (followeeUserIds.empty()) {
            return result;
        }
        
        // 1. 获取数据库连接
        ConnectionGuard connGuard(DatabaseConnectionPool::getInstance());
        if (!connGuard.isValid()) {
            Logger::error("Failed to get database connection");
            return result;
        }
        
        MYSQL* conn = connGuard.get();
        
        // 2. 将业务ID转换为物理ID
        std::vector<int64_t> followeeIds;
        std::map<int64_t, std::string> idMapping;  // 物理ID -> 业务ID 映射
        
        for (const auto& userIdStr : followeeUserIds) {
            auto userOpt = userRepo_->findByUserId(userIdStr);
            if (userOpt.has_value()) {
                int64_t physicalId = userOpt->getId();
                followeeIds.push_back(physicalId);
                idMapping[physicalId] = userIdStr;
                // 初始化为false
                result[userIdStr] = false;
            }
        }
        
        // 3. 批量查询关注关系
        if (!followeeIds.empty()) {
            auto followMap = followRepo_->batchCheckExists(conn, followerId, followeeIds);
            
            // 4. 将结果从物理ID映射回业务ID
            for (const auto& pair : followMap) {
                int64_t physicalId = pair.first;
                bool isFollowing = pair.second;
                
                if (idMapping.find(physicalId) != idMapping.end()) {
                    std::string userIdStr = idMapping[physicalId];
                    result[userIdStr] = isFollowing;
                }
            }
        }
        
        Logger::debug("Batch checked " + std::to_string(followeeUserIds.size()) + " follow relationships");
        return result;
        
    } catch (const std::exception& e) {
        Logger::error("Exception in batchCheckFollowStatus: " + std::string(e.what()));
        return result;
    }
}
