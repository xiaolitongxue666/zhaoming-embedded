/* SPDX-License-Identifier: MIT */
/*
 * led_linux.c - 父类 / 子类 / 板级 / 应用 四层落到 Linux 用户态的样子
 *
 * 本文件是 Linux 用户态真实实现片段。
 * 需要内核启用 sysfs gpio / pwm + i2c-dev 模块。
 * 用法: 把 gpio / pwm / i2c 节点 export 出来后, 用户态进程读写文件 /
 * ioctl 操作硬件。
 *
 * ch15 完整框架在 Linux 用户态只换一份文件: 把 PC 模拟版的 4 个
 * platform 封装函数 (printf 模拟) 换成走 /sys/class/gpio 的真实
 * 文件读写. 父类 led_base.c / 子类 led.c / 板级 board_init.c / 应用 app.c
 * 一字不动.
 *
 * 子类 gpio_on 里 platform_gpio_write(self->pin, self->on_level)
 * 在 Linux 用户态调到底就是 echo 1 > /sys/class/gpio/gpioN/value,
 * 走内核 gpiolib 操作物理引脚. 见 ch15 § 15.11 在 Linux 上长什么样.
 *
 * PWM 子类 (channel + duty) 走 /sys/class/pwm/pwmchipN/pwmM 节点,
 * I2C 子类 (bus + addr) 走 /dev/i2c-N + ioctl(I2C_SLAVE) + write.
 * 三种子类的 *_on / *_off / *_set_brightness 函数签名都是
 * (struct led_base *me), 第一行 container_of 反推回子类拿硬件字段
 * (ch15 沿用 ch13 的 container_of 习惯, 跟 base 在不在 0 偏移无关).
 */

#include "led.h"
#include "platform.h"
#include "container_of.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

/* ============== platform_gpio_* (sysfs gpio 实现) ============== */

void platform_gpio_init(uint8_t pin, uint8_t mode)
{
	char path[64];
	int fd;

	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%u", (unsigned)pin);
		close(fd);
	}
	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/direction", (unsigned)pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		const char *dir = (mode == GPIO_MODE_OUTPUT) ? "out" : "in";
		write(fd, dir, strlen(dir));
		close(fd);
	}
}

void platform_gpio_deinit(uint8_t pin)
{
	int fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (fd >= 0) {
		dprintf(fd, "%u", (unsigned)pin);
		close(fd);
	}
}

void platform_gpio_write(uint8_t pin, bool value)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	fd = open(path, O_WRONLY);
	if (fd >= 0) {
		write(fd, value ? "1" : "0", 1);
		close(fd);
	}
}

bool platform_gpio_read(uint8_t pin)
{
	char path[64], buf[2] = {0};
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/gpio/gpio%u/value", (unsigned)pin);
	fd = open(path, O_RDONLY);
	if (fd < 0)
		return false;
	read(fd, buf, 1);
	close(fd);
	return buf[0] == '1';
}

/* ============== PWM 子类 ops 在 Linux 上的实现 (三件套全填) ==========
 *
 * sysfs pwmchip0/pwmN 节点 (CONFIG_PWM_SYSFS=y) 三个文件: period /
 * duty_cycle / enable. period 先于 duty_cycle 写, duty_cycle <= period.
 * enable 在 period / duty_cycle 都设好之后再写, 否则 -EINVAL.
 *
 * 三件套 (on / off / set_brightness) 在 PWM 子类全部填实, 兑现
 * 调光能力 (set_brightness 把 0..100 映射成 0..period_ns 的 duty_cycle).
 */

static int pwm_apply_duty(uint8_t channel, uint8_t duty)
{
	char path[64];
	int fd;
	unsigned long period_ns = 1000000UL;                  /* 1 kHz */
	unsigned long duty_ns   = period_ns * duty / 100UL;   /* 0..100 % */

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%u/period", (unsigned)channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	dprintf(fd, "%lu", period_ns);
	close(fd);

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%u/duty_cycle", (unsigned)channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	dprintf(fd, "%lu", duty_ns);
	close(fd);
	return 0;
}

static int pwm_apply_enable(uint8_t channel, bool on)
{
	char path[64];
	int fd;

	snprintf(path, sizeof(path),
		 "/sys/class/pwm/pwmchip0/pwm%u/enable", (unsigned)channel);
	fd = open(path, O_WRONLY);
	if (fd < 0)
		return -1;
	write(fd, on ? "1" : "0", 1);
	close(fd);
	return 0;
}

int pwm_on_linux(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	pwm_apply_duty(self->channel, self->duty);
	pwm_apply_enable(self->channel, true);
	me->is_on = true;
	return 0;
}

int pwm_off_linux(struct led_base *me)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	pwm_apply_enable(self->channel, false);
	me->is_on = false;
	return 0;
}

int pwm_set_brightness_linux(struct led_base *me, uint8_t brightness)
{
	struct led_pwm *self = container_of(me, struct led_pwm, base);
	if (brightness > 100)
		brightness = 100;
	self->duty = brightness;
	pwm_apply_duty(self->channel, brightness);
	me->is_on = (brightness > 0);
	return 0;
}

/* ============== I2C 子类 ops 在 Linux 上的实现 ==============
 *
 * /dev/i2c-N 字符设备 (CONFIG_I2C_CHARDEV=y) + ioctl(I2C_SLAVE) 把
 * 7-bit 从机地址绑到 fd, 后续 write 走 START + addr<<1 + data + STOP.
 *
 * cmd 字节是写到芯片寄存器的值: 0x01 = 开, 0x00 = 关. 真实 I/O 扩展
 * 芯片 (PCA9555 / TCA9535 等) 的寄存器映射要看 datasheet, 这里给的是
 * 最小化教学实现.
 *
 * 错误路径: open / ioctl / write 任意一步失败立即 close fd 返回 -1,
 * 防止句柄泄漏. ioctl 返回 < 0 通常 errno = EBUSY (总线被占) 或
 * EREMOTEIO (目标芯片没应答).
 */

static int i2c_send_byte(uint8_t bus, uint8_t addr, uint8_t cmd)
{
	char path[16];
	int fd;

	snprintf(path, sizeof(path), "/dev/i2c-%u", (unsigned)bus);
	fd = open(path, O_RDWR);
	if (fd < 0)
		return -1;
	if (ioctl(fd, I2C_SLAVE, addr) < 0) {
		close(fd);
		return -1;
	}
	if (write(fd, &cmd, 1) != 1) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
}

int i2c_on_linux(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	if (i2c_send_byte(self->bus, self->addr, 0x01) < 0)
		return -1;
	me->is_on = true;
	return 0;
}

int i2c_off_linux(struct led_base *me)
{
	struct led_i2c *self = container_of(me, struct led_i2c, base);
	if (i2c_send_byte(self->bus, self->addr, 0x00) < 0)
		return -1;
	me->is_on = false;
	return 0;
}
