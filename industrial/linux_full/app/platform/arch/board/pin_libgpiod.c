/**
  ******************************************************************************
  * @file    pin_libgpiod.c
  * @brief   Linux libgpiod-based PIN driver (modern, recommended).
  *
  * 基于 libgpiod 1.x API 的 pin 子类. 启动期 INIT_BOARD_EXPORT 自动注册到
  * platform_pin 框架. 上层 driver 调 platform_pin_xxx 时通过框架内部 ops
  * 指针 dispatch 到这一份.
  *
  * 树莓派 4B 默认 GPIO 控制器是 /dev/gpiochip0. 其他平台 (香橙派 / 飞腾 /
  * RK3588) 改 GPIO_CHIP_DEV 即可, driver / app 一字不动.
  *
  * 字符串 pin 名约定: "GPIO17", "GPIO27", "GPIO22"... (BCM 编号).
  *
  * 安装 libgpiod:
  *   Debian/Ubuntu: sudo apt install libgpiod-dev
  *   Fedora:        sudo dnf install libgpiod-devel
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <gpiod.h>

#include "platform/platform_pin.h"
#include "platform/platform_module_export.h"

#define GPIO_CHIP_DEV          "gpiochip0"   /* 树莓派默认控制器 */
#define MAX_LINES              64

struct line_slot {
    int                   pin;       /* BCM 编号 */
    int                   mode;      /* PIN_MODE_OUTPUT / INPUT */
    struct gpiod_line    *line;      /* libgpiod 句柄 */
};

static struct gpiod_chip *_g_chip;
static struct line_slot   _g_slots[MAX_LINES];
static int                _g_slot_count;

/* ------ private helpers ------------------------------------------------ */

static struct line_slot *_find_slot(int pin)
{
    for (int i = 0; i < _g_slot_count; i++) {
        if (_g_slots[i].pin == pin)
            return &_g_slots[i];
    }
    return NULL;
}

static struct line_slot *_acquire_slot(int pin, int mode)
{
    struct line_slot *slot = _find_slot(pin);

    if (slot != NULL) {
        return slot;
    }

    if (_g_slot_count >= MAX_LINES) {
        fprintf(stderr, "[libgpiod] slot table full\n");
        return NULL;
    }

    if (_g_chip == NULL) {
        _g_chip = gpiod_chip_open_by_name(GPIO_CHIP_DEV);
        if (_g_chip == NULL) {
            fprintf(stderr, "[libgpiod] open %s failed\n", GPIO_CHIP_DEV);
            return NULL;
        }
    }

    struct gpiod_line *line = gpiod_chip_get_line(_g_chip, pin);
    if (line == NULL) {
        fprintf(stderr, "[libgpiod] get line %d failed\n", pin);
        return NULL;
    }

    int rc;
    if (mode == PIN_MODE_OUTPUT) {
        rc = gpiod_line_request_output(line, "zhaoming-embedded", 0);
    } else {
        rc = gpiod_line_request_input(line, "zhaoming-embedded");
    }
    if (rc < 0) {
        fprintf(stderr, "[libgpiod] request line %d failed\n", pin);
        return NULL;
    }

    slot = &_g_slots[_g_slot_count++];
    slot->pin  = pin;
    slot->mode = mode;
    slot->line = line;
    return slot;
}

/* ------ ops 实现 -------------------------------------------------------- */

/**
  * @brief  Resolve "GPIO17", "GPIO27" 这类字面字符串到 BCM 编号.
  */
static int32_t _libgpiod_pin_get(const char *name)
{
    if ((name[0] != 'G') || (name[1] != 'P') || (name[2] != 'I') ||
        (name[3] != 'O')) {
        return PLATFORM_EINVAL;
    }
    int n = atoi(name + 4);
    if ((n < 0) || (n > 53)) {
        return PLATFORM_EINVAL;
    }
    return n;
}

static void _libgpiod_pin_mode(int32_t pin, int32_t mode)
{
    (void)_acquire_slot((int)pin, (int)mode);
}

static void _libgpiod_pin_write(int32_t pin, int32_t value)
{
    struct line_slot *slot = _find_slot((int)pin);
    if (slot == NULL) {
        return;
    }
    gpiod_line_set_value(slot->line, value ? 1 : 0);
}

static int32_t _libgpiod_pin_read(int32_t pin)
{
    struct line_slot *slot = _find_slot((int)pin);
    if (slot == NULL) {
        return PIN_LOW;
    }
    int v = gpiod_line_get_value(slot->line);
    return (v > 0) ? PIN_HIGH : PIN_LOW;
}

static const platform_pin_ops_t _libgpiod_pin_ops =
{
    .pin_mode       = _libgpiod_pin_mode,
    .pin_write      = _libgpiod_pin_write,
    .pin_read       = _libgpiod_pin_read,
    .pin_attach_irq = NULL,    /* 教学版不实现 IRQ, 工业版补 gpiod 事件机制 */
    .pin_detach_irq = NULL,
    .pin_irq_enable = NULL,
    .pin_get        = _libgpiod_pin_get,
};

/**
  * @brief  Pin board init, 启动期由 INIT_BOARD_EXPORT 自动调用.
  */
static void _platform_hw_pin_init(void)
{
    platform_pin_register(&_libgpiod_pin_ops);
}
INIT_BOARD_EXPORT(_platform_hw_pin_init);

/******************** END OF FILE ********************/
