# SPDX-License-Identifier: MIT
#
# Makefile - STM32F407 industrial-grade scaffold (双模 build)
#
# PC mock build (验证抽象逻辑, 不依赖 STM32 HAL):
#   make MOCK=1
#   ./build/firmware-pc.exe       (Windows)
#   ./build/firmware-pc           (Linux/macOS)
#
# 真机 build (依赖 arm-none-eabi-gcc + STM32 CubeMX HAL):
#   1. 装 arm-none-eabi-gcc 工具链
#   2. 用 STM32CubeMX 生成 STM32F407VGTx 工程, 把 Drivers/ 拷到 vendor/
#      Drivers/STM32F4xx_HAL_Driver/  -> vendor/STM32F4xx_HAL_Driver/
#      Drivers/CMSIS/                 -> vendor/CMSIS/
#   3. 把 CubeMX 生成的 SystemClock_Config + MX_GPIO_Init 抄进 Core/Src/
#      把 startup_stm32f407xx.s 替换为 CubeMX 生成版 (替换骨架)
#   4. make
#   5. make flash    (st-flash 烧到 0x08000000)

TARGET   = firmware
BUILDDIR = build

# ============================================================================
# 工具链 + 工程源
# ============================================================================
ifeq ($(MOCK),1)

# ---------------- PC mock ----------------
CC       = gcc
LD       = gcc
SIZE     = size
ifeq ($(OS),Windows_NT)
EXT      = .exe
else
EXT      =
endif
TARGET   = firmware-pc

CFLAGS_ARCH  =
LDFLAGS_ARCH =
CFLAGS_DEFS  = -DMOCK_PC

SRCS_C = mock/main_pc.c \
         mock/pin_board_pc.c \
         mock/pwm_board_pc.c \
         mock/i2c_board_pc.c \
         app/platform/platform_assert.c \
         app/platform/platform_pin.c \
         app/platform/platform_pwm.c \
         app/platform/platform_i2c.c \
         app/platform/platform_module_export.c \
         app/drivers/led/led_base.c \
         app/drivers/led/led_gpio.c \
         app/drivers/led/led_pwm.c \
         app/drivers/led/led_i2c.c \
         app/environment_cfg/led_cfg.c
SRCS_S =

INCLUDES = -Iapp -Iapp/platform -Iapp/drivers

else

# ---------------- 真机: STM32F407 ----------------
CC       = arm-none-eabi-gcc
LD       = arm-none-eabi-gcc
OBJCOPY  = arm-none-eabi-objcopy
SIZE     = arm-none-eabi-size
EXT      = .elf

CFLAGS_ARCH  = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
LDFLAGS_ARCH = $(CFLAGS_ARCH) -T linker_stm32f407.ld -nostartfiles \
               -Wl,--gc-sections -Wl,-Map=$(BUILDDIR)/$(TARGET).map
CFLAGS_DEFS  = -DSTM32F407xx -DUSE_HAL_DRIVER

SRCS_C = Core/Src/main.c \
         app/platform/platform_assert.c \
         app/platform/platform_pin.c \
         app/platform/platform_pwm.c \
         app/platform/platform_i2c.c \
         app/platform/platform_module_export.c \
         app/platform/arch/board/pin_board.c \
         app/platform/arch/board/pwm_board.c \
         app/platform/arch/board/i2c_board.c \
         app/drivers/led/led_base.c \
         app/drivers/led/led_gpio.c \
         app/drivers/led/led_pwm.c \
         app/drivers/led/led_i2c.c \
         app/environment_cfg/led_cfg.c
SRCS_S = startup_stm32f407xx.s

HAL_PATH   ?= vendor/STM32F4xx_HAL_Driver
CMSIS_PATH ?= vendor/CMSIS

INCLUDES = -Iapp -Iapp/platform -Iapp/drivers \
           -ICore/Inc \
           -I$(HAL_PATH)/Inc -I$(CMSIS_PATH)/Include \
           -I$(CMSIS_PATH)/Device/ST/STM32F4xx/Include

endif

OBJS_C = $(SRCS_C:%.c=$(BUILDDIR)/%.o)
OBJS_S = $(SRCS_S:%.s=$(BUILDDIR)/%.o)
OBJS   = $(OBJS_C) $(OBJS_S)

CFLAGS  = -std=c99 -Wall -Wextra $(CFLAGS_ARCH) -Os \
          -ffunction-sections -fdata-sections \
          $(INCLUDES) $(CFLAGS_DEFS)
LDFLAGS = $(LDFLAGS_ARCH)

# ============================================================================
.PHONY: all clean flash size run

all: $(BUILDDIR)/$(TARGET)$(EXT)

$(BUILDDIR)/$(TARGET)$(EXT): $(OBJS) | $(BUILDDIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
ifneq ($(MOCK),1)
	$(OBJCOPY) -O binary $@ $(BUILDDIR)/$(TARGET).bin
	$(OBJCOPY) -O ihex   $@ $(BUILDDIR)/$(TARGET).hex
endif
	$(SIZE) $@

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR)/%.o: %.s | $(BUILDDIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILDDIR):
	@mkdir -p $@

clean:
	rm -rf $(BUILDDIR)

flash: $(BUILDDIR)/$(TARGET).bin
	st-flash write $< 0x08000000

run: $(BUILDDIR)/$(TARGET)$(EXT)
	$(BUILDDIR)/$(TARGET)$(EXT)
