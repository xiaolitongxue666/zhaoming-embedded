/**
 * @file     led.h
 * @brief    LED模块公共接口 - struct打包数据 + me指针统一操作
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP06 - 封装：struct与me指针
 *
 * 核心思想：
 *   把描述一颗LED的所有数据打包进 Led_t，
 *   每个函数的第一个参数是 Led_t *me —— "我操作的是哪颗LED"。
 *   同一份代码，不同的数据 —— 这就是封装。
 *
 * 注意：
 *   EP06还没教static和信息隐藏（那是EP07的内容），
 *   所以结构体直接在头文件里定义，让外部可以看到内部成员。
 */

#ifndef LED_H
#define LED_H

#include "platform.h"

/* ---- 数据结构 ---- */

/**
 * @brief  LED对象 —— 把描述一颗LED的所有数据打包在一起
 *
 * 想象成一张"身份证"：
 *   pin        -> 这颗LED接在哪个引脚
 *   brightness -> 当前亮度（0~100）
 *   is_on      -> 现在是亮着还是灭着
 */
typedef struct {
    uint8_t  pin;           /* GPIO引脚号 */
    uint8_t  brightness;    /* 亮度，0~100 */
    bool     is_on;         /* 开关状态：true=亮, false=灭 */
} Led_t;

/* ---- 函数声明 ---- */

/**
 * @brief  初始化LED
 * @param  me   指向要操作的LED对象（"我是谁"）
 * @param  pin  LED连接的GPIO引脚号
 * @return 0=成功, -1=参数错误
 */
int led_init(Led_t *me, uint8_t pin);

/**
 * @brief  反初始化LED（释放资源）
 * @param  me  指向要操作的LED对象
 * @return 0=成功, -1=参数错误
 */
int led_deinit(Led_t *me);

/**
 * @brief  点亮LED
 * @param  me  指向要操作的LED对象
 * @return 0=成功, -1=参数错误
 */
int led_on(Led_t *me);

/**
 * @brief  熄灭LED
 * @param  me  指向要操作的LED对象
 * @return 0=成功, -1=参数错误
 */
int led_off(Led_t *me);

/**
 * @brief  翻转LED状态（亮->灭, 灭->亮）
 * @param  me  指向要操作的LED对象
 * @return 0=成功, -1=参数错误
 */
int led_toggle(Led_t *me);

/**
 * @brief  设置LED亮度
 * @param  me          指向要操作的LED对象
 * @param  brightness  亮度值，0~100
 * @return 0=成功, -1=参数错误, -2=亮度超范围
 */
int led_set_brightness(Led_t *me, uint8_t brightness);

#endif /* LED_H */
