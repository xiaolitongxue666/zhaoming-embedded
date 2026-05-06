# Makefile - ch05 HAL 库源码漫游 (PC 模拟版)
# C99 + Linux 内核风格

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
TARGET  = demo

SRCS    = main.c hal_gpio.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
