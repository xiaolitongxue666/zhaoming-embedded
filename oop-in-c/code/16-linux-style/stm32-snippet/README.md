# ch16 STM32 snippet

ch16 是工程哲学章，pc/ 山寨了一份 Linux 内核的 gpio_chip 子系统。
这一章的 STM32 端没有特殊片段，你的应用 / 驱动代码长成 pc/ 那个样子，
就已经是 Linux 内核风格了。

如果你想把 pc/ 的 gpiolib + vendor_a + vendor_b 框架移植到 STM32 裸机上，
做法是：

1. 把 vendor_a_set / vendor_b_set 里的 printf 替换成真实的 BSRR 写入。
2. board_init 期间调一次 vendor_a_probe / vendor_b_probe（或用 ch17 的
   initcall 自动注册）。
3. 应用层 / leds_gpio 驱动 0 修改。

效果就是"一份 leds-gpio.c 跑在不同 SoC 上"。
