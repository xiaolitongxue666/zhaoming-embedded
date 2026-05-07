/* SPDX-License-Identifier: Apache-2.0
 *
 * Demo 4: 可空 ops 演示·调 led_blink 看 -ENOSYS (ch19/19.10)
 *
 * 教学要点：
 *   - led_driver_api 的 ops 表里 blink 是可空字段
 *   - led_gpio 子类只实现了 set_brightness·blink 留 NULL
 *   - led 子系统检测到 NULL·直接返回 -ENOSYS (errno 88)
 *   - 这就是 ch16 讲的"NULL 检查 + ENOSYS"模式：父类不强迫子类实现所有动作
 *
 * 期望输出：
 *   led_blink returned -88           // -ENOSYS
 *   led_set_brightness returned 0    // 正常
 *
 * 对比 C++ pure virtual：那种是编译期强制·这种是运行期可选·内核风味更宽容
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/led.h>
#include <zephyr/sys/printk.h>
#include <errno.h>

#define LED_NODE DT_NODELABEL(leds)

int main(void)
{
    const struct device *led_dev = DEVICE_DT_GET(LED_NODE);

    if (!device_is_ready(led_dev)) {
        printk("LED device not ready\n");
        return -1;
    }

    printk("[demo4] 可空 ops 演示·调没实现的动作看返回 -ENOSYS\n");

    /* 1) blink·led_gpio 子类没实现·应当返回 -ENOSYS (-88) */
    int ret = led_blink(led_dev, 0, 500, 500);
    printk("[demo4] led_blink returned %d  (期望 -%d 即 -ENOSYS)\n", ret, ENOSYS);

    /* 2) set_brightness·子类有实现·应当返回 0 */
    ret = led_set_brightness(led_dev, 0, 100);
    printk("[demo4] led_set_brightness returned %d  (期望 0)\n", ret);

    /* 3) on/off·子类有实现·应当返回 0 */
    ret = led_on(led_dev, 0);
    printk("[demo4] led_on returned %d  (期望 0)\n", ret);

    return 0;
}
