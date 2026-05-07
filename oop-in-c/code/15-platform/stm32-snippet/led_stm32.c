/* SPDX-License-Identifier: MIT */
/*
 * 本文件是 STM32 真实硬件版本片段。需要 STM32CubeMX 生成的 hi2c1 / htim3 等外设句柄。
 * 实际工程里 platform_xxx 函数体直接调 HAL API。
 * 用 mock-CubeMX 头文件可独立编译验证语法。
 */

/*
 * led_stm32.c - 父类 / 子类 / 板级 / 应用 四层落到 STM32 上的样子
 *
 * ch15 完整框架在真实 STM32 工程里只换一份文件: 把 PC 模拟版的 4 个
 * platform 封装函数 (printf 模拟) + PWM / I2C 子类的 printf 调用换成
 * 走 STM32 HAL 的真实实现. 父类 led.c (led_on / led_off /
 * led_set_brightness) / 板级 board_init.c / 应用 app.c 一字不动.
 *
 * gpio_on 子类实现里 platform_gpio_write(self->pin, self->on_level), 在
 * STM32 上调到底就是 HAL_GPIO_WritePin -> GPIOx->BSRR 一次 32 位 store
 * (原子). PWM 子类调 __HAL_TIM_SET_COMPARE 改 CCR 寄存器, I2C 子类调
 * HAL_I2C_Master_Transmit 发 1 字节命令. 三种 LED 子类的 ops 表 (gpio_ops
 * / pwm_ops / i2c_ops) 形态完全对齐.
 *
 * 见 ch15 § 15.10 在 STM32 上长什么样.
 */

#include "led.h"
#include "container_of.h"
#include "platform.h"
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

/* ============== GPIO 子类 ============== */

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

/* set_brightness 故意不填: GPIO 不支持调光, 走父类默认行为. */
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

/* ============== PWM 子类 ==============
 *
 * 走 TIM3 PWM 通道. duty 字段 0-100 (百分比) 转 CCR 值, ARR 假设 1000
 * (CubeMX 配 TIM3 ARR=999 + 1kHz 频率 时这一档刚好). */

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

/* ============== I2C 子类 ==============
 *
 * HAL_I2C_Master_Transmit 一次发 1 字节命令: 0x01 = ON, 0x00 = OFF.
 * 厂家协议简化版, 真实芯片协议 (PCA9555 等) 命令字位宽和寄存器布局
 * 不一样, 替换 cmd 数组内容即可. addr << 1 是因为 HAL_I2C 接的是 8 位
 * 地址 (7 位地址 + R/W 位). */

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
