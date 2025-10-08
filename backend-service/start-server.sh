#!/bin/bash

# ============================================
# 共享停车后端服务启动脚本
# ============================================

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${BLUE}============================================${NC}"
echo -e "${BLUE}  共享停车后端服务${NC}"
echo -e "${BLUE}============================================${NC}"
echo ""

# 检查可执行文件是否存在
if [ ! -f "build/shared_parking_auth" ]; then
    echo -e "${RED}错误: 找不到可执行文件 build/shared_parking_auth${NC}"
    echo -e "${YELLOW}请先编译项目:${NC}"
    echo -e "  cd build"
    echo -e "  cmake .."
    echo -e "  make"
    exit 1
fi

# 检查配置文件是否存在
if [ ! -f "config/config.json" ]; then
    echo -e "${RED}错误: 找不到配置文件 config/config.json${NC}"
    echo -e "${YELLOW}请从 config.example.json 复制并修改配置${NC}"
    exit 1
fi

# 显示配置信息
echo -e "${YELLOW}配置信息:${NC}"
DB_HOST=$(grep -A 10 '"database"' config/config.json | grep '"host"' | cut -d'"' -f4)
DB_PORT=$(grep -A 10 '"database"' config/config.json | grep '"port"' | cut -d':' -f2 | tr -d ' ,')
DB_NAME=$(grep -A 10 '"database"' config/config.json | grep '"database"' | tail -1 | cut -d'"' -f4)
SERVER_PORT=$(grep -A 5 '"server"' config/config.json | grep '"port"' | cut -d':' -f2 | tr -d ' ,')

echo -e "  数据库: ${BLUE}$DB_HOST:$DB_PORT/$DB_NAME${NC}"
echo -e "  服务端口: ${BLUE}$SERVER_PORT${NC}"
echo ""

# 测试数据库连接
echo -e "${YELLOW}测试数据库连接...${NC}"
if command -v mysql &> /dev/null; then
    DB_USER=$(grep -A 10 '"database"' config/config.json | grep '"username"' | cut -d'"' -f4)
    DB_PASS=$(grep -A 10 '"database"' config/config.json | grep '"password"' | cut -d'"' -f4)
    
    if mysql -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASS" -e "SELECT 1;" &> /dev/null; then
        echo -e "${GREEN}✓ 数据库连接正常${NC}"
    else
        echo -e "${RED}✗ 数据库连接失败${NC}"
        echo -e "${YELLOW}请检查数据库配置和网络连接${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}⚠ 未安装MySQL客户端，跳过数据库连接测试${NC}"
fi
echo ""

# 创建日志目录
mkdir -p logs

# 启动服务
echo -e "${GREEN}启动服务...${NC}"
echo -e "${YELLOW}按 Ctrl+C 停止服务${NC}"
echo ""
echo -e "${BLUE}============================================${NC}"
echo ""

# 运行服务
./build/shared_parking_auth

# 服务停止后的清理
echo ""
echo -e "${BLUE}============================================${NC}"
echo -e "${YELLOW}服务已停止${NC}"
echo -e "${BLUE}============================================${NC}"

