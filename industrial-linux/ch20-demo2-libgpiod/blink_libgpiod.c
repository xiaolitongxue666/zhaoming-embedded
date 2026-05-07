// SPDX-License-Identifier: GPL-2.0
/*
 * blink_libgpiod.c
 *
 * 配套 zhaoming-embedded ch20.9 sysfs vs libgpiod 同硬件两接口
 *
 * 用户态 libgpiod 直接拍 GPIO·和 leds-status.ko 互斥：
 *   - 内核驱动持有 GPIO17 时·这里 request 会失败 (-EBUSY)
 *   - rmmod leds-status + dtoverlay -r 之后·这里就能跑
 *
 * 这就是书里讲的"两条路·要么内核管要么用户态管·不能同时"
 *
 * 编译：
 *   make
 * 跑：
 *   sudo ./blink_libgpiod
 */

#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int i;

    (void)argc;
    (void)argv;

    /* Raspberry Pi 4B 的主 GPIO 控制器是 gpiochip0 */
    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    line = gpiod_chip_get_line(chip, 17);
    if (!line) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(chip);
        return 1;
    }

    if (gpiod_line_request_output(line, "blink_demo", 0) < 0) {
        perror("gpiod_line_request_output (是否被内核驱动占用)");
        gpiod_chip_close(chip);
        return 1;
    }

    printf("[demo2] GPIO17 闪烁 20 次·每次 200ms\n");

    for (i = 0; i < 20; i++) {
        gpiod_line_set_value(line, i & 1);
        usleep(200000);
    }

    gpiod_line_release(line);
    gpiod_chip_close(chip);
    return 0;
}
