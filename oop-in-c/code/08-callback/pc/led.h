/* SPDX-License-Identifier: MIT */
/**
 * @file  led.h
 * @brief 函数指针当参数 - 延迟决定
 *
 * @details
 * 本章主题: 函数指针除了能存进一个变量 (ch07), 还能当参数传给别人.
 *
 * test_led(on, off, id) 不写死调谁的 on/off. 调用方拨什么号码进来,
 * test_led 就在内部"代为拨"那个号码. "调谁"这件事不是写函数时定下来的,
 * 而是调用时再说. 同一个 test_led, 传 gpio_on/gpio_off + 引脚号 15
 * 跑出 GPIO 的样子, 传 pwm_on/pwm_off + 通道号 3 跑出 PWM 的样子,
 * 传 i2c_on/i2c_off + 地址 0x50 跑出 I2C 的样子.
 *
 * 第三个参数 id 是个通用名: 给 GPIO 当引脚号, 给 PWM 当通道号, 给 I2C
 * 当从机地址. 真实硬件细节 (HAL_GPIO_WritePin / __HAL_TIM_SET_COMPARE
 * / HAL_I2C_Master_Transmit) 完整工程见 ch15 15-platform/, 对函数
 * 指针主线本身没有影响.
 *
 * 所有 on/off 函数签名都是 void name(int param) 一致 -- 这样同一对函数
 * 指针参数 void (*)(int) 能匹配任何一组实现. 类型不一致的 on/off 编译
 * 直接报错, 不会等到运行时才发现.
 */

#ifndef LED_H
#define LED_H

/* ---- 三种 LED 的 on / off 占位声明 ---- */

void gpio_on(int pin);
void gpio_off(int pin);

void pwm_on(int channel);
void pwm_off(int channel);

void i2c_on(int addr);
void i2c_off(int addr);

/*
 * 通用测试函数: 不写死调谁的 on/off, 调用方传进来.
 * 第三个参数 id 是通用名 -- GPIO 拿来当引脚号, PWM 当通道号, I2C 当地址.
 */
void test_led(void (*on)(int), void (*off)(int), int id);

#endif /* LED_H */
