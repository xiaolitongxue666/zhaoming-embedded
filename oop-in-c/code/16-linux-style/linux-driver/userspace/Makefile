# SPDX-License-Identifier: MIT
#
# Makefile - ch16 linux-driver/userspace
#
# 依赖 libgpiod-dev:
#   sudo apt install libgpiod-dev      # Debian / Ubuntu / 树莓派 OS
#   sudo dnf install libgpiod-devel    # Fedora / RHEL
#
# 编译:
#   make
#
# 真机跑 (需要 GPIO 权限, 一般要 sudo 或加 gpio group):
#   sudo ./demo
#
# 仅 syntax check (Windows / 没 libgpiod-dev 的环境):
#   make check-syntax
# 用 ../../../industrial/linux_full/syntax_stubs/ 占位头, 跑 -fsyntax-only.

CC      = gcc
TARGET  = demo

SRCS    = led_userspace.c

CFLAGS  = -std=c99 -Wall -Wextra -O2 \
          -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE
LIBS    = -lgpiod

STUBS_DIR = ../../../../../industrial/linux_full/syntax_stubs

.PHONY: all clean check-syntax run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LIBS)

check-syntax:
	@echo "[check-syntax] using $(STUBS_DIR)"
	@for src in $(SRCS); do \
	  echo "  $$src"; \
	  $(CC) $(CFLAGS) -I$(STUBS_DIR) -fsyntax-only $$src || exit 1; \
	done
	@echo "[check-syntax] OK"

run: $(TARGET)
	sudo ./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
