# Makefile - ch15 platform climax (PC)

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -I../../common
TARGET  = demo
SRCS    = main.c app.c led.c board_init.c \
          platform_dispatch.c \
          platform_ops_pc.c platform_ops_stm32_mock.c platform_ops_linux_mock.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
