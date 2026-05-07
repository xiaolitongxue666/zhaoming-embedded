/* SPDX-License-Identifier: MIT */
/*
 * led_errors.h - LED 子系统错误码 (Linux 用户态版本).
 *
 * 这一份保留 platform_err_t 这个类型名, 跟 stm32_full 的 led_base.[hc] 接口
 * 字节级一致, 让两个工程的 led_base.[hc] / led_gpio.[hc] / led_pwm.[hc] /
 * led_i2c.[hc] 共享同一份外观. 但 enum 值的语义对齐 Linux 风 (POSIX errno):
 *
 *   PLATFORM_EOK    =  0     成功
 *   PLATFORM_EINVAL = -1     参数错
 *   PLATFORM_EIO    = -2     IO 错 (包括 libgpiod 调用失败 / sysfs 写失败 / i2c-dev ioctl 失败)
 *   PLATFORM_ENOMEM = -3     内存 / fd 等资源不足
 *
 * 应用层只需检查 ret == PLATFORM_EOK, 不需要区分具体哪一类 EIO. 真要区分,
 * 子类内部已经从 errno 拿到详细原因, 通过 perror / fprintf 打印出来即可.
 *
 * 注意: 这里不依赖任何 platform/ 头文件. 这个工程已经把 platform 抽象层
 * 删掉了 (Linux 内核已经把 platform 抽象做完了, 用户态再套一层是过度封装).
 * 子类直接调 libgpiod / sysfs PWM / i2c-dev.
 */

#ifndef APP_INCLUDE_LED_ERRORS_H_
#define APP_INCLUDE_LED_ERRORS_H_

typedef enum {
	PLATFORM_EOK    =  0,
	PLATFORM_EINVAL = -1,
	PLATFORM_EIO    = -2,
	PLATFORM_ENOMEM = -3,
} platform_err_t;

#endif /* APP_INCLUDE_LED_ERRORS_H_ */
