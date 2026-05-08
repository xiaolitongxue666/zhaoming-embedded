# 05-hal-mapping — HAL 库源码漫游

第 5 章 [HAL 库源码漫游](../../../book/01-封装/05-HAL映射.md) 的配套代码。

## 目录结构

```
05-hal-mapping/
├── pc/                 教学用 mini HAL，gcc 一句编译跑
│   ├── gpio_typedef.h      模拟 GPIO_TypeDef + GPIOA/B/C
│   ├── hal_gpio.h          HAL_GPIO_Init / WritePin / ...
│   ├── hal_gpio.c          .c 实现 + static 辅助函数
│   └── main.c              用真实 HAL 命名跑一遍
└── platform-mcu/
    └── stm32/          对照真实 stm32h7xx_hal_gpio.c
```

## 教学要点

这一章是验证课，不引入新概念。跑完确认你学的概念在工业 HAL 库里
全部对应得上：

| 你学的（ch01-ch04） | HAL 真实命名 |
|---|---|
| `struct led` | `GPIO_TypeDef` |
| `red_led / green_led` | `GPIOA / GPIOB / GPIOC` |
| `struct led *me` | `GPIO_TypeDef *GPIOx` |
| `led_` 前缀 | `HAL_GPIO_` 前缀 |
| `led_init / led_deinit` | `HAL_GPIO_Init / HAL_GPIO_DeInit` |
| `static` 工具函数 | `static` 工具函数（一字不差） |

## 编译运行（PC 版）

```bash
cd pc
make
./demo
```

## STM32 / Linux 完整工程

完整跑通的 STM32 工程见 [附录 B](../../../book/附录/B-STM32完整工程.md)，
完整跑通的 Linux 工程见 [附录 C](../../../book/附录/C-Linux完整工程.md)。
（这两个附录在阶段 5 完成。）
