/**
 * @file post_service.cpp
 * @brief 帖子服务实现
 * @author Knot Team
 * @date 2025-10-12
 */

#include "core/post_service.h"
#include "core/image_service.h"
#include "database/post_repository.h"
#include "database/image_repository.h"
#include "database/tag_repository.h"
#include "database/transaction_manager.h"
#include "utils/logger.h"
#include <random>
#include <sstream>
#include <chrono>

// ============================================================================
// 构造函数和析构函数
// ============================================================================

PostService::PostService() {
    postRepo_ = std::make_unique<PostRepository>();
    imageService_ = std::make_unique<ImageService>();
    imageRepo_ = std::make_unique<ImageRepository>();
    tagRepo_ = std::make_unique<TagRepository>();
    Logger::info("PostService initialized");
}

PostService::~PostService() {
    Logger::info("PostService destroyed");
}

// ============================================================================
// 公共方法实现
// ============================================================================

/**
 * 创建帖子
 */
PostCreateResult PostService::createPost(
    int userId,
    const std::string& title,
    const std::string& description,
    const std::vector<std::string>& imagePaths,
    const std::vector<std::string>& tags
) {
    PostCreateResult result;

    try {
        Logger::info("Creating post for user ID: " + std::to_string(userId));

        // 1. 验证标题
        if (title.empty() || title.length() > 255) {
            result.message = "标题长度必须在1-255字符之间";
            Logger::warning(result.message);
            return result;
        }

        // 2. 验证描述
        if (description.length() > 5000) {
            result.message = "描述长度不能超过5000字符";
            Logger::warning(result.message);
            return result;
        }

        // 3. 验证图片数量
        if (!validateImageCount(imagePaths.size())) {
            result.message = "图片数量必须在1-9张之间";
            Logger::warning(result.message);
            return result;
        }

        // 4. 生成帖子ID
        std::string postId = generatePostId();

        // 5. 构建Post对象
        Post post;
        post.setPostId(postId);
        post.setUserId(userId);
        post.setTitle(title);
        post.setDescription(description);
        post.setImageCount(imagePaths.size());
        post.setStatus(PostStatus::APPROVED);  // 默认审核通过
        post.setLikeCount(0);
        post.setFavoriteCount(0);
        post.setViewCount(0);

        // 6. 创建帖子记录
        if (!postRepo_->createPost(post)) {
            result.message = "创建帖子记录失败";
            Logger::error(result.message);
            return result;
        }

        Logger::info("Post created with ID: " + postId + ", physical ID: " + std::to_string(post.getId()));

        // 7. 处理图片上传
        std::vector<Image> savedImages;
        int actualImageCount = 0;

        for (size_t i = 0; i < imagePaths.size(); i++) {
            Logger::info("Processing image " + std::to_string(i + 1) + "/" + std::to_string(imagePaths.size()));

            // 调用imageService处理图片（压缩、缩略图）
            std::vector<std::string> emptyTags;
            ImageUploadResult imgResult = imageService_->uploadImage(
                userId,
                imagePaths[i],
                title,  // 使用帖子标题作为图片标题
                "",
                emptyTags
            );

            if (!imgResult.success) {
                Logger::warning("Image processing failed: " + imgResult.message);
                // 继续处理下一张图片，但记录失败
                continue;
            }

            // 设置图片的postId和displayOrder
            Image image = imgResult.image;
            image.setPostId(post.getId());
            image.setDisplayOrder(actualImageCount);  // 使用实际保存的图片数量作为顺序

            if (!imageRepo_->createImage(image)) {
                Logger::warning("Failed to save image record: " + image.getImageId());
                // 继续处理其他图片
            } else {
                Logger::info("Image saved successfully: " + image.getImageId());
                savedImages.push_back(image);
                actualImageCount++;
            }
        }

        // 8. 验证至少有一张图片成功上传
        if (savedImages.empty()) {
            Logger::error("No images were successfully processed");
            // 删除刚创建的帖子记录
            postRepo_->deletePost(postId);
            result.message = "所有图片处理失败，帖子创建失败";
            return result;
        }

        // 9. 始终更新帖子的实际图片数量（修复数据一致性问题）
        post.setImageCount(actualImageCount);
        postRepo_->updatePost(post);
        Logger::info("Updated post image count to: " + std::to_string(actualImageCount));

        // 10. 处理标签关联
        for (const auto& tagName : tags) {
            if (tagName.empty()) continue;

            Logger::info("Processing tag: " + tagName);

            // 查找标签是否存在
            auto tagOpt = tagRepo_->findByName(tagName);
            int tagId = 0;

            if (!tagOpt.has_value()) {
                // 标签不存在，创建新标签
                Tag newTag;
                newTag.setName(tagName);
                newTag.setUseCount(0);

                if (!tagRepo_->createTag(newTag)) {
                    Logger::warning("Failed to create tag: " + tagName);
                    continue;  // 跳过此标签，不影响帖子创建
                }

                // 重新查询获取标签ID
                tagOpt = tagRepo_->findByName(tagName);
                if (!tagOpt.has_value()) {
                    Logger::warning("Failed to retrieve created tag: " + tagName);
                    continue;
                }
            }

            tagId = tagOpt->getId();

            // 关联帖子和标签
            if (!tagRepo_->linkPostTag(post.getId(), tagId)) {
                Logger::warning("Failed to link post and tag: " + tagName);
                continue;  // 跳过此标签
            }

            // 增加标签使用次数
            tagRepo_->incrementUseCount(tagId);
            Logger::info("Tag linked successfully: " + tagName);
        }

        // 11. 查询完整帖子信息（包含图片）
        auto postWithImages = postRepo_->findByPostIdWithImages(postId);
        if (!postWithImages.has_value()) {
            // 如果查询失败，使用原始post对象
            result.success = true;
            result.message = "帖子创建成功（图片加载失败）";
            result.post = post;
            Logger::warning("Post created but failed to load with images");
            return result;
        }

        // 12. 返回成功结果
        result.success = true;
        result.message = "帖子创建成功";
        result.post = *postWithImages;
        Logger::info("Post created successfully: " + postId);
        return result;

    } catch (const std::exception& e) {
        result.message = "创建帖子异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

/**
 * 获取帖子详情
 */
std::optional<Post> PostService::getPostDetail(const std::string& postId, bool includeImages) {
    try {
        Logger::info("Fetching post detail: " + postId + ", includeImages=" + (includeImages ? "true" : "false"));

        if (includeImages) {
            return postRepo_->findByPostIdWithImages(postId);
        } else {
            return postRepo_->findByPostId(postId);
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in getPostDetail: " + std::string(e.what()));
        return std::nullopt;
    }
}

/**
 * 更新帖子信息
 */
bool PostService::updatePost(
    const std::string& postId,
    int userId,
    const std::string& title,
    const std::string& description
) {
    try {
        Logger::info("Updating post: " + postId + " by user: " + std::to_string(userId));

        // 1. 验证标题
        if (title.empty() || title.length() > 255) {
            Logger::warning("Invalid title length");
            return false;
        }

        // 2. 验证描述
        if (description.length() > 5000) {
            Logger::warning("Description too long");
            return false;
        }

        // 3. 查询帖子是否存在
        auto postOpt = postRepo_->findByPostId(postId);
        if (!postOpt.has_value()) {
            Logger::warning("Post not found: " + postId);
            return false;
        }

        // 4. 权限验证
        if (!checkOwnership(postId, userId)) {
            Logger::warning("User " + std::to_string(userId) + " does not own post " + postId);
            return false;
        }

        // 5. 更新Post对象
        Post post = *postOpt;
        post.setTitle(title);
        post.setDescription(description);

        // 6. 调用Repository更新
        if (!postRepo_->updatePost(post)) {
            Logger::error("Failed to update post in database");
            return false;
        }

        Logger::info("Post updated successfully: " + postId);
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in updatePost: " + std::string(e.what()));
        return false;
    }
}

/**
 * 删除帖子
 */
bool PostService::deletePost(const std::string& postId, int userId) {
    try {
        Logger::info("Deleting post: " + postId + " by user: " + std::to_string(userId));

        // 1. 权限验证
        if (!checkOwnership(postId, userId)) {
            Logger::warning("User " + std::to_string(userId) + " does not own post " + postId);
            return false;
        }

        // 2. 删除帖子（级联删除由数据库外键处理）
        if (!postRepo_->deletePost(postId)) {
            Logger::error("Failed to delete post: " + postId);
            return false;
        }

        Logger::info("Post deleted successfully: " + postId);
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in deletePost: " + std::string(e.what()));
        return false;
    }
}

/**
 * 获取最新帖子列表
 */
PostQueryResult PostService::getRecentPosts(int page, int pageSize, bool includeImages) {
    PostQueryResult result;

    try {
        Logger::info("Fetching recent posts: page=" + std::to_string(page) +
                     ", pageSize=" + std::to_string(pageSize) +
                     ", includeImages=" + (includeImages ? "true" : "false"));

        // 1. 参数验证
        if (page < 1) {
            result.message = "页码必须大于等于1";
            Logger::warning(result.message);
            return result;
        }

        if (pageSize <= 0 || pageSize > 100) {
            result.message = "每页数量必须在1-100之间";
            Logger::warning(result.message);
            return result;
        }

        // 2. 查询帖子列表
        std::vector<Post> posts;
        if (includeImages) {
            posts = postRepo_->getRecentPostsWithImages(page, pageSize);
        } else {
            posts = postRepo_->getRecentPosts(page, pageSize);
        }

        // 3. 查询总数
        int totalCount = postRepo_->getTotalCount();

        // 4. 构建结果
        result.success = true;
        result.message = "查询成功";
        result.posts = posts;
        result.total = totalCount;
        result.page = page;
        result.pageSize = pageSize;

        Logger::info("Fetched " + std::to_string(posts.size()) + " posts, total: " + std::to_string(totalCount));
        return result;

    } catch (const std::exception& e) {
        result.message = "查询帖子列表异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

/**
 * 获取用户帖子列表
 */
PostQueryResult PostService::getUserPosts(int userId, int page, int pageSize, bool includeImages) {
    PostQueryResult result;

    try {
        Logger::info("Fetching user posts: userId=" + std::to_string(userId) +
                     ", page=" + std::to_string(page) +
                     ", pageSize=" + std::to_string(pageSize) +
                     ", includeImages=" + (includeImages ? "true" : "false"));

        // 1. 参数验证
        if (page < 1) {
            result.message = "页码必须大于等于1";
            Logger::warning(result.message);
            return result;
        }

        if (pageSize <= 0 || pageSize > 100) {
            result.message = "每页数量必须在1-100之间";
            Logger::warning(result.message);
            return result;
        }

        // 2. 查询用户帖子列表
        std::vector<Post> posts;
        if (includeImages) {
            posts = postRepo_->findByUserIdWithImages(userId, page, pageSize);
        } else {
            posts = postRepo_->findByUserId(userId, page, pageSize);
        }

        // 3. 查询用户帖子总数
        int totalCount = postRepo_->getUserPostCount(userId);

        // 4. 构建结果
        result.success = true;
        result.message = "查询成功";
        result.posts = posts;
        result.total = totalCount;
        result.page = page;
        result.pageSize = pageSize;

        Logger::info("Fetched " + std::to_string(posts.size()) + " posts for user " +
                     std::to_string(userId) + ", total: " + std::to_string(totalCount));
        return result;

    } catch (const std::exception& e) {
        result.message = "查询用户帖子列表异常: " + std::string(e.what());
        Logger::error(result.message);
        return result;
    }
}

/**
 * 增加浏览数
 */
bool PostService::incrementViewCount(const std::string& postId) {
    try {
        Logger::info("Incrementing view count for post: " + postId);
        return postRepo_->incrementViewCount(postId);
    } catch (const std::exception& e) {
        Logger::error("Exception in incrementViewCount: " + std::string(e.what()));
        return false;
    }
}

// ============================================================================
// 私有辅助方法实现
// ============================================================================

/**
 * 更新帖子的图片数量
 */
bool PostService::updateImageCount(const std::string& postId, int newCount) {
    try {
        Logger::info("Updating post image count: " + postId + " -> " + std::to_string(newCount));

        // 1. 验证帖子是否存在
        auto postOpt = postRepo_->findByPostId(postId);
        if (!postOpt.has_value()) {
            Logger::error("Post not found for image count update: " + postId);
            return false;
        }

        // 2. 更新图片数量
        if (!postRepo_->updateImageCount(postId, newCount)) {
            Logger::error("Failed to update post image count: " + postId);
            return false;
        }

        Logger::info("Post image count updated successfully: " + postId + " -> " + std::to_string(newCount));
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in updateImageCount: " + std::string(e.what()));
        return false;
    }
}

/**
 * 重新计算并更新帖子的图片数量
 */
bool PostService::recalculateImageCount(const std::string& postId) {
    try {
        Logger::info("Recalculating image count for post: " + postId);

        // 1. 验证帖子是否存在
        auto postOpt = postRepo_->findByPostId(postId);
        if (!postOpt.has_value()) {
            Logger::error("Post not found for image count recalculation: " + postId);
            return false;
        }

        // 2. 获取实际的图片数量
        std::vector<Image> images = imageRepo_->findByPostId(postOpt->getId());
        int actualCount = static_cast<int>(images.size());

        // 3. 更新图片数量
        if (updateImageCount(postId, actualCount)) {
            Logger::info("Image count recalculated successfully: " + postId + " -> " + std::to_string(actualCount));
            return true;
        } else {
            Logger::error("Failed to update image count after recalculation: " + postId);
            return false;
        }

    } catch (const std::exception& e) {
        Logger::error("Exception in recalculateImageCount: " + std::string(e.what()));
        return false;
    }
}

// ============================================================================
// 以下方法实现图片管理功能
// ============================================================================

/**
 * 向帖子添加图片
 */
bool PostService::addImageToPost(const std::string& postId, int userId, const std::string& imagePath) {
    try {
        Logger::info("Adding image to post: postId=" + postId + ", userId=" + std::to_string(userId) +
                     ", imagePath=" + imagePath);

        // 1. 验证帖子所有权
        if (!checkOwnership(postId, userId)) {
            Logger::warning("User " + std::to_string(userId) + " does not own post " + postId);
            return false;
        }

        // 2. 查询帖子信息
        auto postOpt = postRepo_->findByPostId(postId);
        if (!postOpt.has_value()) {
            Logger::error("Post not found: " + postId);
            return false;
        }

        Post post = *postOpt;

        // 3. 检查当前图片数量（不能超过9张）
        int currentImageCount = post.getImageCount();
        if (currentImageCount >= 9) {
            Logger::warning("Post already has maximum 9 images");
            return false;
        }

        // 4. 处理图片（压缩、缩略图）
        std::vector<std::string> emptyTags;
        ImageUploadResult imgResult = imageService_->uploadImage(
            userId,
            imagePath,
            post.getTitle(),  // 使用帖子标题作为图片标题
            "",
            emptyTags
        );

        if (!imgResult.success) {
            Logger::error("Failed to process image: " + imgResult.message);
            return false;
        }

        // 5. 设置图片的postId和displayOrder后保存到数据库
        Image image = imgResult.image;
        image.setPostId(post.getId());
        image.setDisplayOrder(currentImageCount);  // 新图片顺序为当前图片数量

        // 6. 保存图片记录到数据库
        if (!imageRepo_->createImage(image)) {
            Logger::error("Failed to save image record: " + image.getImageId());
            return false;
        }

        // 7. 更新帖子的图片数量
        if (!updateImageCount(postId, currentImageCount + 1)) {
            Logger::error("Failed to update post image count");
            // 尝试删除刚保存的图片记录
            imageRepo_->deleteImage(image.getImageId());
            return false;
        }

        Logger::info("Image added successfully to post: " + postId);
        return true;

    } catch (const std::exception& e) {
        Logger::error("Exception in addImageToPost: " + std::string(e.what()));
        return false;
    }
}

/**
 * 从帖子删除图片
 */
bool PostService::removeImageFromPost(const std::string& postId, const std::string& imageId, int userId) {
    Logger::info("Removing image from post: postId=" + postId + ", imageId=" + imageId +
                 ", userId=" + std::to_string(userId));

    // 使用事务保护整个操作
    return executeInTransaction([this, postId, imageId, userId](MYSQL* conn) -> bool {
        // 1. 验证帖子所有权
        if (!checkOwnership(postId, userId)) {
            Logger::warning("User " + std::to_string(userId) + " does not own post " + postId);
            return false;
        }

        // 2. 查询帖子信息
        auto postOpt = postRepo_->findByPostId(postId);
        if (!postOpt.has_value()) {
            Logger::error("Post not found: " + postId);
            return false;
        }

        Post post = *postOpt;

        // 3. 检查当前图片数量（至少要保留1张）
        int currentImageCount = post.getImageCount();
        if (currentImageCount <= 1) {
            Logger::warning("Cannot remove last image from post (minimum 1 required)");
            return false;
        }

        // 4. 删除图片
        if (!imageService_->deleteImage(imageId, userId)) {
            Logger::error("Failed to delete image: " + imageId);
            return false;
        }

        // 5. 重新计算并更新帖子的图片数量（更可靠的方法）
        if (!recalculateImageCount(postId)) {
            Logger::error("Failed to recalculate post image count");
            return false;
        }

        Logger::info("Image removed successfully from post: " + postId);
        return true;
    });
}

/**
 * 调整图片顺序
 */
bool PostService::reorderImages(const std::string& postId, int userId, const std::vector<std::string>& imageIds) {
    Logger::info("Reordering images for post: postId=" + postId + ", userId=" + std::to_string(userId) +
                 ", imageCount=" + std::to_string(imageIds.size()));

    // 使用事务保护整个排序操作
    return executeInTransaction([this, postId, userId, imageIds](MYSQL* conn) -> bool {
        // 1. 验证帖子所有权
        if (!checkOwnership(postId, userId)) {
            Logger::warning("User " + std::to_string(userId) + " does not own post " + postId);
            return false;
        }

        // 2. 查询帖子信息
        auto postOpt = postRepo_->findByPostId(postId);
        if (!postOpt.has_value()) {
            Logger::error("Post not found: " + postId);
            return false;
        }

        Post post = *postOpt;

        // 3. 验证图片ID数量是否与帖子的图片数量匹配
        if (static_cast<int>(imageIds.size()) != post.getImageCount()) {
            Logger::error("Image count mismatch: expected " + std::to_string(post.getImageCount()) +
                         ", got " + std::to_string(imageIds.size()));
            return false;
        }

        // 4. 验证图片数量在有效范围内（1-9）
        if (!validateImageCount(imageIds.size())) {
            Logger::error("Invalid image count for reordering");
            return false;
        }

        // 5. 更新每张图片的显示顺序（现在在事务保护下）
        for (size_t i = 0; i < imageIds.size(); i++) {
            const std::string& imageId = imageIds[i];
            int newOrder = static_cast<int>(i);

            Logger::info("Updating image " + imageId + " to order " + std::to_string(newOrder));

            if (!imageService_->updateDisplayOrder(imageId, newOrder)) {
                Logger::error("Failed to update display order for image: " + imageId);
                return false; // 事务会自动回滚
            }
        }

        Logger::info("Images reordered successfully for post: " + postId);
        return true;
    });
}

// ============================================================================
// 私有辅助方法实现
// ============================================================================

/**
 * 生成唯一帖子ID
 */
std::string PostService::generatePostId() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);

    int year = tm->tm_year + 1900;
    int quarter = (tm->tm_mon / 3) + 1;

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
    oss << "POST_" << year << "Q" << quarter << "_" << randomPart;

    return oss.str();
}

/**
 * 验证帖子信息
 */
bool PostService::validatePost(const Post& post) {
    // 验证标题：1-255字符
    if (post.getTitle().empty() || post.getTitle().length() > 255) {
        Logger::warning("Invalid post title length");
        return false;
    }

    // 验证描述：最多5000字符
    if (post.getDescription().length() > 5000) {
        Logger::warning("Description too long");
        return false;
    }

    // 验证用户ID
    if (post.getUserId() <= 0) {
        Logger::warning("Invalid user ID");
        return false;
    }

    return true;
}

/**
 * 验证图片数量
 */
bool PostService::validateImageCount(int imageCount) {
    bool valid = (imageCount >= 1 && imageCount <= 9);
    if (!valid) {
        Logger::warning("Invalid image count: " + std::to_string(imageCount) + " (must be 1-9)");
    }
    return valid;
}

/**
 * 检查用户是否拥有帖子
 */
bool PostService::checkOwnership(const std::string& postId, int userId) {
    try {
        auto postOpt = postRepo_->findByPostId(postId);
        if (!postOpt.has_value()) {
            Logger::warning("Post not found for ownership check: " + postId);
            return false;
        }

        bool isOwner = (postOpt->getUserId() == userId);
        if (!isOwner) {
            Logger::warning("User " + std::to_string(userId) +
                          " is not owner of post " + postId +
                          " (owner is " + std::to_string(postOpt->getUserId()) + ")");
        }
        return isOwner;

    } catch (const std::exception& e) {
        Logger::error("Exception in checkOwnership: " + std::string(e.what()));
        return false;
    }
}
