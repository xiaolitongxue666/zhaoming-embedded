/**
  ******************************************************************************
  * @file    pin_sysfs.c
  * @brief   Linux sysfs-based PIN driver (legacy fallback).
  *
  * 基于 /sys/class/gpio 文件接口的 pin 子类. 比 libgpiod 老, 但在没有
  * libgpiod 头文件、或 root 受限场景仍然可用. 启动期 INIT_BOARD_EXPORT
  * 自动注册到 platform_pin 框架.
  *
  * 内核 4.8 后官方推荐 libgpiod. sysfs 仍然能用, 但属于 deprecated 接口,
  * 新工程应该选 pin_libgpiod.c.
  *
  * 字符串 pin 名约定跟 libgpiod 一致: "GPIO17", "GPIO27"... (BCM 编号).
  ******************************************************************************
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "platform/platform_pin.h"
#include "platform/platform_module_export.h"

static int _sysfs_export(int pin)
{
    int fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    char buf[8];
    int n = snprintf(buf, sizeof(buf), "%d", pin);
    write(fd, buf, n);
    close(fd);
    return 0;
}

static int _sysfs_set_direction(int pin, const char *dir)
{
    char path[64];
    snprintf(path, sizeof(path),
             "/sys/class/gpio/gpio%d/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        return -1;
    }
    write(fd, dir, strlen(dir));
    close(fd);
    return 0;
}

/* ------ ops 实现 -------------------------------------------------------- */

static int32_t _sysfs_pin_get(const char *name)
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

static void _sysfs_pin_mode(int32_t pin, int32_t mode)
{
    _sysfs_export((int)pin);
    _sysfs_set_direction((int)pin,
                         (mode == PIN_MODE_OUTPUT) ? "out" : "in");
}

static void _sysfs_pin_write(int32_t pin, int32_t value)
{
    char path[64];
    snprintf(path, sizeof(path),
             "/sys/class/gpio/gpio%d/value", (int)pin);
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        return;
    }
    write(fd, value ? "1" : "0", 1);
    close(fd);
}

static int32_t _sysfs_pin_read(int32_t pin)
{
    char path[64];
    char buf[2] = {0};
    snprintf(path, sizeof(path),
             "/sys/class/gpio/gpio%d/value", (int)pin);
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return PIN_LOW;
    }
    read(fd, buf, 1);
    close(fd);
    return (buf[0] == '1') ? PIN_HIGH : PIN_LOW;
}

static const platform_pin_ops_t _sysfs_pin_ops =
{
    .pin_mode       = _sysfs_pin_mode,
    .pin_write      = _sysfs_pin_write,
    .pin_read       = _sysfs_pin_read,
    .pin_attach_irq = NULL,
    .pin_detach_irq = NULL,
    .pin_irq_enable = NULL,
    .pin_get        = _sysfs_pin_get,
};

static void _platform_hw_pin_init(void)
{
    platform_pin_register(&_sysfs_pin_ops);
}
INIT_BOARD_EXPORT(_platform_hw_pin_init);

/******************** END OF FILE ********************/
