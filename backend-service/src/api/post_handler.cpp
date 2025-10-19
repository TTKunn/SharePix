/**
 * @file post_handler.cpp
 * @brief 帖子API处理器实现
 * @author Knot Team
 * @date 2025-10-08
 */

#include "api/post_handler.h"
#include "utils/logger.h"
#include "utils/url_helper.h"
#include "utils/base64_decoder.h"
#include "database/user_repository.h"
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <cstdlib>
#include <algorithm>
#include <cctype>

// 构造函数
PostHandler::PostHandler() {
    postService_ = std::make_unique<PostService>();
    userService_ = std::make_unique<UserService>();
    likeService_ = std::make_unique<LikeService>();
    favoriteService_ = std::make_unique<FavoriteService>();
    Logger::info("PostHandler initialized with all services");
}

// 注册所有路由
void PostHandler::registerRoutes(httplib::Server& server) {
    // 创建帖子
    server.Post("/api/v1/posts", [this](const httplib::Request& req, httplib::Response& res) {
        handleCreatePost(req, res);
    });
    
    // 获取帖子详情
    server.Get("/api/v1/posts/:post_id", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetPostDetail(req, res);
    });
    
    // 更新帖子
    server.Put("/api/v1/posts/:post_id", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpdatePost(req, res);
    });

    // 删除帖子
    server.Delete("/api/v1/posts/:post_id", [this](const httplib::Request& req, httplib::Response& res) {
        handleDeletePost(req, res);
    });

    // 获取Feed流
    server.Get("/api/v1/posts", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetRecentPosts(req, res);
    });

    // 获取用户帖子列表 (支持逻辑ID和物理ID)
    server.Get("/api/v1/users/:user_id/posts", [this](const httplib::Request& req, httplib::Response& res) {
        handleGetUserPosts(req, res);
    });

    // 向帖子添加图片
    server.Post("/api/v1/posts/:post_id/images", [this](const httplib::Request& req, httplib::Response& res) {
        handleAddImageToPost(req, res);
    });

    // 删除帖子中的图片
    server.Delete("/api/v1/posts/:post_id/images/:image_id", [this](const httplib::Request& req, httplib::Response& res) {
        handleRemoveImageFromPost(req, res);
    });

    // 调整图片顺序
    server.Put("/api/v1/posts/:post_id/images/order", [this](const httplib::Request& req, httplib::Response& res) {
        handleReorderImages(req, res);
    });
    
    Logger::info("PostHandler routes registered");
}

// 将Image对象转换为JSON
Json::Value PostHandler::imageToJson(const Image& image) {
    Json::Value json;
    json["image_id"] = image.getImageId();
    json["display_order"] = image.getDisplayOrder();
    // 统一为返回的路径添加服务器URL前缀
    json["file_url"] = UrlHelper::toFullUrl(image.getFileUrl());
    json["thumbnail_url"] = UrlHelper::toFullUrl(image.getThumbnailUrl());
    json["file_size"] = static_cast<Json::Int64>(image.getFileSize());
    json["width"] = image.getWidth();
    json["height"] = image.getHeight();
    json["mime_type"] = image.getMimeType();
    json["create_time"] = static_cast<Json::Int64>(image.getCreateTime());
    return json;
}

// 将Post对象转换为JSON
Json::Value PostHandler::postToJson(const Post& post, bool includeImages) {
    // 直接使用Post模型的toJson方法，它内部会调用Image::toJson()添加URL前缀
    return post.toJson(includeImages);
}

// POST /api/v1/posts - 创建帖子
void PostHandler::handleCreatePost(const httplib::Request& req, httplib::Response& res) {
    try {
        Logger::info("=== [CREATE POST] Request received ===");
        
        // 记录请求基本信息
        std::string contentType = req.get_header_value("Content-Type");
        Logger::info("[CREATE POST] Request Info - Content-Type: " + contentType + 
                    ", Body Size: " + std::to_string(req.body.size()) + " bytes" +
                    ", Remote Addr: " + req.remote_addr);
        
        // 1. 验证JWT令牌
        std::string token = extractToken(req);
        if (token.empty()) {
            Logger::warning("[CREATE POST] ✗ Authentication failed - No token provided");
            Logger::warning("[CREATE POST]   → Authorization header: " + 
                          std::string(req.has_header("Authorization") ? "present but empty" : "missing"));
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }

        int userId = getUserIdFromToken(token);
        if (userId == 0) {
            Logger::warning("[CREATE POST] ✗ Authentication failed - Invalid token");
            Logger::warning("[CREATE POST]   → Token (first 50 chars): " + token.substr(0, std::min(50, (int)token.size())) + "...");
            sendErrorResponse(res, 401, "无效的认证令牌");
            return;
        }
        
        Logger::info("[CREATE POST] ✓ User authenticated - UserID: " + std::to_string(userId));

        // 2. 检查请求格式（支持multipart和JSON两种格式）
        bool isMultipart = req.is_multipart_form_data();
        bool isJson = (contentType.find("application/json") != std::string::npos);

        if (!isMultipart && !isJson) {
            Logger::error("[CREATE POST] ✗ Unsupported Content-Type");
            Logger::error("[CREATE POST]   → Received: " + contentType);
            Logger::error("[CREATE POST]   → Expected: multipart/form-data or application/json");
            Logger::error("[CREATE POST]   → Suggestion: Check your client's Content-Type header");
            sendErrorResponse(res, 400, "请求必须使用multipart/form-data或application/json格式");
            return;
        }

        Logger::info("[CREATE POST] ✓ Request format validated: " + 
                    std::string(isMultipart ? "multipart/form-data" : "application/json"));

        // 3. 解析请求参数
        std::string title;
        std::string description;
        std::vector<std::string> tags;
        std::vector<std::string> savedImagePaths;

        if (isMultipart) {
            // === multipart/form-data 格式处理 ===

        // 获取title（必填）
        if (req.form.has_field("title")) {
            title = req.form.get_field("title");
        }
        if (title.empty()) {
                Logger::error("[CREATE POST] ✗ Validation failed - Title is empty");
                Logger::error("[CREATE POST]   → Field 'title' is required but not provided or empty");
            sendErrorResponse(res, 400, "标题不能为空");
            return;
        }

        // 获取description（可选）
        if (req.form.has_field("description")) {
            description = req.form.get_field("description");
        }

        // 获取tags（可选，支持两种格式）
        if (req.form.has_field("tags")) {
            // 先尝试获取多个tags字段（原有格式）
            auto tagFields = req.form.get_fields("tags");
            
            if (tagFields.size() == 1) {
                // 只有一个tags字段，检查是否为JSON数组字符串（前端HarmonyOS格式）
                const std::string& tagsValue = tagFields[0];
                
                // 检查是否为JSON数组格式（以 "[" 开头）
                if (!tagsValue.empty() && tagsValue[0] == '[') {
                    try {
                        // 解析JSON数组
                        Json::Value tagsJson;
                        Json::CharReaderBuilder builder;
                        std::string errs;
                        std::istringstream stream(tagsValue);
                        
                        if (Json::parseFromStream(builder, stream, &tagsJson, &errs)) {
                            if (tagsJson.isArray()) {
                                // 成功解析为数组，提取每个标签
                                for (const auto& tag : tagsJson) {
                                    if (tag.isString()) {
                                        tags.push_back(tag.asString());
                                    }
                                }
                                Logger::info("[CREATE POST] ✓ Parsed tags from JSON array: " + 
                                           std::to_string(tags.size()) + " tags");
                            } else {
                                // 不是数组，当作普通字符串处理
                                tags.push_back(tagsValue);
                            }
                        } else {
                            // JSON解析失败，当作普通字符串处理
                            Logger::warning("[CREATE POST] ⚠ Failed to parse tags as JSON, treating as single tag");
                            tags.push_back(tagsValue);
                        }
                    } catch (const std::exception& e) {
                        // 异常处理，当作普通字符串
                        Logger::warning("[CREATE POST] ⚠ Exception parsing tags JSON: " + std::string(e.what()));
                        tags.push_back(tagsValue);
                    }
                } else {
                    // 不是JSON数组格式，当作普通字符串
                    tags.push_back(tagsValue);
                }
            } else {
                // 多个tags字段，使用原有逻辑
                tags = tagFields;
            }
        }
        
            Logger::info("[CREATE POST] ✓ Form data parsed - Title: '" + title + 
                        "' (length: " + std::to_string(title.size()) + ")" +
                        ", Description: " + (description.empty() ? "none" : "'" + description.substr(0, std::min(50, (int)description.size())) + "...'") +
                        ", Tags: " + std::to_string(tags.size()));
            
            if (!tags.empty()) {
                std::string tagList = "";
                for (size_t i = 0; i < tags.size() && i < 5; i++) {
                    if (i > 0) tagList += ", ";
                    tagList += "'" + tags[i] + "'";
                }
                Logger::debug("[CREATE POST]   → Tags: " + tagList);
            }

        // 获取所有上传的图片文件
        if (req.form.has_file("imageFiles")) {
            auto imageFiles = req.form.get_files("imageFiles");
            
                Logger::info("[CREATE POST] ✓ Received " + std::to_string(imageFiles.size()) + " image file(s) in multipart request");

            for (size_t i = 0; i < imageFiles.size(); i++) {
                const auto& fileData = imageFiles[i];
                
                Logger::info("[CREATE POST] Processing image " + std::to_string(i + 1) + "/" + 
                           std::to_string(imageFiles.size()) + " - Filename: " + fileData.filename + 
                           ", ContentType: " + fileData.content_type + 
                           ", Size: " + std::to_string(fileData.content.size()) + " bytes");
                
                // 验证文件类型
                if (fileData.content_type.find("image/") != 0) {
                        Logger::error("[CREATE POST] ✗ Invalid file type for image " + std::to_string(i + 1));
                        Logger::error("[CREATE POST]   → Received: " + fileData.content_type);
                        Logger::error("[CREATE POST]   → Expected: image/* (e.g., image/jpeg, image/png)");
                    sendErrorResponse(res, 400, "只能上传图片文件");
                    return;
                }

                // 验证文件大小（最大5MB）
                if (fileData.content.size() > 5 * 1024 * 1024) {
                        Logger::error("[CREATE POST] ✗ File size exceeds limit for image " + std::to_string(i + 1));
                        Logger::error("[CREATE POST]   → Size: " + std::to_string(fileData.content.size()) + " bytes (" +
                                    std::to_string(fileData.content.size() / 1024 / 1024) + " MB)");
                        Logger::error("[CREATE POST]   → Max allowed: 5 MB (5242880 bytes)");
                    sendErrorResponse(res, 400, "图片文件大小不能超过5MB");
                    return;
                }

                // 保存文件到临时目录
                std::string savedPath = saveUploadedFile(
                    fileData.content,
                    fileData.filename,
                    fileData.content_type
                );

                if (savedPath.empty()) {
                        Logger::error("[CREATE POST] ✗ Failed to save image " + std::to_string(i + 1));
                        Logger::error("[CREATE POST]   → Filename: " + fileData.filename);
                        Logger::error("[CREATE POST]   → Possible causes: disk space, permissions, invalid image data");
                    sendErrorResponse(res, 500, "保存图片文件失败");
                    return;
                }

                savedImagePaths.push_back(savedPath);
                    Logger::info("[CREATE POST] ✓ Image " + std::to_string(i + 1) + " saved successfully - Path: " + savedPath);
            }
        } else {
                Logger::warning("[CREATE POST] ⚠ No image files found in multipart request");
                Logger::warning("[CREATE POST]   → Field name should be 'imageFiles'");
                Logger::warning("[CREATE POST]   → Current form files count: " + std::to_string(req.form.files.size()));
            }

        } else if (isJson) {
            // === application/json 格式处理（支持Base64图片） ===
            
            Logger::info("[CREATE POST] Parsing JSON request body...");
            
            // 🔧 关键修复: 检查JSON body大小，防止内存耗尽
            const size_t MAX_JSON_SIZE = 50 * 1024 * 1024;  // 50MB限制（允许多张大图）
            if (req.body.size() > MAX_JSON_SIZE) {
                Logger::error("[CREATE POST] ✗ JSON body too large");
                Logger::error("[CREATE POST]   → Body size: " + std::to_string(req.body.size()) + " bytes (" +
                            std::to_string(req.body.size() / 1024 / 1024) + " MB)");
                Logger::error("[CREATE POST]   → Max allowed: 50 MB");
                sendErrorResponse(res, 413, "请求体过大（超过50MB）");
                return;
            }
            
            // 3.1 解析JSON请求体
            Json::Value requestBody;
            try {
                if (!parseJsonBody(req.body, requestBody)) {
                    Logger::error("[CREATE POST] ✗ Failed to parse JSON body");
                    Logger::error("[CREATE POST]   → Body size: " + std::to_string(req.body.size()) + " bytes");
                    // 记录请求体的前200个字符用于调试
                    std::string bodyPreview = req.body.substr(0, std::min(200, (int)req.body.size()));
                    Logger::error("[CREATE POST]   → Body preview: " + bodyPreview + "...");
                    Logger::error("[CREATE POST]   → Possible causes: invalid JSON syntax, encoding issues");
                    sendErrorResponse(res, 400, "无效的JSON格式");
                    return;
                }
            } catch (const std::exception& e) {
                // 🔧 关键修复: 捕获JSON解析异常（可能是OOM）
                Logger::error("[CREATE POST] ✗ Exception during JSON parsing");
                Logger::error("[CREATE POST]   → Exception: " + std::string(e.what()));
                Logger::error("[CREATE POST]   → Body size: " + std::to_string(req.body.size()) + " bytes");
                sendErrorResponse(res, 500, "JSON解析失败: " + std::string(e.what()));
                return;
            }
            
            Logger::info("[CREATE POST] ✓ JSON parsed successfully");
            
            // 3.2 提取文本字段
            title = requestBody.get("title", "").asString();
            description = requestBody.get("description", "").asString();
            
            if (title.empty()) {
                Logger::error("[CREATE POST] ✗ Validation failed - Title is empty in JSON");
                Logger::error("[CREATE POST]   → JSON has 'title' field: " + 
                            std::string(requestBody.isMember("title") ? "yes" : "no"));
                if (requestBody.isMember("title")) {
                    Logger::error("[CREATE POST]   → Title value type: " + 
                                std::to_string(requestBody["title"].type()));
                }
                sendErrorResponse(res, 400, "标题不能为空");
                return;
            }
            
            // 3.3 提取标签
            if (requestBody.isMember("tags") && requestBody["tags"].isArray()) {
                for (const auto& tag : requestBody["tags"]) {
                    tags.push_back(tag.asString());
                }
            }
            
            Logger::info("[CREATE POST] ✓ JSON data parsed - Title: '" + title + 
                        "' (length: " + std::to_string(title.size()) + ")" +
                        ", Description: " + (description.empty() ? "none" : "'" + description.substr(0, std::min(50, (int)description.size())) + "...'") +
                        ", Tags: " + std::to_string(tags.size()));
            
            if (!tags.empty()) {
                std::string tagList = "";
                for (size_t i = 0; i < tags.size() && i < 5; i++) {
                    if (i > 0) tagList += ", ";
                    tagList += "'" + tags[i] + "'";
                }
                Logger::debug("[CREATE POST]   → Tags: " + tagList);
            }
            
            // 3.4 处理图片数据（Base64编码）
            if (!requestBody.isMember("images") || !requestBody["images"].isArray()) {
                Logger::error("[CREATE POST] ✗ Missing or invalid 'images' field in JSON");
                Logger::error("[CREATE POST]   → Has 'images' field: " + 
                            std::string(requestBody.isMember("images") ? "yes" : "no"));
                if (requestBody.isMember("images")) {
                    Logger::error("[CREATE POST]   → 'images' is array: " + 
                                std::string(requestBody["images"].isArray() ? "yes" : "no"));
                    Logger::error("[CREATE POST]   → 'images' type: " + 
                                std::to_string(requestBody["images"].type()));
                }
                Logger::error("[CREATE POST]   → Expected: array of image objects");
                sendErrorResponse(res, 400, "缺少images字段");
                return;
            }
            
            const Json::Value& imagesArray = requestBody["images"];
            
            if (imagesArray.empty() || imagesArray.size() > 9) {
                Logger::error("[CREATE POST] ✗ Invalid image count in JSON");
                Logger::error("[CREATE POST]   → Received: " + std::to_string(imagesArray.size()) + " images");
                Logger::error("[CREATE POST]   → Required: 1-9 images");
                sendErrorResponse(res, 400, "图片数量必须在1-9张之间");
                return;
            }
            
            Logger::info("[CREATE POST] ✓ Processing " + std::to_string(imagesArray.size()) + 
                        " images from JSON request");
            
            // 3.5 逐个处理Base64图片
            for (size_t i = 0; i < imagesArray.size(); i++) {
                const Json::Value& imageData = imagesArray[static_cast<Json::ArrayIndex>(i)];
                
                // 提取字段
                std::string filename = imageData.get("filename", "image.jpg").asString();
                std::string imageContentType = imageData.get("content_type", "image/jpeg").asString();
                std::string base64Data = imageData.get("data", "").asString();
                
                // 🔧 关键修复: 检查Base64数据大小，防止内存爆炸
                // Base64编码后约为原始大小的1.33倍，5MB图片编码后约6.65MB
                const size_t MAX_BASE64_SIZE = 7 * 1024 * 1024;  // 7MB限制
                if (base64Data.size() > MAX_BASE64_SIZE) {
                    // 清理已保存的临时文件
                    for (const auto& path : savedImagePaths) {
                        std::remove(path.c_str());
                    }
                    Logger::error("[CREATE POST] ✗ Base64 data too large for image " + std::to_string(i+1));
                    Logger::error("[CREATE POST]   → Base64 size: " + std::to_string(base64Data.size()) + " bytes (" +
                                std::to_string(base64Data.size() / 1024 / 1024) + " MB)");
                    Logger::error("[CREATE POST]   → Max allowed: 7 MB (after Base64 encoding)");
                    Logger::error("[CREATE POST]   → Tip: Original image should be < 5MB");
                    sendErrorResponse(res, 400, "图片" + std::to_string(i+1) + "的Base64数据过大（超过7MB）");
                    return;
                }
                
                if (base64Data.empty()) {
                    // 清理已保存的临时文件
                    for (const auto& path : savedImagePaths) {
                        std::remove(path.c_str());
                    }
                    Logger::error("[CREATE POST] ✗ Image " + std::to_string(i+1) + " data field is empty");
                    Logger::error("[CREATE POST]   → Image has 'data' field: " + 
                                std::string(imageData.isMember("data") ? "yes" : "no"));
                    Logger::error("[CREATE POST]   → Required: Base64 encoded image data");
                    sendErrorResponse(res, 400, "图片" + std::to_string(i+1) + "的data字段为空");
                    return;
                }
                
                Logger::info("[CREATE POST] Processing image " + std::to_string(i + 1) + "/" + 
                           std::to_string(imagesArray.size()) + " - Filename: " + filename + 
                           ", ContentType: " + imageContentType + 
                           ", Base64Size: " + std::to_string(base64Data.size()) + " bytes");
                
                // 验证Content-Type
                if (imageContentType.find("image/") != 0) {
                    // 清理已保存的临时文件
                    for (const auto& path : savedImagePaths) {
                        std::remove(path.c_str());
                    }
                    Logger::error("[CREATE POST] ✗ Invalid content type for image " + std::to_string(i+1));
                    Logger::error("[CREATE POST]   → Received: " + imageContentType);
                    Logger::error("[CREATE POST]   → Expected: image/* (e.g., image/jpeg, image/png)");
                    sendErrorResponse(res, 400, "只能上传图片文件");
                    return;
                }
                
                // 保存文件（saveUploadedFile会自动检测并解码Base64）
                std::string savedPath;
                try {
                    savedPath = saveUploadedFile(base64Data, filename, imageContentType);
                } catch (const std::exception& e) {
                    // 🔧 关键修复: 捕获Base64解码或文件保存异常
                    // 清理已保存的临时文件
                    for (const auto& path : savedImagePaths) {
                        std::remove(path.c_str());
                    }
                    Logger::error("[CREATE POST] ✗ Exception while saving image " + std::to_string(i+1));
                    Logger::error("[CREATE POST]   → Exception: " + std::string(e.what()));
                    Logger::error("[CREATE POST]   → Filename: " + filename);
                    Logger::error("[CREATE POST]   → Base64 size: " + std::to_string(base64Data.size()) + " bytes");
                    sendErrorResponse(res, 500, "保存图片文件时发生错误: " + std::string(e.what()));
                    return;
                }
                
                if (savedPath.empty()) {
                    // 清理已保存的临时文件
                    for (const auto& path : savedImagePaths) {
                        std::remove(path.c_str());
                    }
                    Logger::error("[CREATE POST] ✗ Failed to save image " + std::to_string(i+1) + " from Base64");
                    Logger::error("[CREATE POST]   → Filename: " + filename);
                    Logger::error("[CREATE POST]   → Base64 size: " + std::to_string(base64Data.size()) + " bytes");
                    Logger::error("[CREATE POST]   → Possible causes: invalid Base64, unsupported format, disk space, permissions");
                    sendErrorResponse(res, 500, "保存图片文件失败");
                    return;
                }
                
                savedImagePaths.push_back(savedPath);
                Logger::info("[CREATE POST] ✓ Image " + std::to_string(i + 1) + " saved successfully - Path: " + savedPath);
            }
        }

        // 4. 验证图片数量（统一验证）
        if (savedImagePaths.empty() || savedImagePaths.size() > 9) {
            // 清理已保存的临时文件
            for (const auto& path : savedImagePaths) {
                std::remove(path.c_str());
            }
            Logger::error("[CREATE POST] ✗ Final validation failed - Invalid image count");
            Logger::error("[CREATE POST]   → Saved images: " + std::to_string(savedImagePaths.size()));
            Logger::error("[CREATE POST]   → Required: 1-9 images");
            if (savedImagePaths.empty()) {
                Logger::error("[CREATE POST]   → No images were successfully saved");
            }
            sendErrorResponse(res, 400, "图片数量必须在1-9张之间");
            return;
        }
        
        Logger::info("[CREATE POST] ✓ All images validated successfully, total: " + 
                    std::to_string(savedImagePaths.size()));

        // 5. 调用Service创建帖子
        Logger::info("[CREATE POST] Creating post in database...");
        Logger::info("[CREATE POST]   → Title: '" + title + "'");
        Logger::info("[CREATE POST]   → User: " + std::to_string(userId));
        Logger::info("[CREATE POST]   → Images: " + std::to_string(savedImagePaths.size()));
        Logger::info("[CREATE POST]   → Tags: " + std::to_string(tags.size()));
        
        PostCreateResult result = postService_->createPost(
            userId,
            title,
            description,
            savedImagePaths,
            tags
        );

        // 6. 清理临时文件
        for (const auto& path : savedImagePaths) {
            std::remove(path.c_str());
        }
        Logger::debug("[CREATE POST] Temporary files cleaned up");

        // 7. 返回结果
        if (result.success) {
            Logger::info("[CREATE POST] ✓✓✓ POST CREATED SUCCESSFULLY ✓✓✓");
            Logger::info("[CREATE POST]   → PostID: " + result.post.getPostId());
            Logger::info("[CREATE POST]   → UserID: " + std::to_string(userId));
            Logger::info("[CREATE POST]   → Title: '" + title + "'");
            Logger::info("[CREATE POST]   → Images: " + std::to_string(savedImagePaths.size()));
            Logger::info("[CREATE POST]   → Tags: " + std::to_string(tags.size()));
            Logger::info("[CREATE POST]   → Status: " + Post::statusToString(result.post.getStatus()));
            
            Json::Value data;
            data["post"] = postToJson(result.post, true);
            sendSuccessResponse(res, "帖子创建成功", data);
        } else {
            Logger::error("[CREATE POST] ✗✗✗ POST CREATION FAILED ✗✗✗");
            Logger::error("[CREATE POST]   → Reason: " + result.message);
            Logger::error("[CREATE POST]   → UserID: " + std::to_string(userId));
            Logger::error("[CREATE POST]   → Title: '" + title + "'");
            sendErrorResponse(res, 400, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("[CREATE POST] ✗✗✗ EXCEPTION OCCURRED ✗✗✗");
        Logger::error("[CREATE POST]   → Exception: " + std::string(e.what()));
        Logger::error("[CREATE POST]   → Type: " + std::string(typeid(e).name()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// GET /api/v1/posts/:post_id - 获取帖子详情
void PostHandler::handleGetPostDetail(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 获取post_id
        std::string postId = req.path_params.at("post_id");
        
        // 2. 调用Service查询帖子
        auto postOpt = postService_->getPostDetail(postId);
        
        // 3. 返回结果
        if (postOpt.has_value()) {
            Json::Value data;
            data["post"] = postToJson(postOpt.value(), true);
            sendSuccessResponse(res, "查询成功", data);
        } else {
            sendErrorResponse(res, 404, "帖子不存在");
        }
        
    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetPostDetail: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// PUT /api/v1/posts/:post_id - 更新帖子
void PostHandler::handleUpdatePost(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌
        std::string token = extractToken(req);
        if (token.empty()) {
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }

        int userId = getUserIdFromToken(token);
        if (userId == 0) {
            sendErrorResponse(res, 401, "无效的认证令牌");
            return;
        }

        // 2. 获取post_id
        std::string postId = req.path_params.at("post_id");

        // 3. 解析请求体
        Json::Value requestBody;
        if (!parseJsonBody(req.body, requestBody)) {
            sendErrorResponse(res, 400, "无效的JSON格式");
            return;
        }

        // 4. 获取参数
        std::string title = requestBody.get("title", "").asString();
        std::string description = requestBody.get("description", "").asString();

        // 5. 调用Service更新帖子
        bool success = postService_->updatePost(postId, userId, title, description);

        // 6. 返回结果
        if (success) {
            sendSuccessResponse(res, "帖子更新成功");
        } else {
            sendErrorResponse(res, 400, "更新失败");
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleUpdatePost: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// DELETE /api/v1/posts/:post_id - 删除帖子
void PostHandler::handleDeletePost(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌
        std::string token = extractToken(req);
        if (token.empty()) {
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }

        int userId = getUserIdFromToken(token);
        if (userId == 0) {
            sendErrorResponse(res, 401, "无效的认证令牌");
            return;
        }

        // 2. 获取post_id
        std::string postId = req.path_params.at("post_id");

        // 3. 调用Service删除帖子
        bool success = postService_->deletePost(postId, userId);

        // 4. 返回结果
        if (success) {
            sendSuccessResponse(res, "帖子删除成功");
        } else {
            sendErrorResponse(res, 400, "删除失败");
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleDeletePost: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// GET /api/v1/posts - 获取Feed流
void PostHandler::handleGetRecentPosts(const httplib::Request& req, httplib::Response& res) {
    try {
        Logger::info("=== [GET FEED] Request received ===");
        
        // ========================================
        // 第1步: 可选JWT认证（游客/登录用户统一处理）
        // ========================================
        int currentUserId = 0;  // 0表示游客
        bool isGuest = true;
        
        std::string token = extractToken(req);
        if (!token.empty()) {
            currentUserId = getUserIdFromToken(token);
            if (currentUserId > 0) {
                isGuest = false;
                Logger::info("[GET FEED] ✓ User authenticated - UserID: " + std::to_string(currentUserId));
            } else {
                Logger::warning("[GET FEED] ⚠ Invalid token, degrading to guest mode");
            }
        } else {
            Logger::info("[GET FEED] ℹ Guest mode - No token provided");
        }
        
        // ========================================
        // 第2步: 获取分页参数
        // ========================================
        int page = 1;
        int pageSize = 20;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }

        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
        }
        
        Logger::info("[GET FEED] Query params - Page: " + std::to_string(page) + 
                    ", PageSize: " + std::to_string(pageSize) + 
                    ", IsGuest: " + std::string(isGuest ? "true" : "false"));

        // ========================================
        // 第3步: 查询帖子列表（基础数据）
        // ========================================
        auto startTime = std::chrono::steady_clock::now();
        
        PostQueryResult result = postService_->getRecentPosts(page, pageSize);
        
        if (!result.success) {
            Logger::error("[GET FEED] ✗ Failed to query posts: " + result.message);
            sendErrorResponse(res, 400, result.message);
            return;
        }
        
        Logger::info("[GET FEED] ✓ Base posts queried: " + std::to_string(result.posts.size()) + 
                    " posts, Total: " + std::to_string(result.total));

        // ========================================
        // 第4步: 收集需要批量查询的ID
        // ========================================
        std::vector<int> postIds;
        std::vector<int> authorIds;
        
        for (const auto& post : result.posts) {
            postIds.push_back(post.getId());
            authorIds.push_back(post.getUserId());
        }
        
        // ========================================
        // 第5步: 批量查询作者信息（所有用户都需要）
        // ========================================
        std::unordered_map<int, User> authorMap;
        
        if (!authorIds.empty()) {
            authorMap = userService_->batchGetUsers(authorIds);
            Logger::info("[GET FEED] ✓ Authors queried: " + std::to_string(authorMap.size()) + 
                        "/" + std::to_string(authorIds.size()) + " authors found");
        }
        
        // ========================================
        // 第6步: 批量查询互动状态（仅登录用户）
        // ========================================
        std::unordered_map<int, bool> likeStatusMap;
        std::unordered_map<int, bool> favoriteStatusMap;
        
        if (!isGuest && !postIds.empty()) {
            // 登录用户：批量查询真实的点赞/收藏状态
            likeStatusMap = likeService_->batchCheckLikedStatus(currentUserId, postIds);
            favoriteStatusMap = favoriteService_->batchCheckFavoritedStatus(currentUserId, postIds);
            
            int likedCount = 0, favoritedCount = 0;
            for (const auto& pair : likeStatusMap) if (pair.second) likedCount++;
            for (const auto& pair : favoriteStatusMap) if (pair.second) favoritedCount++;
            
            Logger::info("[GET FEED] ✓ Interaction status queried - " +
                        std::to_string(likedCount) + " liked, " +
                        std::to_string(favoritedCount) + " favorited");
        } else {
            // 游客：不查询数据库，初始化为全false
            for (int postId : postIds) {
                likeStatusMap[postId] = false;
                favoriteStatusMap[postId] = false;
            }
            Logger::info("[GET FEED] ℹ Guest mode - Skipped interaction status query (performance optimization)");
        }
        
        // ========================================
        // 第7步: 组装JSON响应
        // ========================================
        Json::Value data;
        Json::Value postsArray(Json::arrayValue);

        for (const auto& post : result.posts) {
            Json::Value postJson = postToJson(post, true);
            
            // 添加作者信息
            auto authorIt = authorMap.find(post.getUserId());
            if (authorIt != authorMap.end()) {
                Json::Value authorInfo;
                authorInfo["user_id"] = authorIt->second.getUserId();  // 使用逻辑ID
                authorInfo["username"] = authorIt->second.getUsername();
                authorInfo["avatar_url"] = UrlHelper::toFullUrl(authorIt->second.getAvatarUrl());
                postJson["author"] = authorInfo;
            } else {
                // 作者信息缺失时的默认值
                Json::Value authorInfo;
                authorInfo["user_id"] = "";  // 逻辑ID缺失
                authorInfo["username"] = "Unknown";
                authorInfo["avatar_url"] = "";
                postJson["author"] = authorInfo;
            }
            
            // 添加互动状态（字段必须存在）
            postJson["has_liked"] = likeStatusMap[post.getId()];
            postJson["has_favorited"] = favoriteStatusMap[post.getId()];
            
            postsArray.append(postJson);
        }

        data["posts"] = postsArray;
        data["total"] = result.total;
        data["page"] = result.page;
        data["page_size"] = result.pageSize;
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        ).count();
        
        Logger::info("[GET FEED] ✓ Response assembled - Total time: " + 
                    std::to_string(duration) + "ms, " +
                    "Mode: " + std::string(isGuest ? "Guest" : "Authenticated"));

        sendSuccessResponse(res, "查询成功", data);

    } catch (const std::exception& e) {
        Logger::error("[GET FEED] ✗ Exception: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// GET /api/v1/users/:user_id/posts - 获取用户帖子列表
void PostHandler::handleGetUserPosts(const httplib::Request& req, httplib::Response& res) {
    try {
        Logger::info("=== [GET USER POSTS] Request received ===");
        
        // ========================================
        // 第1步: 可选JWT认证（游客/登录用户统一处理）
        // ========================================
        int currentUserId = 0;  // 0表示游客
        bool isGuest = true;
        
        std::string token = extractToken(req);
        if (!token.empty()) {
            currentUserId = getUserIdFromToken(token);
            if (currentUserId > 0) {
                isGuest = false;
                Logger::info("[GET USER POSTS] ✓ User authenticated - UserID: " + std::to_string(currentUserId));
            } else {
                Logger::warning("[GET USER POSTS] ⚠ Invalid token, degrading to guest mode");
            }
        } else {
            Logger::info("[GET USER POSTS] ℹ Guest mode - No token provided");
        }
        
        // ========================================
        // 第2步: 智能解析用户ID和分页参数
        // ========================================
        int targetUserId = 0;
        std::string userIdParam = req.path_params.at("user_id");
        
        // 智能判断: 纯数字=物理ID, 包含字母=逻辑ID
        if (std::all_of(userIdParam.begin(), userIdParam.end(), ::isdigit)) {
            // 场景1: 物理ID (纯数字,如"7")
            targetUserId = std::stoi(userIdParam);
            Logger::info("[GET USER POSTS] Physical user_id detected: " + userIdParam);
        } else {
            // 场景2: 逻辑ID (包含字母,如"USR_2025Q4_ABC123")
            Logger::info("[GET USER POSTS] Logical user_id detected: " + userIdParam);
            
            // 通过逻辑ID查询用户
            UserRepository userRepo;
            auto user = userRepo.findByUserId(userIdParam);
            
            if (!user.has_value()) {
                Logger::error("[GET USER POSTS] ✗ User not found: " + userIdParam);
                sendErrorResponse(res, 404, "用户不存在");
                return;
            }
            
            targetUserId = user->getId();  // 获取物理ID
            Logger::info("[GET USER POSTS] ✓ User found - Physical ID: " + 
                         std::to_string(targetUserId));
        }

        int page = 1;
        int pageSize = 20;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }

        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
        }
        
        Logger::info("[GET USER POSTS] Query params - Target UserID: " + std::to_string(targetUserId) +
                    ", Page: " + std::to_string(page) + 
                    ", PageSize: " + std::to_string(pageSize) + 
                    ", IsGuest: " + std::string(isGuest ? "true" : "false"));

        // ========================================
        // 第3步: 查询用户帖子列表（基础数据）
        // ========================================
        auto startTime = std::chrono::steady_clock::now();
        
        PostQueryResult result = postService_->getUserPosts(targetUserId, page, pageSize);

        if (!result.success) {
            Logger::error("[GET USER POSTS] ✗ Failed to query posts: " + result.message);
            sendErrorResponse(res, 400, result.message);
            return;
        }
        
        Logger::info("[GET USER POSTS] ✓ Base posts queried: " + std::to_string(result.posts.size()) + 
                    " posts, Total: " + std::to_string(result.total));

        // ========================================
        // 第4步: 收集需要批量查询的ID
        // ========================================
        std::vector<int> postIds;
        std::vector<int> authorIds;
        
        for (const auto& post : result.posts) {
            postIds.push_back(post.getId());
            authorIds.push_back(post.getUserId());
        }
        
        // ========================================
        // 第5步: 批量查询作者信息（所有用户都需要）
        // ========================================
        std::unordered_map<int, User> authorMap;
        
        if (!authorIds.empty()) {
            authorMap = userService_->batchGetUsers(authorIds);
            Logger::info("[GET USER POSTS] ✓ Authors queried: " + std::to_string(authorMap.size()) + 
                        "/" + std::to_string(authorIds.size()) + " authors found");
        }
        
        // ========================================
        // 第6步: 批量查询互动状态（仅登录用户）
        // ========================================
        std::unordered_map<int, bool> likeStatusMap;
        std::unordered_map<int, bool> favoriteStatusMap;
        
        if (!isGuest && !postIds.empty()) {
            // 登录用户：批量查询真实的点赞/收藏状态
            likeStatusMap = likeService_->batchCheckLikedStatus(currentUserId, postIds);
            favoriteStatusMap = favoriteService_->batchCheckFavoritedStatus(currentUserId, postIds);
            
            int likedCount = 0, favoritedCount = 0;
            for (const auto& pair : likeStatusMap) if (pair.second) likedCount++;
            for (const auto& pair : favoriteStatusMap) if (pair.second) favoritedCount++;
            
            Logger::info("[GET USER POSTS] ✓ Interaction status queried - " +
                        std::to_string(likedCount) + " liked, " +
                        std::to_string(favoritedCount) + " favorited");
        } else {
            // 游客：不查询数据库，初始化为全false
            for (int postId : postIds) {
                likeStatusMap[postId] = false;
                favoriteStatusMap[postId] = false;
            }
            Logger::info("[GET USER POSTS] ℹ Guest mode - Skipped interaction status query (performance optimization)");
        }
        
        // ========================================
        // 第7步: 组装JSON响应
        // ========================================
        Json::Value data;
        Json::Value postsArray(Json::arrayValue);

        for (const auto& post : result.posts) {
            Json::Value postJson = postToJson(post, true);
            
            // 添加作者信息
            auto authorIt = authorMap.find(post.getUserId());
            if (authorIt != authorMap.end()) {
                Json::Value authorInfo;
                authorInfo["user_id"] = authorIt->second.getUserId();  // 使用逻辑ID
                authorInfo["username"] = authorIt->second.getUsername();
                authorInfo["avatar_url"] = UrlHelper::toFullUrl(authorIt->second.getAvatarUrl());
                postJson["author"] = authorInfo;
            } else {
                // 作者信息缺失时的默认值
                Json::Value authorInfo;
                authorInfo["user_id"] = "";  // 逻辑ID缺失
                authorInfo["username"] = "Unknown";
                authorInfo["avatar_url"] = "";
                postJson["author"] = authorInfo;
            }
            
            // 添加互动状态（字段必须存在）
            postJson["has_liked"] = likeStatusMap[post.getId()];
            postJson["has_favorited"] = favoriteStatusMap[post.getId()];
            
            postsArray.append(postJson);
        }

        data["posts"] = postsArray;
        data["total"] = result.total;
        data["page"] = result.page;
        data["page_size"] = result.pageSize;
        
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            endTime - startTime
        ).count();
        
        Logger::info("[GET USER POSTS] ✓ Response assembled - Total time: " + 
                    std::to_string(duration) + "ms, " +
                    "Mode: " + std::string(isGuest ? "Guest" : "Authenticated"));

        sendSuccessResponse(res, "查询成功", data);

    } catch (const std::exception& e) {
        Logger::error("[GET USER POSTS] ✗ Exception: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// POST /api/v1/posts/:post_id/images - 向帖子添加图片
void PostHandler::handleAddImageToPost(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌
        std::string token = extractToken(req);
        if (token.empty()) {
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }

        int userId = getUserIdFromToken(token);
        if (userId == 0) {
            sendErrorResponse(res, 401, "无效的认证令牌");
            return;
        }

        // 2. 获取post_id
        std::string postId = req.path_params.at("post_id");

        // 3. 检查是否是multipart/form-data
        if (req.is_multipart_form_data()) {
            // 处理multipart/form-data格式
            if (!req.form.has_file("imageFile")) {
                sendErrorResponse(res, 400, "缺少图片文件");
                return;
            }

            auto imageData = req.form.get_file("imageFile");

            // 验证文件类型
            if (imageData.content_type.find("image/") != 0) {
                sendErrorResponse(res, 400, "只能上传图片文件");
                return;
            }

            // 验证文件大小（最大5MB）
            if (imageData.content.size() > 5 * 1024 * 1024) {
                sendErrorResponse(res, 400, "图片文件大小不能超过5MB");
                return;
            }

            // 保存文件到临时目录
            std::string savedPath = saveUploadedFile(
                imageData.content,
                imageData.filename,
                imageData.content_type
            );

            if (savedPath.empty()) {
                sendErrorResponse(res, 500, "保存图片文件失败");
                return;
            }

            // 调用Service添加图片
            bool success = postService_->addImageToPost(postId, userId, savedPath);

            // 清理临时文件
            std::remove(savedPath.c_str());

            // 返回结果
            if (success) {
                sendSuccessResponse(res, "图片添加成功");
            } else {
                sendErrorResponse(res, 400, "添加失败");
            }

        } else {
            // 处理JSON格式（用于向后兼容）
            Json::Value requestBody;
            if (!parseJsonBody(req.body, requestBody)) {
                sendErrorResponse(res, 400, "无效的JSON格式");
                return;
            }

            // 获取图片文件路径
            std::string imageFile = requestBody.get("imageFile", "").asString();
            if (imageFile.empty()) {
                sendErrorResponse(res, 400, "缺少图片文件路径");
                return;
            }

            // 调用Service添加图片
            bool success = postService_->addImageToPost(postId, userId, imageFile);

            // 返回结果
            if (success) {
                sendSuccessResponse(res, "图片添加成功");
            } else {
                sendErrorResponse(res, 400, "添加失败");
            }
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleAddImageToPost: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// DELETE /api/v1/posts/:post_id/images/:image_id - 删除帖子中的图片
void PostHandler::handleRemoveImageFromPost(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌
        std::string token = extractToken(req);
        if (token.empty()) {
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }

        int userId = getUserIdFromToken(token);
        if (userId == 0) {
            sendErrorResponse(res, 401, "无效的认证令牌");
            return;
        }

        // 2. 获取post_id和image_id
        std::string postId = req.path_params.at("post_id");
        std::string imageId = req.path_params.at("image_id");

        // 3. 调用Service删除图片
        bool success = postService_->removeImageFromPost(postId, imageId, userId);

        // 4. 返回结果
        if (success) {
            sendSuccessResponse(res, "图片删除成功");
        } else {
            sendErrorResponse(res, 400, "删除失败");
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleRemoveImageFromPost: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// PUT /api/v1/posts/:post_id/images/order - 调整图片顺序
void PostHandler::handleReorderImages(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 验证JWT令牌
        std::string token = extractToken(req);
        if (token.empty()) {
            sendErrorResponse(res, 401, "未提供认证令牌");
            return;
        }

        int userId = getUserIdFromToken(token);
        if (userId == 0) {
            sendErrorResponse(res, 401, "无效的认证令牌");
            return;
        }

        // 2. 获取post_id
        std::string postId = req.path_params.at("post_id");

        // 3. 解析请求体
        Json::Value requestBody;
        if (!parseJsonBody(req.body, requestBody)) {
            sendErrorResponse(res, 400, "无效的JSON格式");
            return;
        }

        // 4. 获取图片ID列表
        if (!requestBody.isMember("imageIds") || !requestBody["imageIds"].isArray()) {
            sendErrorResponse(res, 400, "缺少imageIds参数");
            return;
        }

        std::vector<std::string> imageIds;
        for (const auto& imageId : requestBody["imageIds"]) {
            imageIds.push_back(imageId.asString());
        }

        // 5. 调用Service调整顺序
        bool success = postService_->reorderImages(postId, userId, imageIds);

        // 6. 返回结果
        if (success) {
            sendSuccessResponse(res, "图片顺序调整成功");
        } else {
            sendErrorResponse(res, 400, "调整失败");
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleReorderImages: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// 保存上传的文件到临时目录
std::string PostHandler::saveUploadedFile(
    const std::string& content,
    const std::string& filename,
    const std::string& contentType
) {
    try {
        Logger::info("[SAVE FILE] Starting to process file - Name: " + filename + 
                    ", Type: " + contentType + 
                    ", OriginalSize: " + std::to_string(content.size()) + " bytes");
        
        // 0. 检测并解码Base64数据
        std::string actualContent = content;
        bool wasBase64 = false;
        auto decodeStart = std::chrono::high_resolution_clock::now();
        
        // 检查是否为Base64编码
        if (Base64Decoder::isBase64(content)) {
            Logger::info("[SAVE FILE] ⚠ Base64 encoded data detected!");
            Logger::info("[SAVE FILE]   → Data format: " + 
                        std::string(content.substr(0, 5) == "data:" ? "Data URI" : "Pure Base64"));
            Logger::info("[SAVE FILE]   → Encoded size: " + std::to_string(content.size()) + " bytes");
            
            try {
                actualContent = Base64Decoder::decode(content);
                wasBase64 = true;
                
                auto decodeEnd = std::chrono::high_resolution_clock::now();
                auto decodeDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    decodeEnd - decodeStart
                ).count();
                
                double compressionRatio = (1.0 - (double)actualContent.size() / content.size()) * 100;
                
                Logger::info("[SAVE FILE] ✓ Base64 decode successful!");
                Logger::info("[SAVE FILE]   → Decoded size: " + std::to_string(actualContent.size()) + " bytes");
                Logger::info("[SAVE FILE]   → Size reduction: " + 
                           std::to_string((int)compressionRatio) + "%");
                Logger::info("[SAVE FILE]   → Decode time: " + std::to_string(decodeDuration) + " ms");
                
                // 检查解码后的数据是否为有效图片
                if (actualContent.size() < 8) {
                    Logger::error("[SAVE FILE] ✗ Decoded data too small to be valid image");
                    return "";
                }
                
                // 检查PNG签名
                if (actualContent.size() >= 8) {
                    const unsigned char png_sig[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
                    bool isPNG = true;
                    for (int i = 0; i < 8; i++) {
                        if ((unsigned char)actualContent[i] != png_sig[i]) {
                            isPNG = false;
                            break;
                        }
                    }
                    if (isPNG) {
                        Logger::info("[SAVE FILE]   → Image format verified: PNG");
                    }
                }
                
                // 检查JPEG签名
                if (actualContent.size() >= 2 && 
                    (unsigned char)actualContent[0] == 0xFF && 
                    (unsigned char)actualContent[1] == 0xD8) {
                    Logger::info("[SAVE FILE]   → Image format verified: JPEG");
                }
                
            } catch (const std::exception& e) {
                Logger::error("[SAVE FILE] ✗ Base64 decode failed: " + std::string(e.what()));
                Logger::warning("[SAVE FILE]   → Falling back to treating as binary data");
                actualContent = content;
                wasBase64 = false;
            }
        } else {
            Logger::info("[SAVE FILE] Binary data detected (not Base64)");
            Logger::info("[SAVE FILE]   → Processing as direct binary upload");
        }

        // 1. 生成唯一文件名
        std::string extension = "";
        size_t dotPos = filename.find_last_of('.');
        if (dotPos != std::string::npos) {
            extension = filename.substr(dotPos);
        }

        // 使用时间戳和随机数生成唯一文件名
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ).count();

        std::string uniqueFilename = "upload_" + std::to_string(timestamp) +
                                    "_" + std::to_string(rand() % 10000) + extension;

        // 2. 确保临时目录存在
        std::string tempDir = "/tmp/knot_uploads";
        std::string mkdirCmd = "mkdir -p " + tempDir;
        int mkdirResult = system(mkdirCmd.c_str());
        if (mkdirResult != 0) {
            Logger::warning("[SAVE FILE] mkdir command returned: " + std::to_string(mkdirResult));
        }

        // 3. 保存文件（使用解码后的数据）
        std::string tempPath = tempDir + "/" + uniqueFilename;
        
        Logger::info("[SAVE FILE] Writing to: " + tempPath);
        
        std::ofstream file(tempPath, std::ios::binary);
        if (!file.is_open()) {
            Logger::error("[SAVE FILE] ✗ Failed to open file for writing: " + tempPath);
            return "";
        }

        file.write(actualContent.data(), actualContent.size());
        file.close();

        Logger::info("[SAVE FILE] ✓ File saved successfully!");
        Logger::info("[SAVE FILE]   → Path: " + tempPath);
        Logger::info("[SAVE FILE]   → Final size: " + std::to_string(actualContent.size()) + " bytes");
        Logger::info("[SAVE FILE]   → Mode: " + std::string(wasBase64 ? "Base64→Binary" : "Direct Binary"));

        return tempPath;

    } catch (const std::exception& e) {
        Logger::error("[SAVE FILE] ✗ Exception in saveUploadedFile: " + std::string(e.what()));
        return "";
    }
}

