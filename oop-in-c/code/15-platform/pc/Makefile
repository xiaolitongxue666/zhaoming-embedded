# Makefile - ch15 OOP complete framework (PC)

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -I../../common
TARGET  = demo
SRCS    = main.c app.c led.c board_init.c ../../common/platform_pc.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
