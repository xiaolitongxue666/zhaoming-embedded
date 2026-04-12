# EP08 你用C手搓了一个class
# 兆鸣嵌入式 - C语言·一个LED讲透面向对象

CC      = gcc
CFLAGS  = -Wall -Wextra -I../common
TARGET  = demo

SRCS    = main.c led.c motor.c ../common/platform_pc.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
