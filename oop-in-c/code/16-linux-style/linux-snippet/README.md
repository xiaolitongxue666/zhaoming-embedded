# ch16 Linux snippet

ch16 整章是讲 Linux 内核子系统是怎么用 OOP 风格组织的。
"snippet" 就是 Linux 内核源码本身，

- `include/linux/gpio/driver.h` 第 415 行：`struct gpio_chip` 真身
- `drivers/gpio/gpiolib.c` 第 3245 行：`gpiod_set_value`
- `drivers/gpio/gpiolib.c` 第 3057 行：`gc->set(gc, ...)`（多态 dispatch）
- `drivers/leds/leds-gpio.c`：调 gpiod_set_value 驱动 LED 的真实驱动

pc/ 里的 gpio_chip / gpiolib / vendor_a / vendor_b 是这套架构的山寨版。
读完 pc/ 再去看上面四个内核源文件，你会发现"原来就是这一招"。
