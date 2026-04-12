/**
 * @file     led.h
 * @brief    LED模块公共接口 - 没有一个裸露的全局变量
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP09 - 你的全局变量，该死了
 *
 * EP09核心思想：每一份数据都有主人
 *
 *   数据归位三步走：
 *   1. 实例数据（pin, brightness）→ struct成员，跟着me指针走
 *   2. 模块数据（init_count, debug_flag）→ static变量，文件私有
 *   3. 只读常量（MAX_BRIGHTNESS）→ static const，不可修改
 *
 *   改造后：.c文件开头没有一个裸露的全局变量。
 *   数据没有主人，bug就是主人。
 */

#ifndef LED_H
#define LED_H

#include "platform.h"

/* ---- 数据结构（实例数据跟着me走）---- */

/**
 * @brief  LED对象
 *
 * pin和brightness是实例数据，每个LED对象独立拥有。
 * 红灯有红灯的pin，绿灯有绿灯的pin，互不干扰。
 */
typedef struct {
    uint8_t  pin;           /* GPIO引脚号      —— 实例数据 */
    uint8_t  brightness;    /* 亮度，0~100     —— 实例数据 */
    bool     is_on;         /* 开关状态        —— 实例数据 */
    bool     initialized;   /* 是否已初始化    —— 实例数据 */
} Led_t;

/* ---- 公共函数声明 ---- */

/**
 * @brief  初始化LED（构造函数）
 * @param  me   指向LED对象
 * @param  pin  GPIO引脚号（0 ~ 15）
 * @return 0=成功, -1=参数错误, -2=引脚超范围
 */
int led_init(Led_t *me, uint8_t pin);

/**
 * @brief  反初始化LED（析构函数）
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

/**
 * @brief  获取模块初始化次数
 *
 * init_count是模块级数据（static），外部不能直接访问。
 * 想知道？通过这个函数 —— 数据的主人说了算。
 *
 * @return 当前模块累计初始化次数
 */
int led_get_init_count(void);

#endif /* LED_H */
