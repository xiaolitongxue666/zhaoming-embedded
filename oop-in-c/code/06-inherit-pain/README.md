# 06-inherit-pain — 共性提取的痛点

第 6 章 [你的代码一半是重复的](../../../book/02-继承/06-代码一半重复.md) 的配套代码。

## 演化点

ch01 起点: `struct led { uint8_t pin; uint8_t brightness; bool is_on; }`

ch06 演化:

```c
struct led_base {
	uint8_t pin;
};

struct led {
	struct led_base base;   /* 公共部分放第一个 */
	uint8_t brightness;
	bool    is_on;
};

struct motor {
	struct led_base base;   /* 同样放第一个 */
	uint8_t pwm_duty;
	int8_t  direction;
};
```

把 LED 和 Motor 共有的 `pin` 字段提到 `struct led_base`，子类各自把 `led_base` 嵌套进 struct 第一个位置。基类 init 由子类 init 链式调用。

## 目录结构

```
06-inherit-pain/
├── pc/                 完整可跑的 PC 模拟版
└── platform-mcu/
    └── stm32/          STM32 真机版（用 PIN_NUM 编码）
```

Linux 用户态完整工程见附录 C。

## 编译运行 (PC 版)

```bash
cd pc
make
./demo
```

预期输出: LED 和 Motor 的 init 都会先打印 `[base] Pinx common init done`，再打印 `[LED] / [motor]` 子类 init。最后通过 `led_base_get_pin(&me->base)` 共享一份基类 API。
