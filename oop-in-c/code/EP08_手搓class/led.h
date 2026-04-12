/**
 * @file     led.h
 * @brief    LED模块公共接口 - "led_"前缀 = 类名
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP08 - 你用C手搓了一个class
 *
 * EP08核心思想：
 *   函数前缀 = 类名。所有LED的函数都以 led_ 开头，
 *   就像"沙县小吃"的招牌——看到前缀就知道是哪个模块的。
 *
 *   led_init = 构造函数（开门营业）
 *   led_deinit = 析构函数（打烊收工）
 *
 *   一个 led.h + 一个 led.c = 一个完整的C语言"类"。
 *
 *   C语言没有class关键字？你天天都在写。
 */

#ifndef LED_H
#define LED_H

#include "platform.h"

/* ---- 引脚有效范围 ---- */
#define LED_PIN_MAX    15   /* 引脚号范围：0 ~ 15 */

/* ---- 数据结构（= C++的成员变量）---- */

/**
 * @brief  LED对象
 *
 * 【重要约定】
 *   请通过 led_xxx() 函数操作LED，不要直接修改下面的成员！
 *   直接改成员会导致软件状态和硬件状态不一致。
 */
typedef struct {
    uint8_t  pin;           /* GPIO引脚号      —— 请勿直接修改 */
    uint8_t  brightness;    /* 亮度，0~100     —— 请勿直接修改 */
    bool     is_on;         /* 开关状态        —— 请勿直接修改 */
    bool     initialized;   /* 是否已初始化    —— 请勿直接修改 */
} Led_t;

/* ---- 公共函数声明（= C++的public方法）---- */

/**
 * @brief  初始化LED（= 构造函数，开门营业）
 *
 * 做三件事：
 *   1. 参数验证（引脚范围检查）
 *   2. 硬件配置（GPIO初始化为输出模式）
 *   3. 默认状态（灯灭、亮度0）
 *
 * @param  me   指向LED对象
 * @param  pin  GPIO引脚号（0 ~ LED_PIN_MAX）
 * @return 0=成功, -1=参数错误, -2=引脚超范围
 */
int led_init(Led_t *me, uint8_t pin);

/**
 * @brief  反初始化LED（= 析构函数，打烊收工）
 *
 * 做两件事：
 *   1. 关闭LED（确保硬件安全）
 *   2. 释放GPIO资源
 *
 * @param  me  指向LED对象
 * @return 0=成功, -1=参数错误
 */
int led_deinit(Led_t *me);

/**
 * @brief  点亮LED
 * @param  me  指向LED对象
 * @return 0=成功, -1=参数错误, -3=未初始化
 */
int led_on(Led_t *me);

/**
 * @brief  熄灭LED
 * @param  me  指向LED对象
 * @return 0=成功, -1=参数错误, -3=未初始化
 */
int led_off(Led_t *me);

/**
 * @brief  翻转LED状态
 * @param  me  指向LED对象
 * @return 0=成功, -1=参数错误, -3=未初始化
 */
int led_toggle(Led_t *me);

/**
 * @brief  设置LED亮度
 * @param  me          指向LED对象
 * @param  brightness  亮度值，0~100
 * @return 0=成功, -1=参数错误, -2=亮度超范围, -3=未初始化
 */
int led_set_brightness(Led_t *me, uint8_t brightness);

/**
 * @brief  获取LED当前状态
 * @param  me          指向LED对象
 * @param  is_on       输出：开关状态（可传NULL）
 * @param  brightness  输出：当前亮度（可传NULL）
 * @return 0=成功, -1=参数错误
 */
int led_get_state(const Led_t *me, bool *is_on, uint8_t *brightness);

#endif /* LED_H */
