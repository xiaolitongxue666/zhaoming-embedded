/* SPDX-License-Identifier: Apache-2.0
 *
 * Demo 2: overlay 改 label·应用层零改动 (ch19/19.8 dts 解耦)
 *
 * 教学要点：
 *   - boards/stm32f4_disco.overlay 改了 green_led_4 的 label
 *   - 这里通过 DT_PROP 在编译期取出 label 字符串·printk 出来证明 overlay 生效
 *   - 没动 .c 任何一行·只是 build 系统把 overlay 合进了 zephyr.dts
 *   - 这就是 dts 解耦的威力：硬件描述外移·应用层稳如老狗
 *
 * 期望输出：
 *   led0 label: Demo Board LD4 (overlay)
 *   (如果不带 overlay 重新 build·会打印 dts 默认的 label·两次对比说明 overlay 生效)
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/led.h>
#include <zephyr/sys/printk.h>

#define LED_NODE DT_NODELABEL(leds)

/* led0 alias 在 stm32f4_disco.dts 里指向 green_led_4 */
#define LED0_NODE DT_ALIAS(led0)

int main(void)
{
    const struct device *led_dev = DEVICE_DT_GET(LED_NODE);

    if (!device_is_ready(led_dev)) {
        printk("LED device not ready\n");
        return -1;
    }

    /* 关键一行：从 dts 拿 label·overlay 改了就这里变 */
    printk("[demo2] led0 label: %s\n", DT_PROP(LED0_NODE, label));
    printk("[demo2] 应用层一行没改·只是 boards/stm32f4_disco.overlay 改了 label\n");

    /* 和 demo1 一样跑 4 LED·证明业务逻辑稳定 */
    while (1) {
        for (int i = 0; i < 4; i++) {
            led_on(led_dev, i);
            k_msleep(200);
            led_off(led_dev, i);
        }
    }

    return 0;
}
