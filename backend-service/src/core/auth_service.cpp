/**
 * @file auth_service.cpp
 * @brief 认证服务实现
 * @author Knot Team
 * @date 2025-10-12
 */

#include "core/auth_service.h"
#include "database/user_repository.h"
#include "security/jwt_manager.h"
#include "security/password_hasher.h"
#include "utils/logger.h"
#include <regex>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

// 构造函数
AuthService::AuthService() {
    userRepo_ = std::make_unique<UserRepository>();
    jwtManager_ = std::make_unique<JWTManager>();
    Logger::info("AuthService initialized");
}

// 析构函数
AuthService::~AuthService() {
    Logger::info("AuthService destroyed");
}

// 用户注册
RegisterResult AuthService::registerUser(
    const std::string& username,
    const std::string& password,
    const std::string& realName,
    const std::string& phone,
    const std::string& email,
    UserRole role
) {
    RegisterResult result;

    try {
        Logger::info("Attempting to register user: " + username);

        // 1. 验证用户名格式
        if (!validateUsername(username)) {
            result.message = "用户名格式无效（3-50字符，字母数字下划线）";
            Logger::warning(result.message);
            return result;
        }

        // 2. 验证密码格式
        if (!validatePassword(password)) {
            result.message = "密码格式无效（8-128字符）";
            Logger::warning(result.message);
            return result;
        }

        // 3. 验证手机号格式
        if (!validatePhone(phone)) {
            result.message = "手机号格式无效";
            Logger::warning(result.message);
            return result;
        }

        // 4. 验证邮箱格式（如果提供）
        if (!email.empty() && !validateEmail(email)) {
            result.message = "邮箱格式无效";
            Logger::warning(result.message);
            return result;
        }

        // 5. 验证真实姓名
        if (realName.empty() || realName.length() > 50) {
            result.message = "真实姓名无效";
            Logger::warning(result.message);
            return result;
        }

        // 6. 检查用户名是否已存在
        if (userRepo_->usernameExists(username)) {
            result.message = "用户名已存在";
            Logger::warning(result.message + ": " + username);
            return result;
        }

        // 7. 检查手机号是否已存在
        if (userRepo_->phoneExists(phone)) {
            result.message = "手机号已注册";
            Logger::warning(result.message + ": " + phone);
            return result;
        }

        // 8. 检查邮箱是否已存在（如果提供了邮箱）
        if (!email.empty() && userRepo_->emailExists(email)) {
            result.message = "邮箱已注册";
            Logger::warning(result.message + ": " + email);
            return result;
        }

        // 9. 生成唯一用户ID
        std::string userId = generateUserId();

        // 10. 生成盐值
        std::string salt = PasswordHasher::generateSalt();
        if (salt.empty()) {
            result.message = "生成盐值失败";
            Logger::error(result.message);
            return result;
        }

        // 11. 哈希密码
        std::string passwordHash = PasswordHasher::hashPassword(password, salt);
        if (passwordHash.empty()) {
            result.message = "密码哈希失败";
            Logger::error(result.message);
            return result;
        }

        // 12. 构建新用户对象
        User newUser;
        newUser.setUserId(userId);
        newUser.setUsername(username);
        newUser.setPassword(passwordHash);
        newUser.setSalt(salt);
        newUser.setRealName(realName);
        newUser.setPhone(phone);
        newUser.setEmail(email);
        newUser.setRole(role);
        newUser.setStatus(UserStatus::ACTIVE);

        // 13. 创建用户记录
        if (!userRepo_->createUser(newUser)) {
            result.message = "创建用户失败";
            Logger::error(result.message);
            return result;
        }

        // 14. 查询刚创建的用户（获取自增ID）
        auto createdUser = userRepo_->findByUserId(userId);
        if (!createdUser.has_value()) {
            result.message = "查询创建的用户失败";
            Logger::error(result.message);
            return result;
        }

        Logger::info("User registered successfully: " + userId);

        // 15. 清除敏感信息后返回
        createdUser->setPassword("");
        createdUser->setSalt("");

        result.success = true;
        result.message = "注册成功";
        result.user = *createdUser;
        return result;

    } catch (const std::exception& e) {
        result.message = "注册异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

// 用户登录
AuthResult AuthService::loginUser(
    const std::string& username,
    const std::string& password
) {
    AuthResult result;

    try {
        Logger::info("User login attempt: " + username);

        // 1. 查询用户（尝试三种方式）
        std::optional<User> user;

        // 首先尝试用户名
        user = userRepo_->findByUsername(username);

        // 如果没找到，尝试手机号
        if (!user.has_value()) {
            user = userRepo_->findByPhone(username);
        }

        // 如果还没找到，尝试邮箱
        if (!user.has_value()) {
            user = userRepo_->findByEmail(username);
        }

        // 用户不存在
        if (!user.has_value()) {
            result.message = "用户不存在";
            Logger::warning(result.message + ": " + username);
            return result;
        }

        // 2. 验证密码
        if (!PasswordHasher::verifyPassword(password, user->getSalt(), user->getPassword())) {
            result.message = "密码错误";
            Logger::warning(result.message + " for user: " + username);
            return result;
        }

        // 3. 检查账户状态
        if (user->getStatus() != UserStatus::ACTIVE) {
            result.message = "账户未激活或已禁用";
            Logger::warning("User account is not active: " + username);
            return result;
        }

        // 4. 生成令牌
        std::string accessToken = jwtManager_->generateAccessToken(user->getId(), user->getUsername());
        std::string refreshToken = jwtManager_->generateRefreshToken(user->getId(), user->getUsername());

        if (accessToken.empty() || refreshToken.empty()) {
            result.message = "生成令牌失败";
            Logger::error(result.message + " for user: " + username);
            return result;
        }

        Logger::info("User logged in successfully: " + username);

        // 5. 清除敏感信息
        user->setPassword("");
        user->setSalt("");

        result.success = true;
        result.message = "登录成功";
        result.accessToken = accessToken;
        result.refreshToken = refreshToken;
        result.user = *user;
        return result;

    } catch (const std::exception& e) {
        result.message = "登录异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

// 验证令牌
TokenValidationResult AuthService::validateToken(const std::string& token) {
    TokenValidationResult result;

    try {
        // 1. 验证令牌签名和有效期
        if (!jwtManager_->validateToken(token)) {
            result.message = "令牌无效或已过期";
            Logger::warning(result.message);
            return result;
        }

        // 2. 解析令牌获取用户信息
        Json::Value tokenData = jwtManager_->decodeToken(token);
        if (tokenData.isNull() || !tokenData.isMember("subject") || !tokenData.isMember("username")) {
            result.message = "令牌数据无效";
            Logger::error(result.message);
            return result;
        }

        result.valid = true;
        result.message = "令牌验证成功";
        result.userId = std::stoi(tokenData["subject"].asString());
        result.username = tokenData["username"].asString();
        return result;

    } catch (const std::exception& e) {
        result.message = "令牌验证异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

// 刷新令牌
AuthResult AuthService::refreshTokens(const std::string& refreshToken) {
    AuthResult result;

    try {
        Logger::info("Attempting to refresh tokens");

        // 1. 验证刷新令牌
        if (!jwtManager_->validateToken(refreshToken)) {
            result.message = "刷新令牌无效或已过期";
            Logger::warning(result.message);
            return result;
        }

        // 2. 解析令牌获取用户信息
        Json::Value tokenData = jwtManager_->decodeToken(refreshToken);
        if (tokenData.isNull() || !tokenData.isMember("subject") || !tokenData.isMember("username")) {
            result.message = "刷新令牌数据无效";
            Logger::error(result.message);
            return result;
        }

        int userId = std::stoi(tokenData["subject"].asString());
        std::string username = tokenData["username"].asString();

        // 3. 验证用户是否仍然存在且激活
        auto user = userRepo_->findById(userId);
        if (!user.has_value()) {
            result.message = "用户不存在";
            Logger::warning(result.message + " for token refresh");
            return result;
        }

        if (user->getStatus() != UserStatus::ACTIVE) {
            result.message = "账户未激活或已禁用";
            Logger::warning(result.message + " for token refresh");
            return result;
        }

        // 4. 生成新令牌
        std::string newAccessToken = jwtManager_->generateAccessToken(userId, username);
        std::string newRefreshToken = jwtManager_->generateRefreshToken(userId, username);

        if (newAccessToken.empty() || newRefreshToken.empty()) {
            result.message = "生成新令牌失败";
            Logger::error(result.message);
            return result;
        }

        Logger::info("Tokens refreshed successfully for user: " + username);

        // 5. 清除敏感信息
        user->setPassword("");
        user->setSalt("");

        result.success = true;
        result.message = "令牌刷新成功";
        result.accessToken = newAccessToken;
        result.refreshToken = newRefreshToken;
        result.user = *user;
        return result;

    } catch (const std::exception& e) {
        result.message = "刷新令牌异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

// 修改密码
bool AuthService::changePassword(int userId, const std::string& oldPassword, const std::string& newPassword) {
    try {
        Logger::info("Attempting to change password for user ID: " + std::to_string(userId));

        // 1. 查询用户
        auto user = userRepo_->findById(userId);
        if (!user.has_value()) {
            Logger::warning("User not found for password change");
            return false;
        }

        // 2. 验证旧密码
        if (!PasswordHasher::verifyPassword(oldPassword, user->getSalt(), user->getPassword())) {
            Logger::warning("Old password verification failed");
            return false;
        }

        // 3. 验证新密码格式
        if (!validatePassword(newPassword)) {
            Logger::warning("New password format validation failed");
            return false;
        }

        // 4. 生成新盐值
        std::string newSalt = PasswordHasher::generateSalt();
        if (newSalt.empty()) {
            Logger::error("Failed to generate new salt");
            return false;
        }

        // 5. 哈希新密码
        std::string newPasswordHash = PasswordHasher::hashPassword(newPassword, newSalt);
        if (newPasswordHash.empty()) {
            Logger::error("Failed to hash new password");
            return false;
        }

        // 6. 更新用户密码
        user->setPassword(newPasswordHash);
        user->setSalt(newSalt);

        if (!userRepo_->updateUser(*user)) {
            Logger::error("Failed to update password in database");
            return false;
        }

        Logger::info("Password changed successfully for user ID: " + std::to_string(userId));
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in changePassword: " + std::string(e.what()));
        return false;
    }
}

// 用户登出
bool AuthService::logoutUser(const std::string& accessToken) {
    try {
        // JWT是无状态的，这里只做简单验证
        if (!jwtManager_->validateToken(accessToken)) {
            Logger::warning("Invalid access token for logout");
            return false;
        }

        Logger::info("User logged out successfully");
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in logoutUser: " + std::string(e.what()));
        return false;
    }
}

// 获取用户完整信息
std::optional<User> AuthService::getUserProfile(int userId) {
    try {
        auto user = userRepo_->findById(userId);
        if (!user.has_value()) {
            return std::nullopt;
        }

        // 清除敏感信息
        user->setPassword("");
        user->setSalt("");

        return user;

    } catch (const std::exception& e) {
        Logger::error("Exception in getUserProfile: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 更新用户信息
UpdateProfileResult AuthService::updateUserProfile(
    int userId,
    const std::string& realName,
    const std::string& email,
    const std::string& phone,
    const std::string& avatarUrl,
    const std::string& bio,
    const std::string& gender,
    const std::string& location
) {
    UpdateProfileResult result;

    try {
        Logger::info("Updating user profile for ID: " + std::to_string(userId));

        // 1. 验证用户存在
        auto existingUser = userRepo_->findById(userId);
        if (!existingUser.has_value()) {
            result.message = "用户不存在";
            Logger::warning(result.message);
            return result;
        }

        // 2. 验证邮箱格式（如果提供）
        if (!email.empty() && !validateEmail(email)) {
            result.message = "邮箱格式无效";
            Logger::warning(result.message);
            return result;
        }

        // 3. 验证手机号格式（如果提供）
        if (!phone.empty() && !validatePhone(phone)) {
            result.message = "手机号格式无效";
            Logger::warning(result.message);
            return result;
        }

        // 4. 如果修改了手机号，检查是否已被其他用户使用
        if (!phone.empty() && phone != existingUser->getPhone()) {
            if (userRepo_->phoneExists(phone)) {
                result.message = "手机号已被使用";
                Logger::warning(result.message + ": " + phone);
                return result;
            }
        }

        // 5. 如果修改了邮箱，检查是否已被其他用户使用
        if (!email.empty() && email != existingUser->getEmail()) {
            if (userRepo_->emailExists(email)) {
                result.message = "邮箱已被使用";
                Logger::warning(result.message + ": " + email);
                return result;
            }
        }

        // 6. 准备更新参数（如果参数为空，使用现有值）
        std::string finalRealName = !realName.empty() ? realName : existingUser->getRealName();
        std::string finalEmail = !email.empty() ? email : existingUser->getEmail();
        std::string finalPhone = !phone.empty() ? phone : existingUser->getPhone();
        std::string finalAvatarUrl = !avatarUrl.empty() ? avatarUrl : existingUser->getAvatarUrl();
        std::string finalBio = !bio.empty() ? bio : existingUser->getBio();
        std::string finalGender = !gender.empty() ? gender : existingUser->getGender();
        std::string finalLocation = !location.empty() ? location : existingUser->getLocation();

        // 7. 调用Repository层更新用户信息
        if (!userRepo_->updateUserProfile(userId, finalRealName, finalEmail, finalPhone, 
                                          finalAvatarUrl, finalBio, finalGender, finalLocation)) {
            result.message = "更新用户信息失败";
            Logger::error(result.message);
            return result;
        }

        Logger::info("User profile updated successfully for ID: " + std::to_string(userId));

        // 8. 查询更新后的用户信息
        auto updatedUser = userRepo_->findById(userId);
        if (!updatedUser.has_value()) {
            result.message = "查询更新后的用户信息失败";
            Logger::error(result.message);
            return result;
        }

        // 9. 清除敏感信息
        updatedUser->setPassword("");
        updatedUser->setSalt("");

        result.success = true;
        result.message = "更新成功";
        result.user = *updatedUser;
        return result;

    } catch (const std::exception& e) {
        result.message = "更新用户信息异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

// 检查用户名可用性
UsernameCheckResult AuthService::checkUsernameAvailability(const std::string& username) {
    UsernameCheckResult result;

    try {
        // 1. 验证格式
        if (!validateUsername(username)) {
            result.valid = false;
            result.available = false;
            result.message = "用户名格式无效（3-50字符，字母数字下划线）";
            Logger::warning(result.message);
            return result;
        }

        result.valid = true;

        // 2. 检查是否已存在
        if (userRepo_->usernameExists(username)) {
            result.available = false;
            result.message = "用户名已被使用";
            Logger::info(result.message + ": " + username);
            return result;
        }

        result.available = true;
        result.message = "用户名可用";
        Logger::info(result.message + ": " + username);
        return result;

    } catch (const std::exception& e) {
        result.valid = false;
        result.available = false;
        result.message = "检查用户名异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

// 获取用户公开信息
std::optional<User> AuthService::getUserPublicInfo(const std::string& userId) {
    try {
        auto user = userRepo_->findByUserId(userId);
        if (!user.has_value()) {
            return std::nullopt;
        }

        // 清除所有敏感信息
        user->setPassword("");
        user->setSalt("");
        user->setPhone("");
        user->setEmail("");

        return user;

    } catch (const std::exception& e) {
        Logger::error("Exception in getUserPublicInfo: " + std::string(e.what()));
        return std::nullopt;
    }
}

// 生成唯一用户ID
std::string AuthService::generateUserId() {
    // 格式：USR_2025Q1_XXXXXX
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);

    int year = tm->tm_year + 1900;
    int quarter = (tm->tm_mon / 3) + 1;

    // 生成6位随机字母数字
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);

    std::string randomPart;
    for (int i = 0; i < 6; ++i) {
        randomPart += alphanum[dis(gen)];
    }

    std::ostringstream oss;
    oss << "USR_" << year << "Q" << quarter << "_" << randomPart;

    return oss.str();
}


// 验证用户名格式
bool AuthService::validateUsername(const std::string& username) {
    // 3-50字符，字母数字下划线
    if (username.length() < 3 || username.length() > 50) {
        return false;
    }

    std::regex usernameRegex("^[a-zA-Z0-9_]+$");
    return std::regex_match(username, usernameRegex);
}

// 验证密码格式
bool AuthService::validatePassword(const std::string& password) {
    // 8-128字符
    return password.length() >= 8 && password.length() <= 128;
}

// 验证手机号格式
bool AuthService::validatePhone(const std::string& phone) {
    // 11位中国手机号
    if (phone.length() != 11) {
        return false;
    }

    std::regex phoneRegex("^1[3-9][0-9]{9}$");
    return std::regex_match(phone, phoneRegex);
}

// 验证邮箱格式
bool AuthService::validateEmail(const std::string& email) {
    // 标准邮箱格式
    std::regex emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(email, emailRegex);
}
