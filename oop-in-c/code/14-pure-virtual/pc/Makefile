# Makefile - ch14 pure virtual (PC)

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -I../../common
TARGET  = demo
SRCS    = main.c led_gpio.c led_pwm.c led_base.c led_board_init.c \
          sensor_base.c sensor_temp.c sensor_board_init.c \
          ../../common/platform_pc.c

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe
