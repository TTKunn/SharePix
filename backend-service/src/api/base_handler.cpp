/**
 * @file base_handler.cpp
 * @brief API处理器基类实现
 * @author Knot Team
 * @date 2025-10-08
 */

#include "base_handler.h"
#include "core/auth_service.h"
#include "utils/logger.h"
#include <sstream>
#include <ctime>

// 从请求中提取JWT令牌
std::string BaseHandler::extractToken(const httplib::Request& req) {
    // 从Authorization头中提取令牌
    if (req.has_header("Authorization")) {
        std::string authHeader = req.get_header_value("Authorization");

        // 格式：Bearer <token>
        if (authHeader.substr(0, 7) == "Bearer ") {
            return authHeader.substr(7);
        }
    }

    return "";
}

// 从JWT令牌中获取用户ID
int BaseHandler::getUserIdFromToken(const std::string& token) {
    try {
        auto authService = std::make_unique<AuthService>();
        TokenValidationResult validation = authService->validateToken(token);

        if (validation.valid) {
            return validation.userId;
        }

        return 0;

    } catch (const std::exception& e) {
        Logger::error("Exception in getUserIdFromToken: " + std::string(e.what()));
        return 0;
    }
}

// 验证JWT令牌并获取用户ID（组合方法）
bool BaseHandler::authenticateRequest(const httplib::Request& req, int& userId) {
    std::string token = extractToken(req);
    if (token.empty()) {
        return false;
    }
    
    userId = getUserIdFromToken(token);
    return userId != 0;
}

// 解析JSON请求体
bool BaseHandler::parseJsonBody(const std::string& body, Json::Value& jsonOut) {
    Json::CharReaderBuilder reader;
    std::istringstream stream(body);
    std::string errors;

    bool success = Json::parseFromStream(reader, stream, &jsonOut, &errors);
    if (!success) {
        Logger::warning("Failed to parse JSON: " + errors);
    }

    return success;
}

// 发送JSON响应
void BaseHandler::sendJsonResponse(httplib::Response& res,
                                   int statusCode,
                                   bool success,
                                   const std::string& message,
                                   const Json::Value& data) {
    Json::Value response;
    response["success"] = success;
    response["message"] = message;
    response["data"] = data;
    response["timestamp"] = static_cast<Json::Int64>(std::time(nullptr));

    Json::StreamWriterBuilder writer;
    std::string jsonStr = Json::writeString(writer, response);

    res.set_content(jsonStr, "application/json");
    res.status = statusCode;
}

// 发送错误响应（快捷方法）
void BaseHandler::sendErrorResponse(httplib::Response& res,
                                   int statusCode,
                                   const std::string& message) {
    sendJsonResponse(res, statusCode, false, message);
}

// 发送成功响应（快捷方法）
void BaseHandler::sendSuccessResponse(httplib::Response& res,
                                     const std::string& message,
                                     const Json::Value& data) {
    sendJsonResponse(res, 200, true, message, data);
}

// 验证分页参数
bool BaseHandler::validatePagination(int page, int pageSize, int maxPageSize) {
    if (page < 1) {
        Logger::warning("Invalid page number: " + std::to_string(page));
        return false;
    }
    
    if (pageSize < 1 || pageSize > maxPageSize) {
        Logger::warning("Invalid page size: " + std::to_string(pageSize));
        return false;
    }
    
    return true;
}

// 从查询参数中获取整数值
int BaseHandler::getQueryParamInt(const httplib::Request& req, 
                                 const std::string& key, 
                                 int defaultValue) {
    if (req.has_param(key.c_str())) {
        try {
            return std::stoi(req.get_param_value(key.c_str()));
        } catch (const std::exception& e) {
            Logger::warning("Failed to parse query param '" + key + "': " + e.what());
            return defaultValue;
        }
    }
    return defaultValue;
}

// 从查询参数中获取字符串值
std::string BaseHandler::getQueryParamString(const httplib::Request& req,
                                            const std::string& key,
                                            const std::string& defaultValue) {
    if (req.has_param(key.c_str())) {
        return req.get_param_value(key.c_str());
    }
    return defaultValue;
}

