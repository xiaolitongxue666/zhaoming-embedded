/* SPDX-License-Identifier: MIT */
/*
 * main.c - 一颗 LED 演化路径全景
 *
 * 这个文件不教新东西。它把 ch01 → ch17 一颗 LED 走过的演化路径
 * 在屏幕上 replay 一遍，让读者看见自己走过的路。
 *
 * 每一段对应书里的一个章节。每一段都能跑（虽然有些段刻意保留了
 * "原始痛点"，比如 stage 1 的三份独立函数）。
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* ======================== Stage 1: ch01 ========================
 * 三个 LED 三份代码 - 复制粘贴
 */

static void s1_red_on(void)
{
	printf("    s1_red_on: write reg(13) = 1\n");
}

static void s1_green_on(void)
{
	printf("    s1_green_on: write reg(14) = 1\n");
}

static void s1_blue_on(void)
{
	printf("    s1_blue_on: write reg(15) = 1\n");
}

/* ======================== Stage 2: ch01 ========================
 * 一份函数 + me 指针。封装的最朴素形态。
 */

struct s2_led {
	uint8_t pin;
	bool    is_on;
};

static void s2_led_on(struct s2_led *me)
{
	me->is_on = true;
	printf("    s2_led_on(pin=%u): write reg = 1\n", (unsigned)me->pin);
}

/* ======================== Stage 3: ch06 - ch11 ========================
 * 继承 + ops 表 + 多态 dispatch
 */

struct s3_led_base;

struct s3_led_ops {
	void (*on)(struct s3_led_base *me);
};

struct s3_led_base {
	const struct s3_led_ops *ops;
	const char              *name;
};

struct s3_led_gpio {
	struct s3_led_base base;
	uint8_t            pin;
};

struct s3_led_pwm {
	struct s3_led_base base;
	uint8_t            channel;
};

static void s3_gpio_on(struct s3_led_base *me)
{
	struct s3_led_gpio *self = (struct s3_led_gpio *)me;
	printf("    s3_gpio_on  [%s]: write reg(%u) = 1\n",
	       me->name, (unsigned)self->pin);
}

static void s3_pwm_on(struct s3_led_base *me)
{
	struct s3_led_pwm *self = (struct s3_led_pwm *)me;
	printf("    s3_pwm_on   [%s]: PWM ch=%u duty=100%%\n",
	       me->name, (unsigned)self->channel);
}

static void s3_led_on(struct s3_led_base *me)
{
	me->ops->on(me);    /* 多态 dispatch */
}

/* ======================== Stage 4: ch12 - ch15 ========================
 * 向上转型 + 全局句柄 + 板级初始化
 */

static struct s3_led_gpio g_gpio;
static struct s3_led_pwm  g_pwm;

static struct s3_led_base *g_led_red;
static struct s3_led_base *g_led_status;

static const struct s3_led_ops gpio_ops = { .on = s3_gpio_on };
static const struct s3_led_ops pwm_ops  = { .on = s3_pwm_on };

static void s4_led_board_init(void)
{
	g_gpio.base.ops  = &gpio_ops;
	g_gpio.base.name = "RED";
	g_gpio.pin       = 13;

	g_pwm.base.ops   = &pwm_ops;
	g_pwm.base.name  = "STAT";
	g_pwm.channel    = 1;

	g_led_red    = &g_gpio.base;     /* 向上转型 */
	g_led_status = &g_pwm.base;
}

/* ======================== Replay ======================== */

int main(void)
{
	printf("=========================================\n");
	printf("  ch18 - the road one LED has walked\n");
	printf("=========================================\n");

	printf("\n[stage 1] ch01 - 3 LEDs, 3 copies\n");
	s1_red_on();
	s1_green_on();
	s1_blue_on();

	printf("\n[stage 2] ch01 - struct + me pointer\n");
	struct s2_led red   = { .pin = 13, .is_on = false };
	struct s2_led green = { .pin = 14, .is_on = false };
	struct s2_led blue  = { .pin = 15, .is_on = false };
	s2_led_on(&red);
	s2_led_on(&green);
	s2_led_on(&blue);

	printf("\n[stage 3] ch06-ch11 - inheritance + ops + polymorphism\n");
	struct s3_led_gpio g = { .base = {.ops = &gpio_ops, .name = "RED"}, .pin = 13 };
	struct s3_led_pwm  p = { .base = {.ops = &pwm_ops,  .name = "STAT"}, .channel = 1 };
	s3_led_on(&g.base);
	s3_led_on(&p.base);

	printf("\n[stage 4] ch12-ch15 - upcasting + handle + led_board_init\n");
	s4_led_board_init();
	s3_led_on(g_led_red);
	s3_led_on(g_led_status);

	printf("\n[stage 5] ch17 - linker auto registration (see 17-initcall)\n");
	printf("    main never references *_init, drivers register themselves\n");

	printf("\n=========================================\n");
	printf("  one LED, 17 chapters, 4000 lines covered\n");
	printf("=========================================\n");

	printf("\nPress Enter to exit...\n");
	getchar();
	return 0;
}
