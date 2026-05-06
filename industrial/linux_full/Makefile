# SPDX-License-Identifier: MIT
#
# Makefile - Linux 用户态完整工程 (三模 build)
#
# PC 模拟模式 (验证抽象逻辑, 不依赖任何 GPIO):
#   make MOCK=1
#   ./build/firmware-pc
#
# libgpiod 模式 (默认, 现代 Linux 推荐):
#   sudo apt install libgpiod-dev
#   make
#   sudo ./build/firmware
#
# sysfs 模式 (退路, 老内核 / 没装 libgpiod 时用):
#   make BACKEND=sysfs
#   sudo ./build/firmware

TARGET   = firmware
BUILDDIR = build
BACKEND ?= libgpiod

CC       = gcc
LD       = gcc
ifeq ($(OS),Windows_NT)
EXT      = .exe
else
EXT      =
endif
CFLAGS_DEFS = -DMOCK_PC

# Linux 用户态用 GCC ctor 自动注册 INIT_xxx_EXPORT, 跟 PC mock 模式
# 同一套机制. 这里始终带 -DMOCK_PC 是为了让 platform_module_export.h
# 选 ctor 分支, 避开 linker section 路径 (Linux 用户态没必要 linker
# 段). 这是命名上跟 stm32_full 共享 #ifdef 开关的妥协.

ifeq ($(MOCK),1)
TARGET   = firmware-pc
SRCS_C   = mock/main_pc.c \
           mock/pin_board_pc.c \
           app/platform/platform_assert.c \
           app/platform/platform_pin.c \
           app/platform/platform_module_export.c \
           app/drivers/led/led_base.c \
           app/drivers/led/led_gpio.c \
           app/environment_cfg/led_cfg.c
LIBS     =
else
ifeq ($(BACKEND),sysfs)
PIN_SRC = app/platform/arch/board/pin_sysfs.c
LIBS    =
else
PIN_SRC = app/platform/arch/board/pin_libgpiod.c
LIBS    = -lgpiod
endif
SRCS_C  = src/main.c \
          $(PIN_SRC) \
          app/platform/platform_assert.c \
          app/platform/platform_pin.c \
          app/platform/platform_module_export.c \
          app/drivers/led/led_base.c \
          app/drivers/led/led_gpio.c \
          app/environment_cfg/led_cfg.c
endif

OBJS_C   = $(SRCS_C:%.c=$(BUILDDIR)/%.o)

INCLUDES = -Iapp -Iapp/platform -Iapp/drivers
CFLAGS   = -std=c99 -Wall -Wextra -O2 \
           -ffunction-sections -fdata-sections \
           $(INCLUDES) $(CFLAGS_DEFS)
LDFLAGS  =

.PHONY: all clean run

all: $(BUILDDIR)/$(TARGET)$(EXT)

$(BUILDDIR)/$(TARGET)$(EXT): $(OBJS_C) | $(BUILDDIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS_C) $(LIBS)
	@ls -la $@

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILDDIR)

run: $(BUILDDIR)/$(TARGET)$(EXT)
	$(BUILDDIR)/$(TARGET)$(EXT)
