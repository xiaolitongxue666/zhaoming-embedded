# EP10 HAL库几千个函数，就一个套路
# 兆鸣嵌入式 - C语言·一个LED讲透面向对象

CC      = gcc
CFLAGS  = -Wall -Wextra
TARGET  = demo

SRCS    = main.c hal_gpio.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)
