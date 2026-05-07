/* SPDX-License-Identifier: MIT */
/*
 * led_linux.c - 函数指针当参数 在 Linux 用户态的样子
 *
 * 本文件是 Linux 用户态真实实现片段。
 * 需要内核启用 sysfs gpio / pwm + i2c-dev 模块。
 * 用法: 把 gpio / pwm / i2c 节点 export 出来后, 用户态进程读写文件 /
 * ioctl 操作硬件。
 *
 * 三种 on/off 实现走真实操作: GPIO 写 sysfs / PWM 写 sysfs duty_cycle /
 * I2C 走 i2c-dev 发命令. 三个函数签名都是 void name(int param), 能给
 * 同一个 test_led 当函数指针参数.
 *
 * 第三个参数在不同实现里含义不同: GPIO 是引脚号, PWM 是通道号, I2C
 * 是从机地址. 由调用方负责传匹配的 id.
 */

#include "led.h"
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

void gpio_on(int pin)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return;
	write(fd, "1", 1);
	close(fd);
}

void gpio_off(int pin)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return;
	write(fd, "0", 1);
	close(fd);
}

/*
 * pwm_on - 通过 sysfs 把一路 PWM 拉到 100% 占空比.
 *
 * sysfs 这一组节点 (period / duty_cycle / enable) 是 Linux 用户态控制
 * PWM 的标准方式. 写顺序有讲究: enable 之前必须先把 period / duty_cycle
 * 设好, 否则 echo 1 > enable 会 -EINVAL.
 *
 * period 单位是纳秒, 1_000_000 ns = 1 ms = 1 kHz, 普通 LED 这个频率不
 * 闪烁. duty_cycle 必须 <= period.
 */
void pwm_on(int channel)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%d/period", channel);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "1000000", 7);          /* 1 ms 周期 = 1 kHz */
		close(fd);
	}
	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", channel);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "1000000", 7);          /* duty 100% (= period) */
		close(fd);
	}
	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%d/enable", channel);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, "1", 1);
		close(fd);
	}
}

void pwm_off(int channel)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%d/enable", channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return;
	write(fd, "0", 1);
	close(fd);
}

/*
 * i2c_on - 通过 i2c-dev 给 7-bit 从机地址 addr 发 1 字节 0x01.
 *
 * /dev/i2c-1 是默认的 I2C 总线 1 节点 (内核要 CONFIG_I2C_CHARDEV=y).
 * ioctl(I2C_SLAVE) 把"目标从机地址"绑到这个 fd 上, 后续 write 就直接
 * 走 START + addr<<1 + cmd + STOP 的标准 I2C 时序.
 *
 * 错误路径: 任何一步失败 (open / ioctl) 都立即关 fd 返回, 防止句柄
 * 泄漏. ioctl 返回 < 0 时 errno 通常是 EBUSY (总线被占) 或 EREMOTEIO
 * (目标芯片没应答).
 */
void i2c_on(int addr)
{
	int fd;
	uint8_t cmd = 0x01;

	fd = open("/dev/i2c-1", O_RDWR);
	if (fd < 0)
		return;
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		close(fd);
		return;
	}
	write(fd, &cmd, 1);
	close(fd);
}

void i2c_off(int addr)
{
	int fd;
	uint8_t cmd = 0x00;

	fd = open("/dev/i2c-1", O_RDWR);
	if (fd < 0)
		return;
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		close(fd);
		return;
	}
	write(fd, &cmd, 1);
	close(fd);
}
