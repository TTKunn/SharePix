/**
 * @file image_handler.cpp
 * @brief 图片API处理器实现
 * @author Knot Team
 * @date 2025-10-07
 */

#include "api/image_handler.h"
#include "core/image_service.h"
#include "core/auth_service.h"
#include "security/jwt_manager.h"
#include "utils/logger.h"
#include <sstream>
#include <fstream>

// 构造函数
ImageHandler::ImageHandler() {
    imageService_ = std::make_unique<ImageService>();
    Logger::info("ImageHandler initialized");
}

// 析构函数
ImageHandler::~ImageHandler() {
    Logger::info("ImageHandler destroyed");
}

// 注册路由
void ImageHandler::registerRoutes(httplib::Server& server) {
    // POST /api/v1/images - 上传图片
    server.Post("/api/v1/images", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpload(req, res);
    });
    
    // GET /api/v1/images - 获取最新图片列表
    server.Get("/api/v1/images", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetRecent(req, res);
    });
    
    // GET /api/v1/images/:id - 获取图片详情
    server.Get("/api/v1/images/:id", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetById(req, res);
    });
    
    // PUT /api/v1/images/:id - 更新图文配文
    server.Put("/api/v1/images/:id", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpdate(req, res);
    });
    
    // DELETE /api/v1/images/:id - 删除图片
    server.Delete("/api/v1/images/:id", [this](const httplib::Request& req, httplib::Response& res) {
        handleDelete(req, res);
    });
    
    // GET /api/v1/users/:id/images - 获取用户图片列表
    server.Get("/api/v1/users/:id/images", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetUserImages(req, res);
    });
    
    Logger::info("Image routes registered");
}

// 处理图片上传请求
void ImageHandler::handleUpload(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌
        std::string token = extractToken(req);
        if (token.empty()) {
            sendJsonResponse(res, 401, false, "未提供认证令牌");
            return;
        }
        
        int userId = getUserIdFromToken(token);
        if (userId == 0) {
            sendJsonResponse(res, 401, false, "无效的认证令牌");
            return;
        }
        
        // 2. 获取上传的文件
        if (req.form.files.find("image") == req.form.files.end()) {
            sendJsonResponse(res, 400, false, "未找到上传的图片文件");
            return;
        }

        auto file = req.form.files.find("image")->second;

        // 3. 保存临时文件
        std::string tempPath = "/tmp/upload_" + std::to_string(std::time(nullptr)) + "_" + file.filename;
        std::ofstream ofs(tempPath, std::ios::binary);
        ofs << file.content;
        ofs.close();

        // 4. 获取标题和配文
        std::string title = req.form.get_field("title");
        std::string description = req.form.get_field("description");
        
        // 5. 获取标签（逗号分隔）
        std::vector<std::string> tags;
        std::string tagsStr = req.form.get_field("tags");
        if (!tagsStr.empty()) {
            std::istringstream iss(tagsStr);
            std::string tag;
            while (std::getline(iss, tag, ',')) {
                // 去除首尾空格
                tag.erase(0, tag.find_first_not_of(" \t"));
                tag.erase(tag.find_last_not_of(" \t") + 1);
                if (!tag.empty()) {
                    tags.push_back(tag);
                }
            }
        }
        
        // 6. 调用Service上传图片
        ImageUploadResult result = imageService_->uploadImage(userId, tempPath, title, description, tags);
        
        if (result.success) {
            Json::Value data = result.image.toJson();
            sendJsonResponse(res, 201, true, result.message, data);
        } else {
            sendJsonResponse(res, 400, false, result.message);
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleUpload: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理获取最新图片列表请求
void ImageHandler::handleGetRecent(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取分页参数
        int page = req.has_param("page") ? std::stoi(req.get_param_value("page")) : 1;
        int pageSize = req.has_param("page_size") ? std::stoi(req.get_param_value("page_size")) : 20;
        
        // 调用Service查询
        ImageQueryResult result = imageService_->getRecentImages(page, pageSize);
        
        if (result.success) {
            Json::Value data;
            data["total"] = result.total;
            data["page"] = result.page;
            data["page_size"] = result.pageSize;
            
            Json::Value imagesArray(Json::arrayValue);
            for (const auto& image : result.images) {
                imagesArray.append(image.toJson());
            }
            data["images"] = imagesArray;
            
            sendJsonResponse(res, 200, true, result.message, data);
        } else {
            sendJsonResponse(res, 400, false, result.message);
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetRecent: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理获取图片详情请求
void ImageHandler::handleGetById(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取图片ID
        std::string imageId = req.path_params.at("id");
        
        // 调用Service查询
        auto image = imageService_->getImageDetail(imageId);
        
        if (image.has_value()) {
            Json::Value data = image->toJson();
            sendJsonResponse(res, 200, true, "查询成功", data);
        } else {
            sendJsonResponse(res, 404, false, "图片不存在");
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetById: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理更新图文配文请求
void ImageHandler::handleUpdate(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌
        std::string token = extractToken(req);
        if (token.empty()) {
            sendJsonResponse(res, 401, false, "未提供认证令牌");
            return;
        }
        
        int userId = getUserIdFromToken(token);
        if (userId == 0) {
            sendJsonResponse(res, 401, false, "无效的认证令牌");
            return;
        }
        
        // 2. 获取图片ID
        std::string imageId = req.path_params.at("id");
        
        // 3. 解析JSON请求体
        Json::Value jsonBody;
        if (!parseJsonBody(req.body, jsonBody)) {
            sendJsonResponse(res, 400, false, "无效的JSON格式");
            return;
        }
        
        // 4. 获取标题和配文
        if (!jsonBody.isMember("title") || !jsonBody["title"].isString()) {
            sendJsonResponse(res, 400, false, "缺少标题字段");
            return;
        }
        
        std::string title = jsonBody["title"].asString();
        std::string description = jsonBody.isMember("description") ? jsonBody["description"].asString() : "";
        
        // 5. 调用Service更新
        bool success = imageService_->updateImageText(imageId, userId, title, description);
        
        if (success) {
            sendJsonResponse(res, 200, true, "更新成功");
        } else {
            sendJsonResponse(res, 403, false, "无权限或图片不存在");
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleUpdate: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理删除图片请求
void ImageHandler::handleDelete(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌
        std::string token = extractToken(req);
        if (token.empty()) {
            sendJsonResponse(res, 401, false, "未提供认证令牌");
            return;
        }
        
        int userId = getUserIdFromToken(token);
        if (userId == 0) {
            sendJsonResponse(res, 401, false, "无效的认证令牌");
            return;
        }
        
        // 2. 获取图片ID
        std::string imageId = req.path_params.at("id");
        
        // 3. 调用Service删除
        bool success = imageService_->deleteImage(imageId, userId);
        
        if (success) {
            sendJsonResponse(res, 200, true, "删除成功");
        } else {
            sendJsonResponse(res, 403, false, "无权限或图片不存在");
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleDelete: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 处理获取用户图片列表请求
void ImageHandler::handleGetUserImages(const httplib::Request& req, httplib::Response& res) {
    try {
        // 获取用户ID
        int userId = std::stoi(req.path_params.at("id"));
        
        // 获取分页参数
        int page = req.has_param("page") ? std::stoi(req.get_param_value("page")) : 1;
        int pageSize = req.has_param("page_size") ? std::stoi(req.get_param_value("page_size")) : 20;
        
        // 调用Service查询
        ImageQueryResult result = imageService_->getUserImages(userId, page, pageSize);
        
        if (result.success) {
            Json::Value data;
            data["total"] = result.total;
            data["page"] = result.page;
            data["page_size"] = result.pageSize;
            
            Json::Value imagesArray(Json::arrayValue);
            for (const auto& image : result.images) {
                imagesArray.append(image.toJson());
            }
            data["images"] = imagesArray;
            
            sendJsonResponse(res, 200, true, result.message, data);
        } else {
            sendJsonResponse(res, 400, false, result.message);
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetUserImages: " + std::string(e.what()));
        sendJsonResponse(res, 500, false, "服务器内部错误");
    }
}

// 从JWT令牌中获取用户ID
int ImageHandler::getUserIdFromToken(const std::string& token) {
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

// 从请求中提取JWT令牌
std::string ImageHandler::extractToken(const httplib::Request& req) {
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

// 解析JSON请求体
bool ImageHandler::parseJsonBody(const std::string& body, Json::Value& jsonOut) {
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
void ImageHandler::sendJsonResponse(httplib::Response& res,
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

