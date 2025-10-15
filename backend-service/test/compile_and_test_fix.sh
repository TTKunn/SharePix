#!/bin/bash

# 用户信息更新接口修复测试脚本
# 创建时间: 2025-10-15

echo "=== 编译测试程序 ==="
g++ -o test_update_profile_fix test_update_profile_fix.cpp \
    -I/usr/include/jsoncpp \
    -lcurl -ljsoncpp \
    -std=c++17

if [ $? -ne 0 ]; then
    echo "✗ 编译失败！"
    exit 1
fi

echo "✓ 编译成功！"
echo ""

echo "=== 运行测试 ==="
./test_update_profile_fix

TEST_RESULT=$?

echo ""
echo "=== 清理临时文件 ==="
rm -f test_update_profile_fix

if [ $TEST_RESULT -eq 0 ]; then
    echo "✓ 测试通过，修复成功！"
else
    echo "✗ 测试失败，请检查错误信息"
fi

exit $TEST_RESULT


