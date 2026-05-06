# Makefile - ch07 写死的函数怎么换 (PC 模拟版)
#
# 本章只演示函数指针变量本身, main.c 里 gpio_on / pwm_on / i2c_on
# 都是 printf 占位, 不依赖 LED 结构或 platform 抽象层.

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
TARGET  = demo

SRCS    = main.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
