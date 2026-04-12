/**
 * @file     led_bad.h
 * @brief    LED模块（反面教材）- 全局变量满天飞
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP09 - 你的全局变量，该死了
 *
 * 【反面教材】
 *   这是EP09之前的"坏代码"。
 *   5个全局变量散在.c文件开头，两个LED共享g_pin，
 *   第二次init覆盖第一次 —— bug就藏在这里。
 *
 *   这个文件存在的唯一目的：让你看到"全局变量有多危险"。
 *   正确写法请看 led.h / led.c。
 */

#ifndef LED_BAD_H
#define LED_BAD_H

#include "platform.h"

/**
 * @brief  初始化LED（坏版本 —— 全局变量）
 * @param  pin  GPIO引脚号
 * @return 0=成功
 */
int bad_led_init(uint8_t pin);

/**
 * @brief  点亮LED（坏版本）
 * @return 0=成功
 */
int bad_led_on(void);

/**
 * @brief  熄灭LED（坏版本）
 * @return 0=成功
 */
int bad_led_off(void);

/**
 * @brief  设置亮度（坏版本）
 * @param  brightness  亮度值 0~255
 * @return 0=成功
 */
int bad_led_set_brightness(uint8_t brightness);

/**
 * @brief  获取当前引脚号（坏版本 —— 暴露问题用）
 * @return 当前g_pin的值
 */
int bad_led_get_pin(void);

/**
 * @brief  获取当前亮度（坏版本）
 * @return 当前g_brightness的值
 */
int bad_led_get_brightness(void);

#endif /* LED_BAD_H */
