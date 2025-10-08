#!/bin/bash
# Knot 网络访问检查脚本
# 用于检查虚拟机网络配置和服务状态

echo "========================================="
echo "  Knot 网络访问检查"
echo "========================================="
echo ""

# 1. 检查IP地址
echo "1. 虚拟机IP地址:"
VM_IP=$(ip addr show ens33 2>/dev/null | grep "inet " | awk '{print $2}' | cut -d'/' -f1)
if [ -z "$VM_IP" ]; then
    echo "  ❌ 无法获取IP地址"
    echo "  请检查网络接口是否启用"
else
    echo "  ✅ $VM_IP"
fi
echo ""

# 2. 检查服务状态
echo "2. 服务运行状态:"
echo "   后端API服务 (端口8080):"
if netstat -tlnp 2>/dev/null | grep -q ":8080"; then
    LISTEN_ADDR=$(netstat -tlnp 2>/dev/null | grep ":8080" | awk '{print $4}')
    echo "  ✅ 运行中 - 监听地址: $LISTEN_ADDR"
    if echo "$LISTEN_ADDR" | grep -q "0.0.0.0:8080"; then
        echo "     ✅ 可从局域网访问"
    elif echo "$LISTEN_ADDR" | grep -q "127.0.0.1:8080"; then
        echo "     ⚠️  仅本地访问（需修改配置为0.0.0.0）"
    fi
else
    echo "  ❌ 未运行"
    echo "     请启动后端服务: ./build/knot_image_sharing"
fi
echo ""

echo "   API文档服务 (端口8081):"
if netstat -tlnp 2>/dev/null | grep -q ":8081"; then
    LISTEN_ADDR=$(netstat -tlnp 2>/dev/null | grep ":8081" | awk '{print $4}')
    echo "  ✅ 运行中 - 监听地址: $LISTEN_ADDR"
    if echo "$LISTEN_ADDR" | grep -q "0.0.0.0:8081"; then
        echo "     ✅ 可从局域网访问"
    elif echo "$LISTEN_ADDR" | grep -q "127.0.0.1:8081"; then
        echo "     ⚠️  仅本地访问（需修改配置为0.0.0.0）"
    fi
else
    echo "  ❌ 未运行"
    echo "     请启动文档服务: cd docs/api && ./start-api-docs.sh"
fi
echo ""

# 3. 检查防火墙（跳过需要sudo的检查）
echo "3. 防火墙状态:"
if command -v ufw &> /dev/null; then
    echo "  ℹ️  检测到UFW防火墙"
    echo "  如需检查状态，请运行: sudo ufw status"
    echo "  如需开放端口，请运行:"
    echo "    sudo ufw allow 8080/tcp"
    echo "    sudo ufw allow 8081/tcp"
elif command -v firewall-cmd &> /dev/null; then
    if systemctl is-active --quiet firewalld 2>/dev/null; then
        echo "  ⚠️  firewalld已启用"
        echo "  请检查端口是否开放: sudo firewall-cmd --list-ports"
    else
        echo "  ✅ firewalld未启用"
    fi
else
    echo "  ✅ 未检测到防火墙"
fi
echo ""

# 4. 网络连通性测试
echo "4. 网络连通性:"
if [ -n "$VM_IP" ]; then
    echo "  本机回环测试:"
    if ping -c 1 -W 1 127.0.0.1 &> /dev/null; then
        echo "    ✅ 本地网络正常"
    else
        echo "    ❌ 本地网络异常"
    fi
    
    echo "  网络接口测试:"
    if ping -c 1 -W 1 "$VM_IP" &> /dev/null; then
        echo "    ✅ 网络接口正常"
    else
        echo "    ⚠️  网络接口可能有问题"
    fi
fi
echo ""

# 5. 访问地址
echo "5. 访问地址（从Windows宿主机）:"
if [ -n "$VM_IP" ]; then
    echo "  📱 API文档:"
    echo "     http://$VM_IP:8081/api-docs.html"
    echo ""
    echo "  🔌 后端API:"
    echo "     http://$VM_IP:8080/api/v1/"
    echo ""
    echo "  🏥 健康检查:"
    echo "     http://$VM_IP:8080/health"
else
    echo "  ❌ 无法生成访问地址（IP地址未获取）"
fi
echo ""

# 6. 快速测试命令
echo "6. Windows测试命令:"
if [ -n "$VM_IP" ]; then
    echo "  在Windows PowerShell中运行:"
    echo "  ping $VM_IP"
    echo "  curl http://$VM_IP:8080/health"
    echo "  curl http://$VM_IP:8081/api-docs.html"
fi
echo ""

# 7. 总结
echo "========================================="
echo "  检查总结"
echo "========================================="
echo ""

# 统计问题
ISSUES=0

if [ -z "$VM_IP" ]; then
    echo "❌ IP地址未获取"
    ISSUES=$((ISSUES + 1))
fi

if ! netstat -tlnp 2>/dev/null | grep -q ":8080"; then
    echo "❌ 后端API服务未运行"
    ISSUES=$((ISSUES + 1))
fi

if ! netstat -tlnp 2>/dev/null | grep -q ":8081"; then
    echo "❌ API文档服务未运行"
    ISSUES=$((ISSUES + 1))
fi

if [ $ISSUES -eq 0 ]; then
    echo "✅ 所有检查通过！"
    echo ""
    echo "您现在可以从Windows宿主机访问："
    echo "  http://$VM_IP:8081/api-docs.html"
else
    echo "⚠️  发现 $ISSUES 个问题，请根据上述提示解决"
fi

echo ""
echo "========================================="

