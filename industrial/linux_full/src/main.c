/**
  ******************************************************************************
  * @file    main.c
  * @brief   Linux 用户态主程序 (流水灯).
  *
  * 见附录 C § C.3 + 第 16 章 "Linux 不难" + 第 15 章 "Platform 抽象到底".
  *
  * 启动流程:
  *   1. ELF loader 跑完所有 __attribute__((constructor(N))) 注册项
  *      (platform_pin_register / led_cfg / 其他 INIT_xxx_EXPORT 全部在
  *      main 之前已经跑过)
  *   2. main 调 platform_module_export_exec() 是 nop (兼容形式跟 stm32_full 一致)
  *   3. 进入应用主循环
  *
  * 应用层调用 led_on / led_off 一字不知 platform 是 libgpiod 还是 sysfs.
  * 切换 platform 子类只换链接的 .c 文件 (Makefile 的 BACKEND 变量).
  ******************************************************************************
  */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <signal.h>

#include "led/led_base.h"
#include "platform/platform_module_export.h"

extern led_base_t *led_green;
extern led_base_t *led_orange;
extern led_base_t *led_red;
extern led_base_t *led_blue;

static volatile int _g_running = 1;

static void _on_sigint(int sig)
{
    (void)sig;
    _g_running = 0;
}

int main(void)
{
    led_base_t *all[4];
    uint32_t cur = 0;

    signal(SIGINT,  _on_sigint);
    signal(SIGTERM, _on_sigint);

    printf("=========================================\n");
    printf("  linux_full: 4 LED running light\n");
    printf("=========================================\n");

    platform_module_export_exec();   /* 真机这里 nop, ctor 已跑完 */

    if ((NULL == led_green)  || (NULL == led_orange) ||
        (NULL == led_red)    || (NULL == led_blue)) {
        fprintf(stderr, "[main] LED handles not ready, exit.\n");
        return -1;
    }

    all[0] = led_green;
    all[1] = led_orange;
    all[2] = led_red;
    all[3] = led_blue;

    while (_g_running) {
        for (uint32_t i = 0; i < 4; i++) {
            led_off(all[i]);
        }
        led_on(all[cur]);
        cur = (cur + 1) % 4;
        sleep(1);
    }

    printf("\n[main] caught signal, cleaning up.\n");
    for (uint32_t i = 0; i < 4; i++) {
        led_off(all[i]);
    }
    return 0;
}

/******************** END OF FILE ********************/
