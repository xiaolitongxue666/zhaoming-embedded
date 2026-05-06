# Makefile - ch08 函数指针当参数 (PC 模拟版)
#
# 本章只演示 test_led 把函数指针当参数, led.c 里 gpio_on / pwm_on /
# i2c_on 都是 printf 占位, 不依赖任何 LED 结构或平台抽象层.

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
TARGET  = demo

SRCS    = main.c led.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
