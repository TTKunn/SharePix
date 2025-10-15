#!/bin/bash

# 图片上传日志分析脚本
# 用途：快速统计图片上传的成功率、Base64使用情况、错误类型等
# 作者：Knot Team
# 日期：2025-10-14

LOG_FILE="logs/auth-service.log"
LINES=1000

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}╔════════════════════════════════════════════════════════╗${NC}"
echo -e "${BLUE}║   图片上传日志分析（最近${LINES}行）                 ║${NC}"
echo -e "${BLUE}╚════════════════════════════════════════════════════════╝${NC}"
echo ""

# 检查日志文件是否存在
if [ ! -f "$LOG_FILE" ]; then
    echo -e "${RED}错误: 日志文件不存在: $LOG_FILE${NC}"
    exit 1
fi

# ============ 上传统计 ============
echo -e "${YELLOW}━━━ 上传统计 ━━━${NC}"

# 总上传次数
TOTAL=$(tail -$LINES $LOG_FILE | grep "CREATE POST.*Request received" | wc -l)
echo -e "总上传请求: ${GREEN}$TOTAL${NC}"

# 成功次数
SUCCESS=$(tail -$LINES $LOG_FILE | grep "CREATE POST.*✓.*successfully" | wc -l)
echo -e "成功次数: ${GREEN}$SUCCESS${NC}"

# 失败次数
FAIL=$(tail -$LINES $LOG_FILE | grep "CREATE POST.*✗" | wc -l)
echo -e "失败次数: ${RED}$FAIL${NC}"

# 成功率
if [ $TOTAL -gt 0 ]; then
    SUCCESS_RATE=$(echo "scale=2; $SUCCESS * 100 / $TOTAL" | bc)
    if (( $(echo "$SUCCESS_RATE >= 95" | bc -l) )); then
        echo -e "成功率: ${GREEN}${SUCCESS_RATE}%${NC} ✓ 优秀"
    elif (( $(echo "$SUCCESS_RATE >= 90" | bc -l) )); then
        echo -e "成功率: ${YELLOW}${SUCCESS_RATE}%${NC} ⚠ 良好"
    else
        echo -e "成功率: ${RED}${SUCCESS_RATE}%${NC} ✗ 需要优化"
    fi
else
    echo "成功率: N/A (无数据)"
fi

echo ""

# ============ Base64使用统计 ============
echo -e "${YELLOW}━━━ Base64使用统计 ━━━${NC}"

# Base64检测次数
BASE64=$(tail -$LINES $LOG_FILE | grep "Base64 encoded data detected" | wc -l)
echo -e "Base64上传次数: ${YELLOW}$BASE64${NC}"

# 二进制上传次数
BINARY=$(tail -$LINES $LOG_FILE | grep "Binary data detected" | wc -l)
echo -e "二进制上传次数: ${GREEN}$BINARY${NC}"

# Base64占比
if [ $(($BASE64 + $BINARY)) -gt 0 ]; then
    BASE64_RATE=$(echo "scale=2; $BASE64 * 100 / ($BASE64 + $BINARY)" | bc)
    if (( $(echo "$BASE64_RATE == 0" | bc -l) )); then
        echo -e "Base64占比: ${GREEN}${BASE64_RATE}%${NC} ✓ 完美（全部二进制）"
    elif (( $(echo "$BASE64_RATE < 50" | bc -l) )); then
        echo -e "Base64占比: ${YELLOW}${BASE64_RATE}%${NC} ⚠ 可接受"
    else
        echo -e "Base64占比: ${RED}${BASE64_RATE}%${NC} ⚠ 建议优化（建议使用二进制上传）"
    fi
else
    echo "Base64占比: N/A (无文件上传)"
fi

# Base64解码性能
if [ $BASE64 -gt 0 ]; then
    echo ""
    echo "Base64解码性能:"
    DECODE_TIMES=$(tail -$LINES $LOG_FILE | grep "Decode time:" | sed 's/.*Decode time: \([0-9]*\) ms.*/\1/')
    if [ ! -z "$DECODE_TIMES" ]; then
        AVG_TIME=$(echo "$DECODE_TIMES" | awk '{sum+=$1; count++} END {printf "%.1f", sum/count}')
        MAX_TIME=$(echo "$DECODE_TIMES" | sort -n | tail -1)
        echo -e "  平均解码时间: ${BLUE}${AVG_TIME}ms${NC}"
        echo -e "  最大解码时间: ${BLUE}${MAX_TIME}ms${NC}"
        
        if (( $(echo "$AVG_TIME < 5" | bc -l) )); then
            echo -e "  性能评级: ${GREEN}优秀${NC}"
        elif (( $(echo "$AVG_TIME < 10" | bc -l) )); then
            echo -e "  性能评级: ${YELLOW}良好${NC}"
        else
            echo -e "  性能评级: ${RED}需要优化${NC}"
        fi
    fi
fi

echo ""

# ============ 错误类型统计 ============
echo -e "${YELLOW}━━━ 错误类型统计 ━━━${NC}"

# Token错误
TOKEN_ERR=$(tail -$LINES $LOG_FILE | grep -c "Invalid authentication token")
if [ $TOKEN_ERR -gt 0 ]; then
    echo -e "Token验证失败: ${RED}$TOKEN_ERR${NC}"
else
    echo -e "Token验证失败: ${GREEN}$TOKEN_ERR${NC}"
fi

# 文件类型错误
TYPE_ERR=$(tail -$LINES $LOG_FILE | grep -c "Invalid content type")
if [ $TYPE_ERR -gt 0 ]; then
    echo -e "文件类型错误: ${RED}$TYPE_ERR${NC}"
else
    echo -e "文件类型错误: ${GREEN}$TYPE_ERR${NC}"
fi

# 文件大小错误
SIZE_ERR=$(tail -$LINES $LOG_FILE | grep -c "File size exceeds limit")
if [ $SIZE_ERR -gt 0 ]; then
    echo -e "文件过大错误: ${RED}$SIZE_ERR${NC}"
else
    echo -e "文件过大错误: ${GREEN}$SIZE_ERR${NC}"
fi

# Base64解码错误
DECODE_ERR=$(tail -$LINES $LOG_FILE | grep -c "Base64 decode failed")
if [ $DECODE_ERR -gt 0 ]; then
    echo -e "Base64解码失败: ${RED}$DECODE_ERR${NC}"
else
    echo -e "Base64解码失败: ${GREEN}$DECODE_ERR${NC}"
fi

# 数据库错误
DB_ERR=$(tail -$LINES $LOG_FILE | grep "CREATE POST" | grep -c "Database")
if [ $DB_ERR -gt 0 ]; then
    echo -e "数据库错误: ${RED}$DB_ERR${NC}"
else
    echo -e "数据库错误: ${GREEN}$DB_ERR${NC}"
fi

echo ""

# ============ 图片格式统计 ============
echo -e "${YELLOW}━━━ 图片格式统计 ━━━${NC}"

PNG_COUNT=$(tail -$LINES $LOG_FILE | grep "Image format verified: PNG" | wc -l)
JPEG_COUNT=$(tail -$LINES $LOG_FILE | grep "Image format verified: JPEG" | wc -l)

echo -e "PNG格式: ${BLUE}$PNG_COUNT${NC}"
echo -e "JPEG格式: ${BLUE}$JPEG_COUNT${NC}"

echo ""

# ============ 最近错误详情 ============
RECENT_ERRORS=$(tail -$LINES $LOG_FILE | grep -E "ERROR|✗" | tail -5)
if [ ! -z "$RECENT_ERRORS" ]; then
    echo -e "${YELLOW}━━━ 最近5条错误 ━━━${NC}"
    echo "$RECENT_ERRORS" | while read line; do
        echo -e "${RED}$line${NC}"
    done
    echo ""
fi

# ============ 建议 ============
echo -e "${YELLOW}━━━ 优化建议 ━━━${NC}"

if [ $(($BASE64 + $BINARY)) -gt 0 ]; then
    BASE64_RATE=$(echo "scale=2; $BASE64 * 100 / ($BASE64 + $BINARY)" | bc)
    if (( $(echo "$BASE64_RATE > 50" | bc -l) )); then
        echo -e "${YELLOW}⚠ 超过50%的上传使用Base64编码，建议前端改用二进制上传以提升性能${NC}"
    fi
fi

if [ $TOKEN_ERR -gt 5 ]; then
    echo -e "${YELLOW}⚠ Token验证失败次数较多（$TOKEN_ERR次），请检查Token刷新机制${NC}"
fi

if [ $DECODE_ERR -gt 0 ]; then
    echo -e "${YELLOW}⚠ 有Base64解码失败（$DECODE_ERR次），请检查前端编码逻辑${NC}"
fi

if [ $TOTAL -gt 0 ] && [ $SUCCESS -eq 0 ]; then
    echo -e "${RED}✗ 所有上传都失败了！请立即检查服务器状态${NC}"
fi

echo ""
echo -e "${GREEN}分析完成！${NC}"
echo ""
echo "提示: 要查看实时日志，请运行："
echo "  tail -f $LOG_FILE | grep -E 'CREATE POST|SAVE FILE'"

