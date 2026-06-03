# Makefile - ch16 Linux-style GPIO Subsystem (PC 模拟版)
#
# 构建一套简化的 GPIO 子系统教学演示程序。在 PC 上用 printf
# 模拟寄存器操作，无需硬件即可运行。
#
# 主要构建目标:
#   all    - 编译生成 demo.exe
#   run    - 编译并运行 demo
#   clean  - 清理构建产物

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99
TARGET  = demo
SRCS    = main.c gpiolib.c gpio_vendor_a.c gpio_vendor_b.c leds_gpio.c

.PHONY: all clean run

# 默认目标：编译可执行文件
all: $(TARGET)

# 链接所有源文件为单个可执行文件
# 注意：不使用 -o 分离编译，简化教学示例
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# 编译并运行
run: $(TARGET)
	./$(TARGET)

# 清理构建产物（兼容 Windows .exe 后缀）
clean:
	rm -f $(TARGET) $(TARGET).exe
