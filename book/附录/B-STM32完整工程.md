# 附录 B · Zephyr 完整工程 · stm32f4_disco

## B.0 关于这一份附录

附录 B 给一份 Zephyr v3.7.0 LTS 上跑的完整参考工程。读者照着步骤走、5 分钟把 4 颗板载 LED 点起来，亲手验证前 18 章的抽象（`struct device` + ops 表 + container_of + initcall + platform）在工业级开源 RTOS 里字节级落地。配套代码对照 Zephyr v3.7.0 LTS 上游写，参照 Zephyr CI 已跑通的 `stm32f4_disco` 板，读者按附录步骤应当原样跑通，有出入欢迎到 GitHub 提 issue。

ch19 给"叙事教学，把前 18 章抽象在 Zephyr 源码里指出来"，附录 B 给"参考工程，照着 5 分钟跑通"。两份正交，读完 ch19 回头跑附录 B，完整闭环。

## B.1 5 分钟跑通

### 环境前提

- Zephyr v3.7.0 LTS
- Zephyr SDK v0.16.x（含 `arm-zephyr-eabi` GCC 工具链）
- west（Zephyr 的 meta-tool · Python 写）
- ST-Link USB 驱动 + ST-Link 命令行工具

环境安装步骤本附录不复述，一切以 Zephyr 官方 Getting Started 文档为准：

`https://docs.zephyrproject.org/3.7.0/develop/getting_started/index.html`

Windows 用户建议走 WSL2 安装 SDK + west，绕开 native Windows 路径和权限的若干历史问题。

### 命令清单

```bash
# 1. 装 west
pip install west

# 2. init zephyr workspace
west init -m https://github.com/zephyrproject-rtos/zephyr --mr v3.7.0 ~/zephyrproject
cd ~/zephyrproject && west update

# 3. 装 SDK
west sdk install

# 4. clone 本书配套代码
git clone https://github.com/ZhaoChengBo/zhaoming-embedded.git ~/zhaoming-embedded

# 5. build demo 1
cd ~/zhaoming-embedded/industrial-zephyr
west build -b stm32f4_disco -p auto -- -DDEMO=1

# 6. flash
west flash
```

期望结果：4 颗板载 LED 跑马灯，依次点亮 `PD12 → PD13 → PD14 → PD15` 再灭。

## B.2 工程目录结构

```
industrial-zephyr/
├── README.md
├── CMakeLists.txt
├── prj.conf
├── boards/
│   └── stm32f4_disco.overlay
└── src/
    ├── main_demo1_4led.c          # 4 颗 LED 同 ops 表跑马灯
    ├── main_demo2_overlay.c       # overlay 改 dts label
    ├── main_demo3_container.c     # CONTAINER_OF 反推 data 地址
    └── main_demo4_enosys.c        # 可空 ops + -ENOSYS 兜底
```

工程是 Zephyr 文档里讲的 freestanding application 形态：自带 `CMakeLists.txt` 和 `prj.conf`，靠环境变量 `ZEPHYR_BASE` 找读者本地的 zephyr/ 源，不依赖 west workspace 的特定布局。

## B.3 关键文件解读

### B.3.1 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(industrial_zephyr)

# 通过 -DDEMO=N 切换 4 个 demo
if(DEMO EQUAL 1)
    target_sources(app PRIVATE src/main_demo1_4led.c)
elseif(DEMO EQUAL 2)
    target_sources(app PRIVATE src/main_demo2_overlay.c)
elseif(DEMO EQUAL 3)
    target_sources(app PRIVATE src/main_demo3_container.c)
elseif(DEMO EQUAL 4)
    target_sources(app PRIVATE src/main_demo4_enosys.c)
endif()
```

`find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})` 这一行告诉 CMake：去 `ZEPHYR_BASE` 环境变量指向的目录拿 Zephyr 全套构建脚本。读者 `west build` 之前，`west` 会自动设置好这个变量。这就是 Zephyr 文档里讲的 freestanding application 模式，不依赖 west workspace 的特定布局。

`-DDEMO=N` 是给本书加的 demo 切换开关，一份 prj.conf 编 4 个不同入口，避免每个 demo 维护一份独立工程。

### B.3.2 prj.conf

```
CONFIG_LED=y
CONFIG_LED_GPIO=y
CONFIG_GPIO=y
CONFIG_LOG=y
CONFIG_PRINTK=y
```

实际文件还有 SERIAL / UART_CONSOLE / MAIN_STACK_SIZE 三行，见仓库。

逐行作用：

- `CONFIG_LED=y`：开 LED subsystem，把 `include/zephyr/drivers/led.h` 的公开 dispatch 编进固件
- `CONFIG_LED_GPIO=y`：开 led_gpio.c 子类驱动，这一行漏了的话 dts 里 `compatible = "gpio-leds"` 节点不会被任何驱动认领，`device_is_ready` 永远返回 false
- `CONFIG_GPIO=y`：开底层 GPIO subsystem，led_gpio 子类内部调 `gpio_pin_set_dt` 要靠这一行
- `CONFIG_LOG=y`：开 logging 框架，驱动里的 `LOG_ERR` / `LOG_INF` 才能输出到 UART
- `CONFIG_PRINTK=y`：开 printk，demo 代码用 printk 打回调地址、错误码

### B.3.3 stm32f4_disco.overlay（demo 2 用）

```dts
&green_led_4 {
    label = "Demo Board LD4 (overlay)";
};
```

这 3 行做的事：找到板自带 dts 里 nodelabel 为 `green_led_4` 的节点，把它的 `label` 属性覆盖成新值。Zephyr 的 dts overlay 不修改 board 自带文件，而是叠加一份补丁，`west build` 自动合并。这是给应用做硬件改动的推荐方式，换板的时候只改这一份就够。

### B.3.4 main_demo1_4led.c

```c
#include <zephyr/kernel.h>
#include <zephyr/drivers/led.h>

#define LED_NODE  DT_NODELABEL(leds)

int main(void)
{
    const struct device *led_dev = DEVICE_DT_GET(LED_NODE);

    if (!device_is_ready(led_dev)) {
        return -1;
    }
    while (1) {
        for (int i = 0; i < 4; i++) {
            led_on(led_dev, i);
            k_msleep(200);
            led_off(led_dev, i);
        }
    }
    return 0;
}
```

逐段解读：

- `DT_NODELABEL(leds)`：编译期从 dts 拿到 `leds { compatible = "gpio-leds"; ... }` 这个节点
- `DEVICE_DT_GET(LED_NODE)`：拿到这个节点对应的 `struct device *` 句柄，Zephyr 在编译期把所有 device 实例放进 ROM，这里只是取地址
- `device_is_ready` 必查，`DEVICE_DT_GET` 拿到的指针只代表这个设备在 ROM 里有，init 函数可能因为底层 GPIO 没就绪而失败
- `led_on` / `led_off` 是 `include/zephyr/drivers/led.h` 的公开 dispatch，应用层不知道底下挂的是 led_gpio 子类还是 led_pwm 子类，这就是 ch12 讲过的"向上转型"威力的工程化表达

源参考：

- `https://github.com/zephyrproject-rtos/zephyr/blob/v3.7.0/include/zephyr/drivers/led.h`
- `https://github.com/zephyrproject-rtos/zephyr/blob/v3.7.0/drivers/led/led_gpio.c`
- `https://github.com/zephyrproject-rtos/zephyr/blob/v3.7.0/boards/st/stm32f4_disco/stm32f4_disco.dts`

## B.4 4 个 demo 的 build / flash / 期望输出

| Demo | west build 命令 | 期望输出 |
|---|---|---|
| 1 · 4LED 跑马灯 | `west build -b stm32f4_disco -p auto -- -DDEMO=1` | PD12 → PD13 → PD14 → PD15 依次亮灭 |
| 2 · overlay | `west build -b stm32f4_disco -p auto -- -DDEMO=2 -DEXTRA_DTC_OVERLAY_FILE=boards/stm32f4_disco.overlay` | UART 打印 label 已被覆盖为 `Demo Board LD4 (overlay)`·LD4 单灯闪 |
| 3 · CONTAINER_OF | `west build -b stm32f4_disco -p auto -- -DDEMO=3` | UART 打印回调中由 `CONTAINER_OF` 反推到的 data 地址 |
| 4 · enosys | `west build -b stm32f4_disco -p auto -- -DDEMO=4` | UART 打印 `led_blink returned -88 (-ENOSYS)` 然后 `led_set_brightness returned 0` |

### Demo 1 · 4 颗 LED 共用一份 ops 表

dts 里 4 颗 LED 全在 `gpio-leds` 节点下，`led_gpio.c` 编译期通过 `DT_INST_FOREACH_CHILD_SEP_VARGS` 把 4 颗 GPIO spec 收进同一份 `led_gpio_config`，一份 ops 表（`led_gpio_api`）服务 4 颗 LED。应用层只调 `led_on(led_dev, i)`，靠下标 i 选 LED，这就是 ch12 讲的"子类只写一次、实例多份"的工业级做法。

### Demo 2 · overlay 换 label 不改源码

应用 `main_demo2_overlay.c` 里硬编码读 `LD4` 的 dts label，跑出来打印的字符串是 overlay 改后的新值。读者动手把 overlay 注释掉重 build，打印就回到板子原 label。验证 ch15 platform 层的核心命题：硬件描述外置，应用层零修改。

### Demo 3 · CONTAINER_OF 反推 data 地址

demo 用 k_timer 周期回调演示同款 CONTAINER_OF 反推机制，因为 stm32f4_disco 板载没合适的用户按钮触发 GPIO 中断（详见源文件注释）。回调函数收到的 `struct k_timer *t` 是嵌在某个更大的 data 结构里的成员，回调里用 `CONTAINER_OF(t, struct app_timer_ctx, timer)` 反推出外层 data 地址，和注册时打印的地址比对。这就是 ch13 + ch18 讲过的"成员指针反推宿主对象"机制，Linux 内核和 Zephyr 中断回调到处都在用。`CONTAINER_OF` 宏定义在 `include/zephyr/sys/util.h`：

```c
#define CONTAINER_OF(ptr, type, field)                               \
    ({                                                               \
        CONTAINER_OF_VALIDATE(ptr, type, field)                      \
        ((type *)(((char *)(ptr)) - offsetof(type, field)));         \
    })
```

`https://github.com/zephyrproject-rtos/zephyr/blob/v3.7.0/include/zephyr/sys/util.h`

### Demo 4 · 可空 ops + -ENOSYS

`led_gpio_api` 只挂了 `on` / `off` / `set_brightness` 三个函数，`blink` 在 ops 表里是 NULL。demo 应用层故意调 `led_blink`，公开 dispatch 看见 `api->blink == NULL` 就返回 `-ENOSYS`（值是 -88），应用层打印这个错误码并继续调 `led_set_brightness` 验证子类支持的能力照常工作。这是 ch16 讲过的"mandatory + optional 两层 ops"设计，比 C++ 纯虚更柔软：父类提供 NULL 检查 + 错误码兜底，子类没实现的能力调到也不会崩。

## B.5 跨板移植清单

学生想把这套 demo 跑到别的板（nucleo_f401re / nucleo_f411re / black_f407ve 等），只要改 3 处：

1. `west build -b <new_board>` 一个 token，别的不动
2. 检查目标板自带 dts 里有没有 `leds { compatible = "gpio-leds"; ... }` 节点，没有就在 `boards/<new_board>.overlay` 里加一份
3. `aliases { led0 / led1 / led2 / led3 }` 是否齐 4 个，不齐就 overlay 补，应用层代码一字不改

board 列表查询：`https://docs.zephyrproject.org/3.7.0/boards/index.html`

## B.6 常见坑

- **SDK 版本对不上**：v3.7.0 必须配 SDK v0.16.x，v0.17 不向后兼容，会在 link 阶段报符号缺失
- **`west flash` 找不到 runner**：默认 runner 是 openocd，没装 openocd 用 `west flash --runner stlink` 切到 ST-Link 命令行
- **`device_is_ready` 检查跳过**：`DEVICE_DT_GET` 拿到指针不代表 init 成功，init 函数可能因为底层 GPIO 没就绪而 return 错误码，跳过这步检查后续 `led_on` 行为未定义
- **`CONFIG_LED_GPIO=y` 漏了**：dts 里 `compatible = "gpio-leds"` 节点没有任何驱动认领，`DEVICE_DT_GET` 编译能过，`device_is_ready` 运行时永远 false，容易误判成"板子坏了"
- **overlay 改完没 pristine 重 build**：CMake 缓存会让 dts 改动不生效，overlay 改后必须 `west build -p auto`，让 build 系统从干净状态重新生成 `devicetree_generated.h`
- **Windows native 装 SDK 路径有空格**：SDK 安装路径里有空格或中文，west 调 GCC 时会炸，建议走 WSL2 装 SDK + west，路径全走 Linux 风格
- **`-DEXTRA_DTC_OVERLAY_FILE` 路径写错**：路径相对工程根目录，写绝对路径也行，写错时 build 不报错但 overlay 不生效，灯还是原 label

## B.7 这一份附录在全书的位置

ch19 负责叙事教学，按"前 18 章某抽象 → Zephyr 源里在哪 → 字节级对应"四步走，读者读完知道 `struct device` 就是 ch11 的父类、`DEVICE_DT_DEFINE` 就是 ch17 initcall 的升级版。附录 B 负责动手验证，读者照着 5 分钟跑通，亲眼看见 4 颗 LED 共用一份 ops 表、overlay 改 dts 不动 app、CONTAINER_OF 抓回调、可空 ops 走 -ENOSYS。两份配对，读完 ch19 回头跑附录 B，完整闭环。
