# Makefile - ch17 initcall (PC)
#
# 注意：__start_my_initcall / __stop_my_initcall 这两个符号是 GNU ld 在
# ELF 平台自动生成的（任何独立 section 都会有）。MinGW 的 PE 链接器同样支持。
#
# 我们额外在 drv_dummy.c 里准备一份 fallback，确保即使链接器没有给你
# 自动生成边界符号，我们也能跑通。

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
TARGET  = demo
SRCS    = main.c initcall.c drv_led.c drv_uart.c drv_i2c.c drv_spi.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
