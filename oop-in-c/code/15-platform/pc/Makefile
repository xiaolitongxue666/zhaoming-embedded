# Makefile - ch15 OOP complete framework (PC, 风格 A)
#
# CFLAGS 里 -I.. 让源码用 #include "platform/platform_i2c.h" 风格的路径
# (和 drivers/led/, arch/<mcu>/pin_board.c 同款 include 路径), 这样 PC
# 端和真机端的 #include 形式一致, 子类源码字节级不动.

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -I../../common -I..
TARGET  = demo

SRCS    = main.c app.c \
          led_base.c led_gpio.c led_pwm.c led_i2c.c \
          board_init.c \
          platform_pwm_pc.c platform_i2c_pc.c \
          ../platform/platform_pwm.c ../platform/platform_i2c.c \
          ../../common/platform_pc.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
