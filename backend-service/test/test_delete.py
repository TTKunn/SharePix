#!/usr/bin/env python3
# 测试DELETE请求是否能收到响应

import requests

TOKEN = "eyJhbGciOiJIUzI1NiJ9.eyJleHAiOjE3NTk4NTQwMTUsImlhdCI6MTc1OTg1MDQxNSwiaXNzIjoic2hhcmVkLXBhcmtpbmctYXV0aCIsInN1YiI6IjIiLCJ1c2VybmFtZSI6InRlc3R1c2VyIn0.kdiUdF4SAYQjxwQqV-Y_1QmLLZN8UlUs5rcaYBICbvo"

url = "http://localhost:8080/api/v1/images/IMG_2025Q4_ZQKXY0"
headers = {"Authorization": f"Bearer {TOKEN}"}

print("Sending DELETE request...")
response = requests.delete(url, headers=headers, timeout=10)

print(f"Status Code: {response.status_code}")
print(f"Headers: {response.headers}")
print(f"Content: {response.text}")

