/* SPDX-License-Identifier: MIT */
/*
 * led_userspace.c - ch16 linux-driver/userspace 应用层最小例.
 *
 * ch16 整章讲 Linux 内核 GPIO 子系统的骨架: struct gpio_chip + ops 表 +
 * 多态 dispatch (16.5 节 gc->set). pc/ 山寨了一份教学版, 这里给应用层
 * 视角: 一行 gpiod_line_set_value 直接跨进程 syscall 进内核, 内核里跑
 * 的就是 ch16 整章讲的那一套 dispatch (真身, drivers/gpio/gpiolib.c).
 *
 * 关键观察: 应用层无需自抽 platform 层. 内核 driver model 已经把硬件
 * 抽象做完, 这里直接 libgpiod 就是 ch16 § 16.13 / § 16.14 讲的"内核
 * 已做完别再抽"的代码兑现.
 *
 * libgpiod 1.x API (Debian 12 / Ubuntu 22.04 默认装的版本). 2.x API 改
 * 成 request builder 风格, 移植改 init 段两行调用即可.
 */

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* 真机假设: 树莓派 4B 默认 device tree, BCM GPIO17. 换板改这两行. */
#define CHIP_NAME    "gpiochip0"
#define LINE_OFFSET  17

int main(void)
{
	struct gpiod_chip *chip;
	struct gpiod_line *line;
	int                rc;

	printf("=========================================\n");
	printf("  ch16 - userspace libgpiod (no platform layer)\n");
	printf("=========================================\n");

	chip = gpiod_chip_open_by_name(CHIP_NAME);
	if (chip == NULL) {
		fprintf(stderr, "open %s failed (root or gpio group needed)\n",
		        CHIP_NAME);
		return -1;
	}

	line = gpiod_chip_get_line(chip, LINE_OFFSET);
	if (line == NULL) {
		fprintf(stderr, "get_line(%u) failed\n", LINE_OFFSET);
		gpiod_chip_close(chip);
		return -1;
	}

	/* 申请 line 为输出, 上电先关灯 (active-high 假设). */
	rc = gpiod_line_request_output(line, "led", 0);
	if (rc < 0) {
		fprintf(stderr, "request_output failed\n");
		gpiod_chip_close(chip);
		return -1;
	}

	printf("\n--- toggle line %u on chip %s ---\n",
	       LINE_OFFSET, CHIP_NAME);

	/*
	 * 这一行进内核之后走的就是 ch16 § 16.5 讲的:
	 *   gpiod_set_value -> gpiod_set_raw_value_commit ->
	 *   gc->set(gc, hwgpio, value);
	 * gc->set 多态 dispatch 到 BCM2711 SoC 的 gpio-bcm2835.c.
	 * 应用层一行, 内核里跑完整的 ops 表 dispatch.
	 */
	rc = gpiod_line_set_value(line, 1);
	printf("set 1 -> rc=%d (LED on)\n", rc);
	sleep(1);

	rc = gpiod_line_set_value(line, 0);
	printf("set 0 -> rc=%d (LED off)\n", rc);
	sleep(1);

	rc = gpiod_line_set_value(line, 1);
	printf("set 1 -> rc=%d (LED on)\n", rc);
	sleep(1);

	rc = gpiod_line_set_value(line, 0);
	printf("set 0 -> rc=%d (LED off)\n", rc);

	printf("\n>>> userspace done, kernel did the dispatch <<<\n");

	gpiod_line_release(line);
	gpiod_chip_close(chip);
	return 0;
}
