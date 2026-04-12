/**
 * @file     led.h
 * @brief    LED模块公共接口 - 头文件就是"菜单"，只写能点的菜
 * @author   兆鸣嵌入式
 * @series   C语言·一个LED讲透面向对象
 * @episode  EP07 - 信息隐藏：static与头文件
 *
 * 核心思想：
 *   头文件 = 餐厅菜单，只告诉你"能点什么菜"（函数声明）；
 *   .c文件 = 后厨，具体怎么炒的你不用管（实现细节）。
 *
 *   EP06里，struct成员暴露在外，谁都能直接改 me->is_on = true，
 *   结果同事改了数据但没操作硬件，LED状态和实际引脚不一致——全乱了。
 *
 *   EP07的改进：
 *   1. struct定义还在头文件（不教不透明指针），但注释明确"不要直接改"
 *   2. 提供 led_get_state() 让外部"看"状态，而不是直接读 me->is_on
 *   3. .c里的辅助函数用 static 修饰，外部根本调不到
 *
 * 信息隐藏不是不信任 —— 是锁上门，谁都不容易犯错。
 */

#ifndef LED_H
#define LED_H

#include "platform.h"

/* ---- 数据结构 ---- */

/**
 * @brief  LED对象
 *
 * 【重要约定】
 *   请通过 led_xxx() 函数操作LED，不要直接修改下面的成员！
 *   直接改成员会导致软件状态和硬件状态不一致（详见main.c演示）。
 *
 *   这就像餐厅规矩：要加醋找服务员（调函数），
 *   别自己跑进后厨乱拿（直接改struct）。
 */
typedef struct {
    uint8_t  pin;           /* GPIO引脚号      —— 请勿直接修改 */
    uint8_t  brightness;    /* 亮度，0~100     —— 请勿直接修改 */
    bool     is_on;         /* 开关状态        —— 请勿直接修改 */
} Led_t;

/* ---- 公共函数声明（菜单上的菜） ---- */

/**
 * @brief  初始化LED
 * @param  me   指向要操作的LED对象
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

/**
 * @brief  获取LED当前状态（安全的"读"接口）
 * @param  me      指向要查询的LED对象
 * @param  is_on   输出参数：true=亮, false=灭（可传NULL表示不需要）
 * @param  brightness  输出参数：当前亮度0~100（可传NULL表示不需要）
 * @return 0=成功, -1=参数错误
 *
 * 为什么不直接读 me->is_on？
 *   因为直接读成员就养成了直接访问struct的习惯，
 *   迟早有一天你会手滑写成 me->is_on = true，
 *   而忘了操作硬件——bug就来了。
 *   通过函数读，养成"什么都通过接口"的好习惯。
 */
int led_get_state(const Led_t *me, bool *is_on, uint8_t *brightness);

#endif /* LED_H */
