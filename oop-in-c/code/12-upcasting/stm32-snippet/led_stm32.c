/* SPDX-License-Identifier: MIT */
/*
 * 本文件是 STM32 真实硬件版本片段。需要 STM32CubeMX 生成的 hi2c1 / htim3 等外设句柄。
 * 实际工程里 platform_xxx 函数体直接调 HAL API。
 * 用 mock-CubeMX 头文件可独立编译验证语法。
 */

/*
 * led_stm32.c - 父类统一接口 led_on 落到 STM32 上的样子
 *
 * 父类 led_on / led_off 写在 led.c, 子类实现 (gpio_on / pwm_on / i2c_on)
 * 走 HAL 真实硬件. STM32 上 GPIO 落到 HAL_GPIO_WritePin, PWM 落到
 * __HAL_TIM_SET_COMPARE + HAL_TIM_PWM_Start/Stop, I2C 落到
 * HAL_I2C_Master_Transmit. 应用层 / 父类 / 子类签名 / board_init.c 一字
 * 不改 -- 整张 ops 表的形态、子类 init 的形态全部不动.
 *
 * 子类实现里第一行 (struct led_xxx *)me 强转回子类是合法的, 因为本章
 * 把 base 放在子类的第 0 字段 (向上转型不变量, 见 ch12 § 12.2).
 *
 * 跟 pc/ 唯一的差别就在这个文件: 把 printf 模拟换成真实 HAL 操作.
 */

#include "led.h"
#include "stm32f4xx_hal.h"

/* CubeMX 生成的外设句柄: PWM 走 TIM3, I2C 走 I2C1. */
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

/* ============== 子类一: GPIO LED ==============
 *
 * 实现层接 struct led_base *me. 第一行 (struct led_gpio *)me 强转回
 * 子类拿 pin / on_level. base 在 led_gpio 的第 0 字段, 所以这一招合法
 * (见 ch12 § 12.2 向上转型不变量). */

static int gpio_on(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
	platform_gpio_write(self->pin, self->on_level);
	me->is_on = true;
	return 0;
}

static int gpio_off(struct led_base *me)
{
	struct led_gpio *self = (struct led_gpio *)me;
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
	me->pin      = pin;
	me->on_level = on_level;
	platform_gpio_init(pin, GPIO_MODE_OUTPUT);
	platform_gpio_write(pin, !on_level);    /* 上电先关灯 */
	return 0;
}

/* ============== 子类二: PWM LED ==============
 *
 * __HAL_TIM_SET_COMPARE 改 CCR 寄存器, HAL_TIM_PWM_Start / Stop 控通道
 * 使能位. duty 字段 0-100 (百分比) 转 CCR 值, ARR 假设 1000 (CubeMX
 * 配 TIM3 ARR=999 + 1kHz 频率 时这一档刚好). */

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

static const struct led_ops pwm_ops = {
	.on  = pwm_on,
	.off = pwm_off,
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
 * 厂家协议简化版, 真实芯片协议 (PCA9555 等) 命令字位宽和寄存器布局
 * 不一样, 替换 cmd 数组内容即可. addr << 1 是因为 HAL_I2C 接的是
 * 8 位地址 (7 位地址 + R/W 位), 调用方传的是标准 7 位地址. */

static int i2c_on(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
	uint8_t cmd[1] = { 0x01 };

	HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(self->addr << 1),
	                        cmd, sizeof(cmd), 100);
	me->is_on = true;
	return 0;
}

static int i2c_off(struct led_base *me)
{
	struct led_i2c *self = (struct led_i2c *)me;
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
