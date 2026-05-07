/* SPDX-License-Identifier: MIT */
/*
 * 本文件是 STM32 真实硬件版本片段。需要 STM32CubeMX 生成的 hi2c1 / htim3 等外设句柄。
 * 实际工程里 platform_xxx 函数体直接调 HAL API。
 * 用 mock-CubeMX 头文件可独立编译验证语法。
 */

/*
 * led_stm32.c - ch13 STM32 等效片段（container_of 反推子类）
 *
 * 父类 led_on / led_off / led_set_brightness 写在 led.c, 子类实现里
 * 第一行用 container_of 反推自己, 后面调真实 HAL 操作. STM32 上 GPIO
 * 落到 HAL_GPIO_WritePin, PWM 落到 __HAL_TIM_SET_COMPARE +
 * HAL_TIM_PWM_Start / Stop, I2C 落到 HAL_I2C_Master_Transmit. 应用层 /
 * 父类 / 子类签名一字不改.
 *
 * 跟 pc/ 唯一的差别就在这个文件: 把 printf 模拟换成真实 HAL 操作.
 *
 * container_of 这一招在 STM32 上编译产物就是 ARM Cortex-M 的
 * SUB Rd, Rn, #imm 一条指令, 零运行时开销. GPIO 子类故意把 base 挪
 * 到中间字段 (magic 前缀), container_of 照样对 -- 强转那一招 (ch12)
 * 在这种布局下会算错地址.
 *
 * 见 ch13 § 13.9 在 STM32 上长什么样.
 */

#include "led.h"
#include "container_of.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄. */
extern TIM_HandleTypeDef htim3;
extern I2C_HandleTypeDef hi2c1;

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

/* ============== 子类一: GPIO LED (base 在中间, container_of 反推) ==============
 *
 * GPIO 子类的 base 故意放在 magic 之后, offsetof != 0. container_of
 * 在编译期把这个偏移算出来, 运行时一条减法指令还原回外层 struct
 * 起点. 这是 ch13 整章想证明的核心 -- container_of 与 base 的位置
 * 无关, 强转那一招在这种布局下会算错 4 字节. */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = container_of(me, struct led_gpio, base);
	platform_gpio_write(self->pin, !self->on_level);
	me->is_on = false;
	return 0;
}

static const struct led_ops gpio_ops = {
	.on  = gpio_on,
	.off = gpio_off,
};

int led_gpio_init(struct led_gpio *me, const char *name,
                  uint8_t pin, bool on_level)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &gpio_ops);
	if (rc != 0)
		return rc;
	me->magic    = 0xCAFE;
	me->pin      = pin;
	me->on_level = on_level;

	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);     /* 上电先关灯 */
	return 0;
}

/* ============== 子类二: PWM LED ==============
 *
 * 走 TIM3 PWM 通道. duty 字段 0-100 (百分比) 转 CCR 值, ARR 假设 1000. */

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
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	uint32_t ch  = pwm_channel_to_hal(self->channel);
	uint32_t ccr = (uint32_t)self->duty * 1000U / 100U;

	__HAL_TIM_SET_COMPARE(&htim3, ch, ccr);
	HAL_TIM_PWM_Start(&htim3, ch);
	me->is_on = true;
	return 0;
}

static int pwm_off(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	uint32_t ch = pwm_channel_to_hal(self->channel);

	__HAL_TIM_SET_COMPARE(&htim3, ch, 0);
	HAL_TIM_PWM_Stop(&htim3, ch);
	me->is_on = false;
	return 0;
}

static int pwm_set_brightness(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	uint32_t ch  = pwm_channel_to_hal(self->channel);
	uint32_t ccr;

	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	ccr = (uint32_t)brightness * 1000U / 100U;

	__HAL_TIM_SET_COMPARE(&htim3, ch, ccr);
	if (brightness > 0)
		HAL_TIM_PWM_Start(&htim3, ch);
	else
		HAL_TIM_PWM_Stop(&htim3, ch);
	me->is_on = (brightness > 0);
	return 0;
}

static const struct led_ops pwm_ops = {
	.on             = pwm_on,
	.off            = pwm_off,
	.set_brightness = pwm_set_brightness,
};

int led_pwm_init(struct led_pwm *me, const char *name,
                 uint8_t channel, uint8_t duty)
{
	int rc;
	if (!me)
		return -1;
	if (duty > 100)
		return -2;
	rc = led_base_init(&me->base, name, &pwm_ops);
	if (rc != 0)
		return rc;
	me->channel = channel;
	me->duty    = duty;
	return 0;
}

/* ============== 子类三: I2C 扩展芯片 LED ==============
 *
 * HAL_I2C_Master_Transmit 一次发 1 字节命令: 0x01 = ON, 0x00 = OFF.
 * addr << 1 是因为 HAL_I2C 接的是 8 位地址 (7 位地址 + R/W 位). */

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	uint8_t cmd[1] = { 0x01 };

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(self->addr << 1),
	                        cmd, sizeof(cmd), 100);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	uint8_t cmd[1] = { 0x00 };

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(self->addr << 1),
	                        cmd, sizeof(cmd), 100);
	me->is_on = false;
	return 0;
}

static const struct led_ops i2c_ops = {
	.on  = i2c_on,
	.off = i2c_off,
};

int led_i2c_init(struct led_i2c *me, const char *name,
                 uint8_t bus, uint8_t addr)
{
	int rc;
	if (!me)
		return -1;
	rc = led_base_init(&me->base, name, &i2c_ops);
	if (rc != 0)
		return rc;
	me->bus  = bus;
	me->addr = addr;
	return 0;
}
