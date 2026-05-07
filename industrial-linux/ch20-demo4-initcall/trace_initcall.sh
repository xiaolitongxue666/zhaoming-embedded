#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
#
# trace_initcall.sh
# 配套 zhaoming-embedded ch20.11·追踪 leds-status 模块的 init 链路
#
# 三件事：
#   1) 看启动期 initcall 时序 (内核自带·dmesg 就有)
#   2) 看 module_platform_driver 宏展开后的真实函数名
#   3) 用 ftrace 跟踪 status_led_probe 的调用栈
#
# 用法：
#   chmod +x trace_initcall.sh
#   ./trace_initcall.sh

set -e

echo "=== 1. 看启动期 initcall 时序 (内核启动时打的 log) ==="
# initcall_debug=1 在 cmdline 里开了才有这条日志·没开就空
sudo dmesg | grep -i 'initcall' | tail -20 || \
    echo "(没看到·内核 cmdline 没加 initcall_debug=1·这正常)"

echo
echo "=== 2. module_platform_driver 展开后的真实符号 ==="
echo "从 /proc/kallsyms 找 leds_status 模块的 init / exit 入口"
sudo cat /proc/kallsyms | grep -E 'leds_status|status_led_driver' || \
    echo "(模块没加载·先 sudo insmod leds-status.ko 再跑这条)"

echo
echo "=== 3. ftrace 跟踪 status_led_probe ==="
echo "设置 function tracer·只过滤 status_led_probe"

if [ ! -d /sys/kernel/debug/tracing ]; then
    echo "debugfs 没挂·先 sudo mount -t debugfs none /sys/kernel/debug"
    exit 1
fi

echo function | sudo tee /sys/kernel/debug/tracing/current_tracer
echo status_led_probe | sudo tee /sys/kernel/debug/tracing/set_ftrace_filter
echo 1 | sudo tee /sys/kernel/debug/tracing/tracing_on

echo
echo "现在 ftrace 已就绪·下一步："
echo "  sudo insmod leds-status.ko    # 触发 probe"
echo "  sudo cat /sys/kernel/debug/tracing/trace"
echo
echo "看完关掉跟踪："
echo "  echo 0 | sudo tee /sys/kernel/debug/tracing/tracing_on"
echo "  echo nop | sudo tee /sys/kernel/debug/tracing/current_tracer"
