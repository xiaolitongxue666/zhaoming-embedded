/**
 * @file    platform.h
 * @brief   平台抽象层 - GPIO 操作接口（跨章共用的对外封装函数声明）
 * @author  兆鸣嵌入式
 * @note    "换硬件不改应用"的兑现层。同一套封装函数声明，PC 上 printf 模拟、
 *          STM32 上操作真实寄存器、Linux 上写 sysfs，上层一字不改。
 *
 *          这一份头文件是从 ch01 起整本书贯穿到底的对外接口，每一章的
 *          led 子类实现里都只调这里声明的 4 个函数（platform_gpio_init /
 *          deinit / write / read）。从来不直接碰任何寄存器、不碰 sysfs、
 *          不碰 ops 字段。这是工业代码"对外稳定、对内可换"的纪律。
 *
 *          见 ch15 § 15.1 四层架构 / ch16 § 16.3 四层架构。
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

/* GPIO模式定义 */
#define GPIO_MODE_OUTPUT    0   /* 输出模式 */
#define GPIO_MODE_INPUT     1   /* 输入模式 */

/*
 * pin 是一个编码值，包含 port + 引脚号。工业代码通用做法：
 *   #define PIN(port, num)   (((port) << 4) | ((num) & 0xF))
 * 调用方写 PIN(D, 12) 表示 PD12。port 信息藏在 pin 编码里，
 * 接口签名永远只有一个 pin 参数——这样换芯片（port 数量、命名都变）
 * 时，签名不动、调用点不动，只有 PIN 宏的展开和 platform 实现要改。
 * Linux 内核 gpio_set_value(unsigned int gpio, ...) 同款做法。
 * 详见 ch01 1.x 节 + ch15 / 附录 B 展开。
 */

/**
 * @brief  初始化一个GPIO引脚
 * @param  pin   引脚号
 * @param  mode  GPIO_MODE_OUTPUT 或 GPIO_MODE_INPUT
 */
void platform_gpio_init(uint8_t pin, uint8_t mode);

/**
 * @brief  反初始化一个GPIO引脚（释放资源）
 * @param  pin  引脚号
 */
void platform_gpio_deinit(uint8_t pin);

/**
 * @brief  写GPIO电平
 * @param  pin    引脚号
 * @param  value  true=高电平, false=低电平
 */
void platform_gpio_write(uint8_t pin, bool value);

/**
 * @brief  读GPIO电平
 * @param  pin  引脚号
 * @return true=高电平, false=低电平
 */
bool platform_gpio_read(uint8_t pin);

#endif /* PLATFORM_H */
