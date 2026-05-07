# ch20-leds-status

Linux platform driver demo·配套在线书 ch20.8。

## 文件

| 文件 | 用途 |
|------|------|
| `leds-status.c` | 内核模块主文件·三件套：父类首字段 / container_of 反推 / module_platform_driver |
| `leds-status-overlay.dts` | dts overlay·给运行系统加一个 `compatible = "status-led"` 节点 |
| `Makefile` | 树外构建·`make` 编出 `leds-status.ko` |

## 前置条件

```bash
sudo apt install raspberrypi-kernel-headers device-tree-compiler
```

## 编 + 装

```bash
# 1) 编模块
make

# 2) 编 overlay 二进制
dtc -@ -I dts -O dtb -o leds-status.dtbo leds-status-overlay.dts

# 3) 加载 overlay (runtime·Pi 4 内核 6.x)
sudo dtoverlay leds-status.dtbo

# 4) 加载模块
sudo insmod leds-status.ko

# 5) 验证 sysfs 节点
ls /sys/class/leds/status-led/

# 6) 点灯
echo 1 | sudo tee /sys/class/leds/status-led/brightness
echo 0 | sudo tee /sys/class/leds/status-led/brightness
```

## 卸载

```bash
sudo rmmod leds-status
sudo dtoverlay -r leds-status-overlay
make clean
```

## 教学呼应

| 章节 | 主题 | 文件里看哪 |
|------|------|------------|
| ch12 | 父类作首字段·向上转型 | `struct status_led_data` 第一字段 `cdev` |
| ch13 | container_of 反推 | `status_led_brightness_set` 第一行 |
| ch14 | ops 表·虚函数槽位 | `led->cdev.brightness_set = ...` |
| ch17 | initcall 链路 | `module_platform_driver(status_led_driver)` |
| ch18 | of_match_table·dts 解耦 | `status_led_of_match[].compatible` 和 dts 对上 |
