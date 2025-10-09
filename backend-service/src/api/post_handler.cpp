/**
 * @file post_handler.cpp
 * @brief 帖子API处理器实现
 * @author Knot Team
 * @date 2025-10-08
 */

#include "api/post_handler.h"
#include "utils/logger.h"
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <chrono>
#include <cstdlib>

// 构造函数
PostHandler::PostHandler() {
    postService_ = std::make_unique<PostService>();
    Logger::info("PostHandler initialized");
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
    
    // 获取用户帖子列表
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
    json["file_url"] = image.getFileUrl();
    json["thumbnail_url"] = image.getThumbnailUrl();
    json["file_size"] = static_cast<Json::Int64>(image.getFileSize());
    json["width"] = image.getWidth();
    json["height"] = image.getHeight();
    json["mime_type"] = image.getMimeType();
    json["create_time"] = static_cast<Json::Int64>(image.getCreateTime());
    return json;
}

// 将Post对象转换为JSON
Json::Value PostHandler::postToJson(const Post& post, bool includeImages) {
    Json::Value json;
    json["post_id"] = post.getPostId();
    json["user_id"] = post.getUserId();
    json["title"] = post.getTitle();
    json["description"] = post.getDescription();
    json["image_count"] = post.getImageCount();
    json["like_count"] = post.getLikeCount();
    json["favorite_count"] = post.getFavoriteCount();
    json["view_count"] = post.getViewCount();
    json["status"] = Post::statusToString(post.getStatus());
    json["create_time"] = static_cast<Json::Int64>(post.getCreateTime());
    json["update_time"] = static_cast<Json::Int64>(post.getUpdateTime());
    
    if (includeImages) {
        Json::Value imagesArray(Json::arrayValue);
        for (const auto& image : post.getImages()) {
            imagesArray.append(imageToJson(image));
        }
        json["images"] = imagesArray;
    }
    
    return json;
}

// POST /api/v1/posts - 创建帖子
void PostHandler::handleCreatePost(const httplib::Request& req, httplib::Response& res) {
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

        // 2. 检查是否是multipart/form-data
        if (!req.is_multipart_form_data()) {
            sendErrorResponse(res, 400, "请求必须使用multipart/form-data格式");
            return;
        }

        // 3. 获取表单字段
        std::string title;
        std::string description;
        std::vector<std::string> tags;

        // 获取title（必填）
        if (req.form.has_field("title")) {
            title = req.form.get_field("title");
        }
        if (title.empty()) {
            sendErrorResponse(res, 400, "标题不能为空");
            return;
        }

        // 获取description（可选）
        if (req.form.has_field("description")) {
            description = req.form.get_field("description");
        }

        // 获取tags（可选，可以有多个）
        if (req.form.has_field("tags")) {
            tags = req.form.get_fields("tags");
        }

        // 4. 保存上传的图片文件到临时目录
        std::vector<std::string> savedImagePaths;

        // 获取所有上传的图片文件
        if (req.form.has_file("imageFiles")) {
            auto imageFiles = req.form.get_files("imageFiles");

            for (const auto& fileData : imageFiles) {
                // 验证文件类型
                if (fileData.content_type.find("image/") != 0) {
                    sendErrorResponse(res, 400, "只能上传图片文件");
                    return;
                }

                // 验证文件大小（最大5MB）
                if (fileData.content.size() > 5 * 1024 * 1024) {
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
                    sendErrorResponse(res, 500, "保存图片文件失败");
                    return;
                }

                savedImagePaths.push_back(savedPath);
            }
        }

        // 5. 验证图片数量
        if (savedImagePaths.empty() || savedImagePaths.size() > 9) {
            // 清理已保存的临时文件
            for (const auto& path : savedImagePaths) {
                std::remove(path.c_str());
            }
            sendErrorResponse(res, 400, "图片数量必须在1-9张之间");
            return;
        }

        // 6. 调用Service创建帖子
        PostCreateResult result = postService_->createPost(
            userId,
            title,
            description,
            savedImagePaths,
            tags
        );

        // 7. 清理临时文件
        for (const auto& path : savedImagePaths) {
            std::remove(path.c_str());
        }

        // 8. 返回结果
        if (result.success) {
            Json::Value data;
            data["post"] = postToJson(result.post, true);
            sendSuccessResponse(res, "帖子创建成功", data);
        } else {
            sendErrorResponse(res, 400, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleCreatePost: " + std::string(e.what()));
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
        // 1. 获取分页参数
        int page = 1;
        int pageSize = 20;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }

        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
        }

        // 2. 调用Service查询帖子列表
        PostQueryResult result = postService_->getRecentPosts(page, pageSize);

        // 3. 返回结果
        if (result.success) {
            Json::Value data;
            Json::Value postsArray(Json::arrayValue);

            for (const auto& post : result.posts) {
                postsArray.append(postToJson(post, true));
            }

            data["posts"] = postsArray;
            data["total"] = result.total;
            data["page"] = result.page;
            data["page_size"] = result.pageSize;

            sendSuccessResponse(res, "查询成功", data);
        } else {
            sendErrorResponse(res, 400, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetRecentPosts: " + std::string(e.what()));
        sendErrorResponse(res, 500, "服务器内部错误");
    }
}

// GET /api/v1/users/:user_id/posts - 获取用户帖子列表
void PostHandler::handleGetUserPosts(const httplib::Request& req, httplib::Response& res) {
    try {
        // 1. 获取user_id
        int userId = std::stoi(req.path_params.at("user_id"));

        // 2. 获取分页参数
        int page = 1;
        int pageSize = 20;

        if (req.has_param("page")) {
            page = std::stoi(req.get_param_value("page"));
        }

        if (req.has_param("page_size")) {
            pageSize = std::stoi(req.get_param_value("page_size"));
        }

        // 3. 调用Service查询帖子列表
        PostQueryResult result = postService_->getUserPosts(userId, page, pageSize);

        // 4. 返回结果
        if (result.success) {
            Json::Value data;
            Json::Value postsArray(Json::arrayValue);

            for (const auto& post : result.posts) {
                postsArray.append(postToJson(post, true));
            }

            data["posts"] = postsArray;
            data["total"] = result.total;
            data["page"] = result.page;
            data["page_size"] = result.pageSize;

            sendSuccessResponse(res, "查询成功", data);
        } else {
            sendErrorResponse(res, 400, result.message);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in handleGetUserPosts: " + std::string(e.what()));
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

        // 3. 解析请求体（假设前端已上传图片到临时目录）
        Json::Value requestBody;
        if (!parseJsonBody(req.body, requestBody)) {
            sendErrorResponse(res, 400, "无效的JSON格式");
            return;
        }

        // 4. 获取图片文件路径
        std::string imageFile = requestBody.get("image_file", "").asString();
        if (imageFile.empty()) {
            sendErrorResponse(res, 400, "缺少图片文件路径");
            return;
        }

        // 5. 调用Service添加图片
        bool success = postService_->addImageToPost(postId, userId, imageFile);

        // 6. 返回结果
        if (success) {
            sendSuccessResponse(res, "图片添加成功");
        } else {
            sendErrorResponse(res, 400, "添加失败");
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
        if (!requestBody.isMember("image_ids") || !requestBody["image_ids"].isArray()) {
            sendErrorResponse(res, 400, "缺少image_ids参数");
            return;
        }

        std::vector<std::string> imageIds;
        for (const auto& imageId : requestBody["image_ids"]) {
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
        system(mkdirCmd.c_str());

        // 3. 保存文件
        std::string tempPath = tempDir + "/" + uniqueFilename;
        std::ofstream file(tempPath, std::ios::binary);
        if (!file.is_open()) {
            Logger::error("Failed to open file for writing: " + tempPath);
            return "";
        }

        file.write(content.data(), content.size());
        file.close();

        Logger::info("Saved uploaded file: " + tempPath + " (" +
                    std::to_string(content.size()) + " bytes)");

        return tempPath;

    } catch (const std::exception& e) {
        Logger::error("Exception in saveUploadedFile: " + std::string(e.what()));
        return "";
    }
}

