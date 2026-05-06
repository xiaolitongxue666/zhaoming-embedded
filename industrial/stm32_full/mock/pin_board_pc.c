/**
  ******************************************************************************
  * @file    pin_board_pc.c
  * @brief   PC mock board-level PIN driver implementation.
  *
  * 见附录 B § B.6 + 第 15 章 "Platform 抽象到底".
  *
  * 在 PC 上用 printf 假装 GPIO 操作. 启动期通过 INIT_BOARD_EXPORT 自动注册
  * 到 platform_pin 框架, 跟 STM32 真机版 pin_board.c 接口一致, 应用层 /
  * driver 层一字不改. 这是"换硬件不改应用"在 PC mock 维度的具体演示:
  * 板级层换一份 .c 文件, 上层零改动.
  ******************************************************************************
  */

#include <stdio.h>
#include <string.h>

#include "platform/platform_pin.h"
#include "platform/platform_module_export.h"

#define PIN_NUM(port, no)        (((((port) & 0xFu) << 4) | ((no) & 0xFu)))
#define PIN_PORT(pin)            ((uint8_t)(((pin) >> 4) & 0xFu))
#define PIN_NO(pin)              ((uint8_t)((pin) & 0xFu))

static int32_t _pc_pin_get(const char *name)
{
    int32_t ret = PLATFORM_EINVAL;
    int hw_port_num;
    int hw_pin_num = 0;
    int i;
    int name_len;

    name_len = (int)strlen(name);

    if ((name_len < 4) || (name_len >= 6))
    {
        goto exit;
    }
    if ((name[0] != 'P') || (name[2] != '.'))
    {
        goto exit;
    }
    if ((name[1] < 'A') || (name[1] > 'Z'))
    {
        goto exit;
    }
    hw_port_num = (int)(name[1] - 'A');

    for (i = 3; i < name_len; i++)
    {
        hw_pin_num *= 10;
        hw_pin_num += name[i] - '0';
    }

    ret = PIN_NUM(hw_port_num, hw_pin_num);

exit:
    return ret;
}

static void _pc_pin_mode(int32_t pin, int32_t mode)
{
    char port = (char)('A' + PIN_PORT(pin));
    const char *m = (mode == PIN_MODE_OUTPUT) ? "OUTPUT" :
                    (mode == PIN_MODE_INPUT)  ? "INPUT"  : "OTHER";
    printf("    [PC] P%c.%u mode -> %s\n",
           port, (unsigned)PIN_NO(pin), m);
}

static void _pc_pin_write(int32_t pin, int32_t value)
{
    char port = (char)('A' + PIN_PORT(pin));
    printf("    [PC] P%c.%u <- %s\n",
           port, (unsigned)PIN_NO(pin),
           value ? "HIGH" : "LOW");
}

static int32_t _pc_pin_read(int32_t pin)
{
    (void)pin;
    return PIN_LOW;
}

static const platform_pin_ops_t _pc_pin_ops =
{
    .pin_mode       = _pc_pin_mode,
    .pin_write      = _pc_pin_write,
    .pin_read       = _pc_pin_read,
    .pin_attach_irq = NULL,
    .pin_detach_irq = NULL,
    .pin_irq_enable = NULL,
    .pin_get        = _pc_pin_get,
};

/**
  * @brief  PC mock 的 pin board init, 启动期由 INIT_BOARD_EXPORT 自动调用.
  */
static void _platform_hw_pin_init(void)
{
    platform_pin_register(&_pc_pin_ops);
}
INIT_BOARD_EXPORT(_platform_hw_pin_init);

/******************** END OF FILE ********************/
