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

## GPIO 端口实例：extern 与定义为何分两处

真实 STM32H7 上 `GPIOA` 是指向固定物理地址的指针：

```c
#define GPIOA  ((GPIO_TypeDef *)0x58020000UL)
```

PC 模拟用 **全局变量** 代替寄存器块；`GPIOA` 宏改为 `(&g_gpioa_regs)`，
用法与真机一致（`GPIOA->MODER`、`HAL_GPIO_Init(GPIOA, …)`）。

| 位置 | 写法 | 作用 |
|------|------|------|
| `gpio_typedef.h` | `extern GPIO_TypeDef g_gpioa_regs;` | **声明**：告诉编译器符号存在；配合 `#define GPIOA (&g_gpioa_regs)` |
| `hal_gpio.c` | `GPIO_TypeDef g_gpioa_regs;` | **定义**：真正分配内存；全项目只能有一处（C 单一定义规则） |

若在头文件里直接写 `GPIO_TypeDef g_gpioa_regs;`（无 `extern`），每个
`#include` 它的 `.c` 都会再定义一份，链接时报 multiple definition。

定义放在 `hal_gpio.c` 对应 ch04 **数据归位**：这三个变量是 HAL 模块的
共享状态（模拟硅片寄存器），由实现层 `.c` 拥有，头文件只对外声明。

## Windows 用户

跑一次 `make` 就有 `demo.exe`。

## 配套章节

[第 5 章 · HAL 库源码漫游](../../../book/01-封装/05-HAL映射.md)
