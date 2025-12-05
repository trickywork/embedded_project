#!/bin/bash

echo "=========================================="
echo "  帕金森症状检测系统 - 电脑端测试"
echo "=========================================="
echo ""

# 检查PlatformIO是否安装
if ! command -v pio &> /dev/null; then
    echo "错误: PlatformIO未安装"
    echo "请运行: pip install platformio"
    exit 1
fi

# 编译项目
echo "正在编译测试程序..."
pio run -e native

if [ $? -ne 0 ]; then
    echo "编译失败！"
    exit 1
fi

echo ""
echo "编译成功！"
echo ""
echo "正在运行测试..."
echo ""

# 运行测试
pio run -e native -t exec

