# pc/ — PC 模拟版

不需要任何开发板，gcc 一句编译就能看到效果。

## 编译运行

```bash
make
./demo
```

## 看到什么

教学用的 mini HAL 库跑一个最小例子。代码风格和真实 STM32 HAL
（`stm32h7xx_hal_gpio.c`）几乎一字不差：

- `GPIO_TypeDef` struct 把所有 GPIO 寄存器打包
- `GPIOA / GPIOB / GPIOC` 是同一个 struct 的多实例
- `HAL_GPIO_Init / HAL_GPIO_DeInit` 是构造 / 析构
- `GPIO_TypeDef *GPIOx` 是 me 指针
- `static` 工具函数在 `.c` 内部（`get_pin_number / set_2bit_field`）

跑完打印一张映射表，把 ch01 到 ch04 学的概念和真实 HAL 命名一一对应。

## Windows 用户

跑一次 `make` 就有 `demo.exe`。

## 配套章节

[第 5 章 · HAL 库源码漫游](../../../book/01-封装/05-HAL映射.md)
