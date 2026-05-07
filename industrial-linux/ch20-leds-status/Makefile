# SPDX-License-Identifier: GPL-2.0
# 配套 zhaoming-embedded ch20 (Linux 实战) + 附录 C
# 外部模块 build·不进内核源码树
#
# 用法 (在本目录下)：
#   make           # 编出 leds-status.ko
#   sudo insmod leds-status.ko
#   sudo rmmod leds-status
#   make clean

obj-m += leds-status.o

# 树外构建·指向当前运行内核的 build 目录
KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
