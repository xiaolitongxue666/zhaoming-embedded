# 02-static-hiding — 锁后厨 + 标 private 字段

第 2 章 [同事改了一行 LED 全乱了](../../../book/01-封装/02-同事改了一行.md) 的配套代码。

## 目录结构

```
02-static-hiding/
├── pc/                 完整可跑的 PC 模拟版（gcc 一句编译）
└── platform-mcu/
    └── stm32/          STM32 真机版（用 PIN_NUM 编码）
```

## 教学要点

ch01 的 `struct led` 字段公开但没有任何标记，外部 `me->pin = 999` 既
没人拦也没人骂。ch02 在 ch01 基础上加三件事：

1. **字段挂 `/* private */` 注释** — 字段还在 `led.h` 公开（继承机制
   层需要看到字段，下章 ch06 起反复用），但加了注释明示外部别直接写
2. **内部工具加 `static`** — `led.c` 里的 `update_hardware` /
   `brightness_valid` 加 `static`，链接器只把它们写成 file-local 符号，
   外部 `.c` 文件根本看不到名字
3. **`led_get_state` API** — 外部要读字段也走 API，不直接读 `me->is_on`

主流 C 项目（nginx / Redis / LVGL / FreeRTOS / Linux 内核大部分驱动）
就是这套写法：字段公开 + 命名纪律 + 内部工具 `static`。

字段藏 `.c` 的方案叫不透明指针（`FILE *` / `pthread_t` / `sqlite3 *`），
适合跨二进制库 ABI 边界，本书 OOP 主线 18 章不用：继承时子类必须
看到父类字段才能 embed。

## 编译运行（PC 版）

```bash
cd pc
make
./demo
```

期待输出：两颗 LED 各做一遍 on/off/toggle/set_brightness/get_state，
out-of-range 亮度被 API 拒绝（返回 -2），最后清理。

## 试两层强度

main.c 里有两段被注释的代码：

```c
/* red.pin = 999;          软 private — 编译能过，靠纪律 + review 拦 */
/* update_hardware(&red);  硬 private — 链接器报 undefined reference */
```

取消第一行，编译过，运行也过：这是 C 软 private 的边界。
取消第二行，链接器直接拒绝：这是 `static` 给的硬锁。

理解这两层差别，就理解了 C 圈子的工程现实。

## STM32 完整工程

`platform-mcu/stm32/led_stm32.c` 是片段，需要套到 STM32CubeMX 工程里编译。完整跑通的 STM32 工程见 [附录 B](../../../book/附录/B-STM32完整工程.md)。
