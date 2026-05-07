# SPDX-License-Identifier: Apache-2.0
# 配套 zhaoming-embedded 在线书 ch19 + 附录 B
# 用法 (在本目录下)：
#   west build -b stm32f4_disco -p auto -- -DDEMO=1
#   west build -b stm32f4_disco -p auto -- -DDEMO=2
#   west build -b stm32f4_disco -p auto -- -DDEMO=3
#   west build -b stm32f4_disco -p auto -- -DDEMO=4

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(industrial_zephyr)

# 默认跑 demo1 (4 LED 同 ops 表)
if(NOT DEFINED DEMO)
    set(DEMO 1)
endif()

# 显式 if/elseif 把 DEMO 编号映射到 .c 文件·避免 wildcard 把多个文件同时编进去
if(DEMO EQUAL 1)
    target_sources(app PRIVATE src/main_demo1_4led.c)
    message(STATUS "[industrial-zephyr] DEMO=1 4 LED 同 ops 表 (ch19/19.7 向上转型)")
elseif(DEMO EQUAL 2)
    target_sources(app PRIVATE src/main_demo2_overlay.c)
    message(STATUS "[industrial-zephyr] DEMO=2 overlay 改 label (ch19/19.8 dts 解耦)")
elseif(DEMO EQUAL 3)
    target_sources(app PRIVATE src/main_demo3_container.c)
    message(STATUS "[industrial-zephyr] DEMO=3 CONTAINER_OF 抓回调 (ch19/19.9)")
elseif(DEMO EQUAL 4)
    target_sources(app PRIVATE src/main_demo4_enosys.c)
    message(STATUS "[industrial-zephyr] DEMO=4 可空 ops 看 -ENOSYS (ch19/19.10)")
else()
    message(FATAL_ERROR "DEMO 只能是 1/2/3/4·当前 DEMO=${DEMO}")
endif()
