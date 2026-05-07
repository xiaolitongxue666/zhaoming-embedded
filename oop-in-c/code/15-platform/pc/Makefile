# Makefile - ch15 OOP complete framework (PC, 风格 A)

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -I../../common
TARGET  = demo

SRCS    = main.c app.c led.c led_base.c board_init.c ../../common/platform_pc.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
