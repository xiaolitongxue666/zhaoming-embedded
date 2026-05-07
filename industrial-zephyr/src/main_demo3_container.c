/* SPDX-License-Identifier: Apache-2.0
 *
 * Demo 3: CONTAINER_OF 抓回调
 *
 * 教学要点：
 *   - Zephyr 的回调签名固定·只给一个基础类型指针 (k_timer * / gpio_callback *)
 *   - 要拿到自定义上下文·必须 CONTAINER_OF 反推
 *   - ch13 讲过的"从字段指针反推容器指针"·offsetof 算偏移再减
 *
 * stm32f4_disco 板载没合适用户按钮·这里改用定时器周期回调
 * 演示原理一样：回调函数只拿到 k_timer*·靠 CONTAINER_OF 反推到 app_timer_ctx
 *
 * gdb 验证：
 *   p &ctx              // 自定义结构起始地址
 *   p &ctx.timer        // timer 字段地址
 *   p &ctx.timer - &ctx // 应该等于 offsetof(app_timer_ctx, timer)
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/led.h>
#include <zephyr/sys/printk.h>
#include <stddef.h>

#define LED_NODE DT_NODELABEL(leds)

/* 自定义上下文·timer 是父类·业务字段 led_dev / tick_count 是派生扩展 */
struct app_timer_ctx {
    struct k_timer        timer;     /* 父类·必须放在能被 CONTAINER_OF 反推的位置 */
    const struct device  *led_dev;
    uint32_t              tick_count;
    uint8_t               which_led;
};

static void on_timer_expired(struct k_timer *t)
{
    /* 关键一行：从 t 反推回 ctx·拿到自定义上下文 */
    struct app_timer_ctx *ctx =
        CONTAINER_OF(t, struct app_timer_ctx, timer);

    ctx->tick_count++;

    /* 偶数 tick 点亮·奇数 tick 熄灭·演示拿到上下文之后做业务 */
    if (ctx->tick_count & 1U) {
        led_on(ctx->led_dev, ctx->which_led);
    } else {
        led_off(ctx->led_dev, ctx->which_led);
    }

    if (ctx->tick_count <= 6) {
        printk("[demo3] tick=%u led=%u  ctx=%p timer=%p offset=%u\n",
               ctx->tick_count, ctx->which_led,
               (void *)ctx, (void *)t,
               (unsigned)offsetof(struct app_timer_ctx, timer));
    }
}

static struct app_timer_ctx g_ctx;

int main(void)
{
    g_ctx.led_dev    = DEVICE_DT_GET(LED_NODE);
    g_ctx.tick_count = 0;
    g_ctx.which_led  = 0;

    if (!device_is_ready(g_ctx.led_dev)) {
        printk("LED device not ready\n");
        return -1;
    }

    printk("[demo3] CONTAINER_OF 抓回调演示开始\n");
    printk("[demo3] offsetof(ctx, timer) = %u\n",
           (unsigned)offsetof(struct app_timer_ctx, timer));

    k_timer_init(&g_ctx.timer, on_timer_expired, NULL);
    k_timer_start(&g_ctx.timer, K_MSEC(300), K_MSEC(300));

    /* main 退出·timer 在系统线程里继续跑 */
    return 0;
}
