#!/bin/bash

# Knot API文档服务器启动脚本
# 使用Python的http.server模块提供静态文件服务

PORT=8081
HOST="0.0.0.0"

# 获取脚本所在目录
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "========================================="
echo "  Knot API 文档服务器"
echo "========================================="
echo ""
echo "启动中..."
echo "访问地址: http://localhost:$PORT/api-docs.html"
echo "文档目录: $SCRIPT_DIR"
echo ""
echo "按 Ctrl+C 停止服务器"
echo "========================================="
echo ""

# 切换到文档目录并启动HTTP服务器
cd "$SCRIPT_DIR"
python3 -m http.server $PORT --bind $HOST

