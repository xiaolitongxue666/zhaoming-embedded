# SPDX-License-Identifier: GPL-2.0
# 配套 ch20.7 用户态 libgpiod demo
#
# 前置：sudo apt install libgpiod-dev
# 用法：make && sudo ./blink_libgpiod

CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra

blink_libgpiod: blink_libgpiod.c
	$(CC) $(CFLAGS) -o $@ $< -lgpiod

clean:
	rm -f blink_libgpiod

.PHONY: clean
