#!/bin/bash
# 文档重命名脚本
# 创建时间: 2025-10-19
# 说明: 按照文档重新编号方案重命名所有文档

set -e  # 遇到错误立即退出

echo "================================"
echo "项目文档重新编号脚本"
echo "================================"
echo ""

# 切换到文档目录
cd "$(dirname "$0")"
SCRIPT_DIR=$(pwd)

echo "当前目录: $SCRIPT_DIR"
echo ""

# 创建备份目录
BACKUP_DIR="backup_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$BACKUP_DIR"
echo "✓ 创建备份目录: $BACKUP_DIR"

# 备份所有markdown文件
cp *.md "$BACKUP_DIR/" 2>/dev/null || true
echo "✓ 已备份所有文档到: $BACKUP_DIR"
echo ""

echo "开始重命名..."
echo "---"

# 计数器
SUCCESS_COUNT=0
SKIP_COUNT=0
ERROR_COUNT=0

# 函数：安全重命名
rename_file() {
    local OLD_NAME="$1"
    local NEW_NAME="$2"
    local DESC="$3"
    
    if [ -f "$OLD_NAME" ]; then
        if [ -f "$NEW_NAME" ]; then
            echo "⚠ 跳过: $DESC (目标文件已存在)"
            ((SKIP_COUNT++))
        else
            mv "$OLD_NAME" "$NEW_NAME" && {
                echo "✓ $DESC"
                ((SUCCESS_COUNT++))
            } || {
                echo "✗ 失败: $DESC"
                ((ERROR_COUNT++))
            }
        fi
    else
        echo "⚠ 跳过: $DESC (源文件不存在)"
        ((SKIP_COUNT++))
    fi
}

# 阶段实现文档 [100-149]
echo "阶段实现文档 [100-149]:"
rename_file "[111]阶段D-4-图片静态文件服务实现方案.md" "[109]阶段D-4-图片静态文件服务实现方案.md" "[111] -> [109] 阶段D-4"

# 问题修复与优化文档 [150-179]
echo ""
echo "问题修复与优化文档 [150-179]:"
rename_file "[113]全流程测试报告-帖子管理重点.md" "[150]全流程测试报告-帖子管理重点.md" "[113] -> [150] 全流程测试报告"
rename_file "[114]多图片功能修复和实现报告.md" "[151]多图片功能修复和实现报告.md" "[114] -> [151] 多图片功能修复"
rename_file "[112]Base64图片上传问题分析与解决方案.md" "[152]Base64图片上传问题分析与解决方案.md" "[112] -> [152] Base64问题分析"
rename_file "[113]日志监控指南-图片上传排查手册.md" "[153]日志监控指南-图片上传排查手册.md" "[113] -> [153] 日志监控指南"
rename_file "[115]创建帖子接口支持JSON+Base64格式-实施计划.md" "[154]创建帖子接口支持JSON+Base64格式-实施计划.md" "[115] -> [154] JSON+Base64实施计划"
rename_file "[116]JSON+Base64内存崩溃紧急修复报告.md" "[155]JSON+Base64内存崩溃紧急修复报告.md" "[116] -> [155] 内存崩溃修复"
rename_file "创建帖子请求流程文档.md" "[156]创建帖子请求流程文档.md" "无编号 -> [156] 创建帖子流程"
rename_file "[109]用户信息更新接口BUG修复报告.md" "[157]用户信息更新接口BUG修复报告.md" "[109] -> [157] 用户信息BUG修复"
rename_file "[110]Feed流性能优化报告.md" "[158]Feed流性能优化报告.md" "[110] -> [158] Feed流优化"
rename_file "[112]JWT测试.md" "[159]JWT测试.md" "[112] -> [159] JWT测试"
rename_file "[110]v2.4.0-图片URL自动前缀功能.md" "[160]v2.4.0-图片URL自动前缀功能.md" "[110] -> [160] v2.4.0功能"
rename_file "[122]关注功能测试问题汇总.md" "[161]关注功能测试问题汇总.md" "[122] -> [161] 关注功能问题"
rename_file "[121]关注功能完整测试报告.md" "[162]关注功能完整测试报告.md" "[121] -> [162] 关注功能测试"
rename_file "[117]帖子接口用户ID逻辑化改造方案.md" "[163]帖子接口用户ID逻辑化改造方案.md" "[117] -> [163] 用户ID改造方案"
rename_file "[117]帖子接口用户ID逻辑化改造-实施报告.md" "[164]帖子接口用户ID逻辑化改造-实施报告.md" "[117] -> [164] 用户ID改造实施"
rename_file "[118]Feed流用户状态批量查询优化方案.md" "[165]Feed流用户状态批量查询优化方案.md" "[118] -> [165] Feed流批量查询"
rename_file "[119]帖子详情接口添加用户昵称字段-实施方案.md" "[166]帖子详情接口添加用户昵称字段-实施方案.md" "[119] -> [166] 帖子详情昵称"
rename_file "[120]用户头像上传功能-实施方案.md" "[167]用户头像上传功能-实施方案.md" "[120] -> [167] 头像上传实施"
rename_file "[123]获取用户帖子接口问题修复方案.md" "[168]获取用户帖子接口问题修复方案.md" "[123] -> [168] 用户帖子接口"
rename_file "[121]头像上传功能内存泄漏问题分析报告.md" "[169]头像上传功能内存泄漏问题分析报告.md" "[121] -> [169] 内存泄漏分析"

# 其他文档
echo ""
echo "其他文档:"
rename_file "API_DOC_UPDATE.md" "[005]API_DOC_UPDATE.md" "无编号 -> [005] API更新文档"
rename_file "README.md" "[004]README.md" "无编号 -> [004] README"

echo ""
echo "---"
echo "重命名完成！"
echo ""
echo "统计信息："
echo "  成功: $SUCCESS_COUNT 个"
echo "  跳过: $SKIP_COUNT 个"
echo "  失败: $ERROR_COUNT 个"
echo ""
echo "备份文件保存在: $BACKUP_DIR"
echo ""

# 显示重命名后的文档列表
echo "重命名后的文档列表："
echo "---"
ls -1 [0-9]*.md 2>/dev/null | head -20
echo ""

if [ $ERROR_COUNT -gt 0 ]; then
    echo "⚠ 警告: 有 $ERROR_COUNT 个文件重命名失败，请检查"
    exit 1
else
    echo "✓ 所有文档重命名成功！"
    exit 0
fi

