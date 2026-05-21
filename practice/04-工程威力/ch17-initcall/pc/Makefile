# Makefile - 个人练习 (PC)
# 手写后把 .c 文件追加到 SRCS

CC      = gcc
COMMON  = ../../../../oop-in-c/code/common
CFLAGS  = -Wall -Wextra -std=c99 -I$(COMMON)
TARGET  = demo
SRCS    = main.c $(COMMON)/platform_pc.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
