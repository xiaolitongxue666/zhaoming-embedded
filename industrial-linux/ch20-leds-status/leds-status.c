// SPDX-License-Identifier: GPL-2.0
/*
 * leds-status.c
 *
 * 配套 zhaoming-embedded ch20 (Linux 实战) + 附录 C
 * 教学用 platform driver·演示 Linux 内核里的 OOP-in-C 三件套：
 *
 *   1) 父类作首字段 (ch12 经典向上转型)
 *      struct status_led_data { struct led_classdev cdev; ... }
 *      led_classdev 在第一字段·led_classdev * 和 status_led_data * 同地址
 *
 *   2) container_of 反推 (ch13)
 *      .brightness_set 回调拿到 led_classdev *·用 container_of 反推到 status_led_data
 *      然后才能拿到 gpiod 等扩展字段
 *
 *   3) module_platform_driver -> __initcall 链路 (ch17/ch18)
 *      一行宏展开成 module_init / module_exit·链路嵌进 .initcall 段
 *
 * 硬件假设: Raspberry Pi 4B + GPIO17 接一只 LED
 * 设备绑定靠 device-tree overlay 里的 compatible = "status-led"
 *
 * 验证：
 *   sudo dtoverlay leds-status.dtbo   (overlay 编译后的 .dtbo)
 *   sudo insmod leds-status.ko
 *   ls /sys/class/leds/                         (应当看到 status-led)
 *   echo 1 | sudo tee /sys/class/leds/status-led/brightness
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>

/*
 * 派生类·led_classdev 必须在首字段
 * 这样 led_classdev * 和 status_led_data * 内存上是同一个地址
 * container_of 计算偏移时是 0·零开销反推
 */
struct status_led_data {
    struct led_classdev cdev;     /* 父类·首字段 (ch12 向上转型) */
    struct gpio_desc   *gpiod;    /* 派生扩展·私货 */
};

/*
 * brightness_set 是 led_classdev 的虚函数槽位
 * 内核回调时只给你 led_classdev *·没有任何上下文
 * 唯一拿到 status_led_data 的办法就是 container_of 反推
 */
static void status_led_brightness_set(struct led_classdev *led_cdev,
                                    enum led_brightness value)
{
    /* ch13 经典动作：从父类指针反推派生类指针 */
    struct status_led_data *led =
        container_of(led_cdev, struct status_led_data, cdev);

    gpiod_set_value(led->gpiod, value ? 1 : 0);
}

static int status_led_probe(struct platform_device *pdev)
{
    struct status_led_data *led;
    int ret;

    /* devm_ 系列·dev 销毁时自动 free·错误路径不用 goto unwind */
    led = devm_kzalloc(&pdev->dev, sizeof(*led), GFP_KERNEL);
    if (!led)
        return -ENOMEM;

    /* 从 dts 拿 gpios = <...>·devm 版自动 release */
    led->gpiod = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led->gpiod))
        return PTR_ERR(led->gpiod);

    /* 填父类字段·name 决定 /sys/class/leds/<name>/ */
    led->cdev.name           = "status-led";
    led->cdev.max_brightness = 1;
    led->cdev.brightness_set = status_led_brightness_set;

    /* 注册到 led 子系统·用 devm_ 版自动 unregister */
    ret = devm_led_classdev_register(&pdev->dev, &led->cdev);
    if (ret) {
        dev_err(&pdev->dev, "led_classdev_register failed: %d\n", ret);
        return ret;
    }

    dev_info(&pdev->dev,
             "status-led probed·container_of offset=%zu (父类是首字段所以 0)\n",
             offsetof(struct status_led_data, cdev));

    return 0;
}

/*
 * compatible 字符串和 dts 里的 status-led 对上·内核才能匹配
 * 这里用 generic 单标识符·参考 Linux 主线 gpio-leds 同款风格
 */
static const struct of_device_id status_led_of_match[] = {
    { .compatible = "status-led", },
    { /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, status_led_of_match);

static struct platform_driver status_led_driver = {
    .probe = status_led_probe,
    .driver = {
        .name           = "leds-status",
        .of_match_table = status_led_of_match,
    },
};

/*
 * ch17 / ch18 主角：module_platform_driver 一行宏带走
 * 展开后等价于：
 *   static int __init status_led_driver_init(void) { return platform_driver_register(&status_led_driver); }
 *   static void __exit status_led_driver_exit(void) { platform_driver_unregister(&status_led_driver); }
 *   module_init(status_led_driver_init);
 *   module_exit(status_led_driver_exit);
 *
 * module_init 把 status_led_driver_init 的指针扔进 .initcall 段
 * 启动时 do_initcalls() 按段顺序调一遍·这就是 initcall 链路
 */
module_platform_driver(status_led_driver);

MODULE_AUTHOR("zhaoming-embedded");
MODULE_DESCRIPTION("Educational LED platform driver for OOP-in-C book");
MODULE_LICENSE("GPL");
