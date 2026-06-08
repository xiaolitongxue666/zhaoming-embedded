/* SPDX-License-Identifier: MIT */
/*
 * main.c - ch15 完整 LED 框架演示 (风格 A)
 *
 * 整本书 ch01 - ch14 一路学的所有 OOP 武器, 在这里组装成一份完整工程.
 * 四层 (自上而下, 详见 pc/README.md):
 *
 *   ① 应用层 (app.h, app.c, main 业务部分)
 *      只认业务语义 + struct led_base * 句柄
 *   ② 板级 BSP (leds.h, led_board_init.c)
 *      这块板的接线: ERR=GPIO pin10, STAT=PWM ch1, NET=I2C 0x3C
 *      开机 init 时出现; 运行时 led_on 不经过本层
 *   ③ 设备驱动 (led_base / led_gpio / led_pwm / led_i2c)
 *      父类 + 子类 + ops 多态; 只调 platform_* API
 *   ④ Platform (platform_init.c, ../platform/platform_*.c, platform_*_pc.c)
 *      dispatcher + 后端; 栈底, 但 main 里最先 platform_init()
 *
 * 父类/子类/file 对应关系 (与上表交叉索引):
 *   父类层 (led_base.h, led_base.c)   : ops 表 + led_on/off/set_brightness
 *   子类层 (led_gpio / led_pwm / led_i2c) : 三种子类, container_of
 *   平台层 (platform_init.c)          : 注册 PC 后端 ops 进 dispatcher
 *   板级层 (leds.h, led_board_init.c) : 硬件参数 + 向上转型句柄
 *   应用层 (app.h, app.c)             : alarm_blink / status_indicate / power_on_test
 *
 * 启动顺序: platform_init() -> led_board_init() -> 应用代码. 真实工程
 * 一块板上还有 sensor / uart / motor 等等, 每个外设各自一份
 * xxx_board_init.c 在 platform_init() 之后调一次, 都是同款风格.
 *
 * 主线: grep app.c 拿不到任何硬件字样 (led_gpio / led_pwm / led_i2c /
 * gpio_write 全部 0 命中). 应用层只用 led_base * 句柄. 换硬件方案 ->
 * 改 led_board_init.c 三行, app.c 0 改动. 这就是 ch15 章末金句:
 *
 *   好的架构不是让你写更多代码, 是让你改更少代码.
 *
 * 见 ch15 § 15.5 应用层 + § 15.6 换硬件 diff.
 */

#include "app.h"
#include "leds.h"
#include "platform_init.h"
#include <stdio.h>

int main(void)
{
	int rc;

	printf("=========================================\n");
	printf("  ch15 - OOP complete framework demo\n");
	printf("=========================================\n");

	rc = platform_init();
	if (rc != 0) {
		printf("[main] platform_init failed, rc=%d, abort.\n", rc);
		return 1;
	}

	rc = led_board_init();
	if (rc != 0) {
		printf("[main] led_board_init failed, rc=%d, abort.\n", rc);
		return 1;
	}

	power_on_test();
	alarm_blink();
	status_indicate(0);   /* 正常 -> 状态灯 */
	status_indicate(1);   /* 故障 -> 报警灯 */

	printf("\n=========================================\n");
	printf("  app.c never named any hardware type\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
