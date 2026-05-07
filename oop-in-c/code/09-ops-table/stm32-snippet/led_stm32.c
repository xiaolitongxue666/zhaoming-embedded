/* SPDX-License-Identifier: MIT */
/*
 * 本文件是 STM32 真实硬件版本片段。需要 STM32CubeMX 生成的 hi2c1 / htim3 等外设句柄。
 * 实际工程里 platform_xxx 函数体直接调 HAL API。
 * 用 mock-CubeMX 头文件可独立编译验证语法。
 */

/*
 * led_stm32.c - ops 表在 STM32 上的样子
 *
 * led_ops_gpio / led_ops_pwm 这两张表常驻 .rodata, 整个 firmware
 * 全局只有一份, 不占 RAM. 这是 ARM 嵌入式上 ops 表落地的最大好处.
 *
 * 跟 pc/ 唯一的差别就在这个文件: pc/ 用 printf 模拟硬件动作, STM32
 * 上 gpio_xxx 走 HAL_GPIO_WritePin, pwm_xxx 走 HAL_TIM_PWM_xxx +
 * __HAL_TIM_SET_COMPARE 改占空比. ops 表的形态、子类 init 的形态、
 * 父类 test_led 走 ops 分发的形态全部一字不动. 这就是"换硬件不改应用".
 *
 * 真实工程里 PWM / I2C 句柄通过 CubeMX 生成的全局变量拿到, 这里片段
 * 用 extern 声明引用. 见 ch09 § 9.5 在 STM32 上长什么样.
 */

#include "led.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄. 工业代码里通过 #include "main.h" 拿到这些
 * extern 声明, 这里片段直接 extern 一下方便对照. */
extern TIM_HandleTypeDef htim3;

/* ============== platform 层 (GPIO 封装函数) ============== */

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	GPIO_InitTypeDef cfg = {0};

	__HAL_RCC_GPIOA_CLK_ENABLE();

	cfg.Pin   = (uint16_t)(1U << pin);
	cfg.Mode  = (mode == GPIO_MODE_OUTPUT) ?
	            GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
	cfg.Pull  = GPIO_NOPULL;
	cfg.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &cfg);
}

void platform_gpio_deinit(uint8_t pin)
{
	HAL_GPIO_DeInit(GPIOA, (uint16_t)(1U << pin));
}

void platform_gpio_write(uint8_t pin, bool value)
{
	HAL_GPIO_WritePin(GPIOA, (uint16_t)(1U << pin),
	                  value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool platform_gpio_read(uint8_t pin)
{
	return HAL_GPIO_ReadPin(GPIOA, (uint16_t)(1U << pin)) == GPIO_PIN_SET;
}

/* ============== 子类层 (gpio_on / pwm_on 在 STM32 上的实现) ==============
 *
 * pc/ 模拟版用 printf, STM32 版换成真实 HAL 操作. 函数名/签名/static 修饰
 * 都跟 pc/led.c 一字不变. ops 表通过 designated initializer 把这些静态
 * 函数挂到 .on / .off / .toggle 字段, 落 .rodata 段全程序唯一一份.
 *
 * 子类 init (led_gpio_init / led_pwm_init) / 父类 test_led / led_base
 * 共有 init 一字不变, 都在本片段视野之外的文件里 -- 那些代码不依赖
 * 平台, PC 和 STM32 共用同一份.
 */

/* ---- GPIO 子类: 拉 GPIO 电平 ---- */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = true;
	platform_gpio_write(self->pin, true);
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	me->is_on = false;
	platform_gpio_write(self->pin, false);
	return 0;
}

static int gpio_toggle(struct led_base *me)
{
	if (me->is_on)
		return gpio_off(me);
	return gpio_on(me);
}

/* ---- PWM 子类: 改 TIM3 通道占空比 ----
 *
 * __HAL_TIM_SET_COMPARE 直接改 CCR 寄存器, HAL_TIM_PWM_Start / Stop
 * 控通道使能位. duty 字段 0-100 (百分比) 转成 CCR 值, ARR 假设 1000
 * (CubeMX 配 TIM3 的 ARR=999 + 频率 1kHz 时这一档刚好). */

static uint32_t pwm_channel_to_hal(uint8_t channel)
{
	switch (channel) {
	case 1:  return TIM_CHANNEL_1;
	case 2:  return TIM_CHANNEL_2;
	case 3:  return TIM_CHANNEL_3;
	case 4:  return TIM_CHANNEL_4;
	default: return TIM_CHANNEL_1;
	}
}

static int pwm_on(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	uint32_t ch  = pwm_channel_to_hal(self->channel);
	uint32_t ccr = (uint32_t)self->duty * 1000U / 100U;

	__HAL_TIM_SET_COMPARE(&htim3, ch, ccr);
	HAL_TIM_PWM_Start(&htim3, ch);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = (struct led_pwm *)me;
	uint32_t ch = pwm_channel_to_hal(self->channel);

	__HAL_TIM_SET_COMPARE(&htim3, ch, 0);
	HAL_TIM_PWM_Stop(&htim3, ch);
	me->is_on = false;
	return 0;
}

static int pwm_toggle(struct led_base *me)
{
	if (me->is_on)
		return pwm_off(me);
	return pwm_on(me);
}

/* ============== 两张 ops 表 ============== */

const struct led_ops led_ops_gpio = {
	.on     = gpio_on,
	.off    = gpio_off,
	.toggle = gpio_toggle,
};

const struct led_ops led_ops_pwm = {
	.on     = pwm_on,
	.off    = pwm_off,
	.toggle = pwm_toggle,
};
