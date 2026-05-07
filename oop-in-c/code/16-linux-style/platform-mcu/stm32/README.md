# ch16 platform-mcu/stm32 · 没有 STM32 实战代码

ch16 是工程哲学章, 主题是"Linux 内核 gpio_chip 子系统的骨架". pc/ 山寨了一份能跑的教学版, linux-driver/userspace/ 给应用层 libgpiod 视角, 这一档 platform-mcu/stm32/ **不演示 STM32**: 这一章的 STM32 端没有特殊片段, 你的应用 / 驱动代码长成 pc/ 那个样子, 就已经是 Linux 内核风格了。

如果你想把 pc/ 的 gpiolib + vendor_a + vendor_b 框架移植到 STM32 裸机上，
做法是：

1. 把 vendor_a_set / vendor_b_set 里的 printf 替换成真实的 BSRR 写入。
2. board_init 期间调一次 vendor_a_probe / vendor_b_probe（或用 ch17 的
   initcall 自动注册）。
3. 应用层 / leds_gpio 驱动 0 修改。

效果就是"一份 leds-gpio.c 跑在不同 SoC 上"。
