/* SPDX-License-Identifier: Apache-2.0
 *
 * Demo 1: 4 颗 LED 同 ops 表跑 (ch19/19.7 向上转型)
 *
 * 教学要点：
 *   - 4 颗 LED 共用一个 led_classdev / leds 节点
 *   - led_on / led_off 第一参数都是 const struct device *
 *   - 子类在 dts 里 (gpios = <...>)·应用层只见父类 ops
 *   - 这就是经典向上转型：应用层拿到 base 指针·不知道也不关心子类是 GPIO LED 还是 PWM LED
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/led.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_NODELABEL(leds)

int main(void)
{
    const struct device *led_dev = DEVICE_DT_GET(LED_NODE);

    if (!device_is_ready(led_dev)) {
        printk("LED device not ready\n");
        return -1;
    }

    printk("[demo1] 4 LED 同 ops 表跑·按编号 0/1/2/3 顺次点亮\n");

    while (1) {
        for (int i = 0; i < 4; i++) {
            led_on(led_dev, i);
            k_msleep(200);
            led_off(led_dev, i);
        }
    }

    return 0;
}
