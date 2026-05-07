# SPDX-License-Identifier: MIT
#
# Makefile - Linux 用户态完整工程
#
# 单一 build 模式. 依赖 libgpiod-dev 装上:
#   sudo apt install libgpiod-dev      # Debian / Ubuntu / 树莓派 OS
#   sudo dnf install libgpiod-devel    # Fedora / RHEL
#
# 编译:
#   make
#
# 真机跑 (需要 GPIO / PWM / I2C 权限, 一般要 sudo 或加 i2c / gpio group):
#   sudo ./build/firmware
#
# 仅 syntax check (Windows / 没 libgpiod-dev 的环境):
#   make check-syntax
# 这个 target 用 syntax_stubs/ 下的占位头文件代替 <gpiod.h> / <linux/i2c-dev.h>,
# 跑 gcc -fsyntax-only 验证 .c 文件语法和头文件依赖是否合法.

TARGET   = firmware
BUILDDIR = build

# CC 默认 gcc. 用 GNU make 在 Linux 上 cc -> gcc 软链常见, Windows / MSYS2
# 上没有 cc 软链, 这里强制 gcc 让两边都跑得通. 用别的工具链交叉编译时
# 命令行覆盖即可: make CC=aarch64-linux-gnu-gcc
CC       = gcc
LD       = $(CC)

SRCS    = src/main.c \
          app/drivers/led/led_base.c \
          app/drivers/led/led_gpio.c \
          app/drivers/led/led_pwm.c \
          app/drivers/led/led_i2c.c \
          app/environment_cfg/led_cfg.c

OBJS    = $(SRCS:%.c=$(BUILDDIR)/%.o)

INCLUDES = -Iapp -Iapp/include -Iapp/drivers
# _POSIX_C_SOURCE=200809L 让 glibc 暴露 sleep / strerror / open / write 等
# POSIX 接口 (c99 默认隐藏). _DEFAULT_SOURCE 让 <linux/i2c-dev.h> 走得通
# (i2c_smbus_xxx 系列宏需要 BSD/SVID 段).
CFLAGS   = -std=c99 -Wall -Wextra -O2 \
           -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE \
           -ffunction-sections -fdata-sections \
           $(INCLUDES)
LDFLAGS  =
LIBS     = -lgpiod

.PHONY: all clean run check-syntax

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJS) | $(BUILDDIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
	@ls -la $@

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILDDIR)

run: $(BUILDDIR)/$(TARGET)
	sudo $(BUILDDIR)/$(TARGET)

# ---------------- syntax check (no libgpiod-dev needed) ----------------
# 用 syntax_stubs/ 下的占位 <gpiod.h> / <linux/i2c-dev.h> / <sys/ioctl.h>,
# 让 .c 文件能在没装 libgpiod-dev / 不在 Linux 上的环境过 -fsyntax-only.
# 仅做语法 + include 路径合法性检查, 不生成可执行文件.
check-syntax:
	@echo "[check-syntax] using syntax_stubs/ (no libgpiod-dev needed)"
	@for src in $(SRCS); do \
	  echo "  $$src"; \
	  $(CC) -std=c99 -Wall -Wextra \
	    -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE \
	    -Iapp -Iapp/include -Iapp/drivers -Isyntax_stubs \
	    -fsyntax-only $$src || exit 1; \
	done
	@echo "[check-syntax] OK"
