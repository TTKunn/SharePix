/**
 * 测试文件: test_update_profile_fix.cpp
 * 测试目的: 验证用户信息更新接口的修复
 * 测试内容:
 *   1. 测试bio、gender、location字段能否正常更新
 *   2. 测试avatar_url不会被误判为手机号
 *   3. 测试参数传递顺序是否正确
 * 创建时间: 2025-10-15
 * 测试完成后删除
 */

#include <iostream>
#include <string>
#include <curl/curl.h>
#include <json/json.h>

// 回调函数用于接收响应
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// 发送HTTP请求
std::string sendRequest(const std::string& url, const std::string& method, 
                       const std::string& token, const std::string& jsonData) {
    CURL* curl = curl_easy_init();
    std::string response;
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        if (!token.empty()) {
            std::string authHeader = "Authorization: Bearer " + token;
            headers = curl_slist_append(headers, authHeader.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        if (method == "PUT") {
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        } else if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        }
        
        curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
    
    return response;
}

int main() {
    std::cout << "=== 用户信息更新接口修复测试 ===" << std::endl << std::endl;
    
    const std::string BASE_URL = "http://localhost:8080/api/v1";
    std::string token;
    
    // 1. 先登录获取token
    std::cout << "步骤1: 登录获取Token..." << std::endl;
    Json::Value loginData;
    loginData["username"] = "testuser001";
    loginData["password"] = "Test123456";
    
    Json::StreamWriterBuilder writer;
    std::string loginJson = Json::writeString(writer, loginData);
    
    std::string loginResponse = sendRequest(BASE_URL + "/auth/login", "POST", "", loginJson);
    
    Json::CharReaderBuilder reader;
    Json::Value loginResult;
    std::istringstream loginStream(loginResponse);
    std::string errs;
    
    if (Json::parseFromStream(reader, loginStream, &loginResult, &errs)) {
        if (loginResult["success"].asBool()) {
            token = loginResult["data"]["access_token"].asString();
            std::cout << "✓ 登录成功，获得Token" << std::endl << std::endl;
        } else {
            std::cout << "✗ 登录失败: " << loginResult["message"].asString() << std::endl;
            return 1;
        }
    }
    
    // 2. 测试更新用户信息（包含所有字段）
    std::cout << "步骤2: 测试更新用户信息（包含bio、gender、location、avatar_url）..." << std::endl;
    Json::Value updateData;
    updateData["real_name"] = "测试用户";
    updateData["phone"] = "13900000001";
    updateData["email"] = "test@example.com";
    updateData["avatar_url"] = "https://example.com/avatar/test.jpg";  // 测试URL不会被误判为手机号
    updateData["bio"] = "这是我的个人简介，测试bio字段更新";
    updateData["gender"] = "male";
    updateData["location"] = "北京市朝阳区";
    
    std::string updateJson = Json::writeString(writer, updateData);
    std::string updateResponse = sendRequest(BASE_URL + "/users/profile", "PUT", token, updateJson);
    
    Json::Value updateResult;
    std::istringstream updateStream(updateResponse);
    
    if (Json::parseFromStream(reader, updateStream, &updateResult, &errs)) {
        std::cout << "响应: " << updateResult.toStyledString() << std::endl;
        
        if (updateResult["success"].asBool()) {
            std::cout << "✓ 更新成功！" << std::endl;
            
            // 验证各字段
            auto data = updateResult["data"];
            bool allCorrect = true;
            
            // 检查bio
            if (data["bio"].asString() == "这是我的个人简介，测试bio字段更新") {
                std::cout << "  ✓ bio字段正确: " << data["bio"].asString() << std::endl;
            } else {
                std::cout << "  ✗ bio字段错误: " << data["bio"].asString() << std::endl;
                allCorrect = false;
            }
            
            // 检查gender
            if (data["gender"].asString() == "male") {
                std::cout << "  ✓ gender字段正确: " << data["gender"].asString() << std::endl;
            } else {
                std::cout << "  ✗ gender字段错误: " << data["gender"].asString() << std::endl;
                allCorrect = false;
            }
            
            // 检查location
            if (data["location"].asString() == "北京市朝阳区") {
                std::cout << "  ✓ location字段正确: " << data["location"].asString() << std::endl;
            } else {
                std::cout << "  ✗ location字段错误: " << data["location"].asString() << std::endl;
                allCorrect = false;
            }
            
            // 检查avatar_url
            if (data["avatar_url"].asString().find("test.jpg") != std::string::npos) {
                std::cout << "  ✓ avatar_url字段正确: " << data["avatar_url"].asString() << std::endl;
            } else {
                std::cout << "  ✗ avatar_url字段错误: " << data["avatar_url"].asString() << std::endl;
                allCorrect = false;
            }
            
            // 检查phone
            if (data["phone"].asString() == "13900000001") {
                std::cout << "  ✓ phone字段正确: " << data["phone"].asString() << std::endl;
            } else {
                std::cout << "  ✗ phone字段错误: " << data["phone"].asString() << std::endl;
                allCorrect = false;
            }
            
            std::cout << std::endl;
            if (allCorrect) {
                std::cout << "🎉 所有测试通过！修复成功！" << std::endl;
                return 0;
            } else {
                std::cout << "❌ 部分测试失败！" << std::endl;
                return 1;
            }
        } else {
            std::cout << "✗ 更新失败: " << updateResult["message"].asString() << std::endl;
            return 1;
        }
    }
    
    return 0;
}


