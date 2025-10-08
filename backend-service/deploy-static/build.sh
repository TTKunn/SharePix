#!/bin/bash
# Knot 静态编译脚本
# 用于生成完全静态链接的可执行文件，便于服务器部署
# 
# 使用方法:
#   cd backend-service/deploy-static
#   ./build.sh

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Knot 静态编译脚本${NC}"
echo -e "${BLUE}  Static Build for Production Deployment${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${CYAN}工作目录: ${SCRIPT_DIR}${NC}"
echo ""

# ============================================
# 1. 检查编译工具
# ============================================
echo -e "${YELLOW}[1/9] 检查编译工具...${NC}"

if ! command -v gcc &> /dev/null; then
    echo -e "${RED}✗ 错误: 未找到 gcc${NC}"
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo -e "${RED}✗ 错误: 未找到 g++${NC}"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo -e "${RED}✗ 错误: 未找到 cmake${NC}"
    exit 1
fi

GCC_VERSION=$(gcc --version | head -1)
CMAKE_VERSION=$(cmake --version | head -1)
echo -e "${GREEN}✓ ${GCC_VERSION}${NC}"
echo -e "${GREEN}✓ ${CMAKE_VERSION}${NC}"
echo ""

# ============================================
# 2. 检查静态库
# ============================================
echo -e "${YELLOW}[2/9] 检查静态库依赖...${NC}"

SYSTEM_LIB_PATH="/usr/lib/x86_64-linux-gnu"
MISSING_LIBS=0

check_static_lib() {
    local lib_name=$1
    local lib_file=$2
    
    if [ -f "$lib_file" ]; then
        echo -e "${GREEN}✓ ${lib_name}: ${lib_file}${NC}"
        return 0
    else
        echo -e "${RED}✗ ${lib_name}: 未找到 ${lib_file}${NC}"
        return 1
    fi
}

# 检查必需的静态库
check_static_lib "MySQL" "${SYSTEM_LIB_PATH}/libmysqlclient.a" || MISSING_LIBS=$((MISSING_LIBS + 1))
check_static_lib "OpenSSL(ssl)" "${SYSTEM_LIB_PATH}/libssl.a" || MISSING_LIBS=$((MISSING_LIBS + 1))
check_static_lib "OpenSSL(crypto)" "${SYSTEM_LIB_PATH}/libcrypto.a" || MISSING_LIBS=$((MISSING_LIBS + 1))

# 检查JsonCpp动态库（Ubuntu不提供静态库）
if [ -f "${SYSTEM_LIB_PATH}/libjsoncpp.so" ]; then
    echo -e "${GREEN}✓ JsonCpp: ${SYSTEM_LIB_PATH}/libjsoncpp.so (dynamic)${NC}"
else
    echo -e "${RED}✗ JsonCpp: 未找到${NC}"
    MISSING_LIBS=$((MISSING_LIBS + 1))
fi

if [ $MISSING_LIBS -gt 0 ]; then
    echo ""
    echo -e "${RED}错误: 缺少 ${MISSING_LIBS} 个静态库${NC}"
    echo -e "${YELLOW}请运行以下命令安装依赖:${NC}"
    echo -e "  ${CYAN}sudo apt install -y libmysqlclient-dev libjsoncpp-dev libssl-dev${NC}"
    exit 1
fi
echo ""

# ============================================
# 3. 禁用Anaconda环境
# ============================================
echo -e "${YELLOW}[3/9] 配置编译环境（禁用Anaconda）...${NC}"

# 保存原始环境变量
ORIGINAL_PATH="$PATH"
ORIGINAL_LD_LIBRARY_PATH="$LD_LIBRARY_PATH"

# 临时禁用Anaconda
export PATH="/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin"
unset LD_LIBRARY_PATH
unset CONDA_PREFIX
unset CONDA_DEFAULT_ENV

echo -e "${GREEN}✓ 已禁用Anaconda环境${NC}"
echo -e "${CYAN}  PATH: ${PATH}${NC}"
echo ""

# ============================================
# 4. 清理旧的构建
# ============================================
echo -e "${YELLOW}[4/9] 清理旧的构建...${NC}"

if [ -d "build" ]; then
    rm -rf build
    echo -e "${GREEN}✓ 已清理 build 目录${NC}"
fi
mkdir -p build
echo ""

# ============================================
# 5. 配置CMake
# ============================================
echo -e "${YELLOW}[5/9] 配置CMake（静态编译模式）...${NC}"
cd build

cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_FIND_LIBRARY_SUFFIXES=".a" \
      -DBUILD_SHARED_LIBS=OFF \
      ..

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ CMake配置失败${NC}"
    # 恢复环境变量
    export PATH="$ORIGINAL_PATH"
    export LD_LIBRARY_PATH="$ORIGINAL_LD_LIBRARY_PATH"
    exit 1
fi
echo -e "${GREEN}✓ CMake配置成功${NC}"
echo ""

# ============================================
# 6. 编译项目
# ============================================
echo -e "${YELLOW}[6/9] 编译项目...${NC}"
CPU_CORES=$(nproc)
echo -e "${CYAN}使用 ${CPU_CORES} 个CPU核心编译${NC}"

make -j${CPU_CORES}

if [ $? -ne 0 ]; then
    echo -e "${RED}✗ 编译失败${NC}"
    # 恢复环境变量
    export PATH="$ORIGINAL_PATH"
    export LD_LIBRARY_PATH="$ORIGINAL_LD_LIBRARY_PATH"
    exit 1
fi
echo -e "${GREEN}✓ 编译成功${NC}"
echo ""

# 恢复环境变量
export PATH="$ORIGINAL_PATH"
export LD_LIBRARY_PATH="$ORIGINAL_LD_LIBRARY_PATH"

# ============================================
# 7. 检查可执行文件
# ============================================
echo -e "${YELLOW}[7/9] 检查可执行文件...${NC}"

if [ ! -f "knot_image_sharing" ]; then
    echo -e "${RED}✗ 未找到可执行文件${NC}"
    exit 1
fi

FILE_SIZE=$(du -h knot_image_sharing | cut -f1)
echo -e "${GREEN}✓ 可执行文件: knot_image_sharing (${FILE_SIZE})${NC}"
echo ""

# ============================================
# 8. 检查动态库依赖
# ============================================
echo -e "${YELLOW}[8/9] 检查动态库依赖...${NC}"
echo -e "${CYAN}动态库依赖列表:${NC}"
ldd knot_image_sharing

echo ""
# 统计非系统库依赖
NON_SYSTEM_LIBS=$(ldd knot_image_sharing | grep -v "linux-vdso\|ld-linux\|libc.so\|libm.so\|libpthread.so\|libdl.so\|librt.so\|libresolv.so\|libgcc_s.so\|libstdc++.so\|libz.so" | grep "=>" | wc -l)

if [ $NON_SYSTEM_LIBS -eq 0 ]; then
    echo -e "${GREEN}✓ 完美！仅依赖系统库${NC}"
else
    echo -e "${YELLOW}⚠ 仍有 ${NON_SYSTEM_LIBS} 个非系统库依赖${NC}"
    echo -e "${YELLOW}  这些库需要在目标服务器上安装${NC}"
fi
echo ""

# ============================================
# 9. 创建部署包
# ============================================
echo -e "${YELLOW}[9/9] 创建部署包...${NC}"

TIMESTAMP=$(date +%Y%m%d-%H%M%S)
DEPLOY_DIR="knot-deploy-${TIMESTAMP}"
mkdir -p "$DEPLOY_DIR"

# 复制可执行文件
cp knot_image_sharing "$DEPLOY_DIR/"
echo -e "${GREEN}✓ 已复制可执行文件${NC}"

# 复制配置文件
mkdir -p "$DEPLOY_DIR/config"
cp ../../config/config.json "$DEPLOY_DIR/config/config.production.json"
echo -e "${GREEN}✓ 已复制配置文件${NC}"

# 复制API文档
mkdir -p "$DEPLOY_DIR/docs"
cp -r ../../docs/api "$DEPLOY_DIR/docs/"
echo -e "${GREEN}✓ 已复制API文档${NC}"

# 创建启动脚本
cat > "$DEPLOY_DIR/start.sh" << 'EOF'
#!/bin/bash
# Knot 服务启动脚本

cd "$(dirname "$0")"

echo "启动 Knot 图片分享服务..."
./knot_image_sharing config/config.production.json
EOF
chmod +x "$DEPLOY_DIR/start.sh"
echo -e "${GREEN}✓ 已创建启动脚本${NC}"

# 创建README
cat > "$DEPLOY_DIR/README.txt" << 'EOF'
Knot 图片分享系统 - 部署包

文件说明:
- knot_image_sharing: 后端API服务可执行文件（静态编译）
- config/config.production.json: 生产环境配置文件
- docs/api/: API在线文档
- start.sh: 服务启动脚本

部署步骤:
1. 修改 config/config.production.json 中的配置
   - 数据库地址、用户名、密码
   - JWT密钥（必须修改！）
   - 日志路径

2. 启动服务:
   ./start.sh

3. 启动API文档（可选）:
   cd docs/api && ./start-api-docs.sh

注意事项:
- 确保服务器能访问数据库
- 确保端口8080和8081未被占用
- 建议使用systemd管理服务
- 日志文件位置: logs/auth-service.log

服务地址:
- 后端API: http://服务器IP:8080/api/v1/
- API文档: http://服务器IP:8081/api-docs.html
EOF
echo -e "${GREEN}✓ 已创建README${NC}"

# 打包
tar -czf "${DEPLOY_DIR}.tar.gz" "$DEPLOY_DIR"
echo -e "${GREEN}✓ 部署包已创建: ${DEPLOY_DIR}.tar.gz${NC}"
echo ""

# ============================================
# 完成
# ============================================
echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}  ✓ 编译完成！${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "${YELLOW}输出文件:${NC}"
echo -e "  可执行文件: ${CYAN}${SCRIPT_DIR}/build/knot_image_sharing${NC}"
echo -e "  部署包: ${CYAN}${SCRIPT_DIR}/build/${DEPLOY_DIR}.tar.gz${NC}"
echo ""
echo -e "${YELLOW}文件大小:${NC}"
echo -e "  可执行文件: ${FILE_SIZE}"
PACKAGE_SIZE=$(du -h "build/${DEPLOY_DIR}.tar.gz" | cut -f1)
echo -e "  部署包: ${PACKAGE_SIZE}"
echo ""
echo -e "${YELLOW}下一步:${NC}"
echo -e "  1. 上传部署包到服务器:"
echo -e "     ${CYAN}scp build/${DEPLOY_DIR}.tar.gz user@server:/opt/${NC}"
echo -e "  2. 在服务器上解压:"
echo -e "     ${CYAN}tar -xzf ${DEPLOY_DIR}.tar.gz${NC}"
echo -e "  3. 修改配置:"
echo -e "     ${CYAN}vim ${DEPLOY_DIR}/config/config.production.json${NC}"
echo -e "  4. 启动服务:"
echo -e "     ${CYAN}cd ${DEPLOY_DIR} && ./start.sh${NC}"
echo ""

