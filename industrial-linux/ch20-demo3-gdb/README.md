# ch20-demo3-gdb

QEMU + gdb-multiarch 验证 container_of 的偏移量·配套 ch20.10。

## 目的

把 `leds-status.ko` 跑在 QEMU 里·在 `status_led_brightness_set` 入口下断点·用 gdb 看：

1. `led_cdev` 的地址
2. `led` (反推后) 的地址
3. 两者偏移 = `offsetof(struct status_led_data, cdev)` = **0**

因为 `cdev` 是首字段·零偏移·所以两个地址完全相同。这就是书里 ch12 + ch13 反复讲的"父类首字段·向上向下转型零开销"。

## 前置条件

```bash
sudo apt install qemu-system-arm gdb-multiarch
# 或 ARM64
sudo apt install qemu-system-aarch64 gdb-multiarch
```

外加一个能跑 ARM 的内核镜像 + rootfs。两条路：

- **A 路**：buildroot / yocto 自己 cook 一个最小镜像·把 `leds-status.ko` 塞进 `/lib/modules`·再加一段 dts 节点·开机自动 probe。这条路重·但走完一遍受用一辈子。
- **B 路**：直接用 raspbian-lite 的 sd 卡 image·`qemu-system-aarch64 -machine virt -kernel ./Image -drive file=raspbian.img,format=raw -append "root=/dev/vda2 console=ttyAMA0" -nographic`·然后在 guest 里 insmod。这条路糙·能验。

## 步骤

### 1. 启动 QEMU·开 gdb stub

```bash
qemu-system-aarch64 \
    -machine virt -cpu cortex-a72 -smp 4 -m 1G \
    -kernel ./Image \
    -append "root=/dev/vda2 console=ttyAMA0 nokaslr" \
    -drive file=rootfs.img,format=raw \
    -nographic \
    -s -S
```

`-s` = `-gdb tcp::1234`·`-S` = boot 后等 gdb 连上再跑·`nokaslr` 关 KASLR 让符号地址稳定。

### 2. 另一个终端·gdb 连上

```bash
gdb-multiarch ./vmlinux
(gdb) target remote :1234
(gdb) c
```

### 3. 进 guest·加载模块

guest 里：

```bash
insmod leds-status.ko
```

### 4. gdb 下断点

回 gdb·`Ctrl-C` 中断·然后：

```gdb
(gdb) add-symbol-file leds-status.ko 0x<模块加载地址>
(gdb) b status_led_brightness_set
(gdb) c
```

模块加载地址在 guest `cat /sys/module/leds_status/sections/.text` 看。

### 5. 触发回调

guest 里：

```bash
echo 1 > /sys/class/leds/status-led/brightness
```

gdb 命中断点·这时打：

```gdb
(gdb) p led_cdev
$1 = (struct led_classdev *) 0xffff8000xxxxxxxx

(gdb) p led
$2 = (struct status_led_data *) 0xffff8000xxxxxxxx

(gdb) p &led->cdev
$3 = (struct led_classdev *) 0xffff8000xxxxxxxx

(gdb) p (char *)&led->cdev - (char *)led
$4 = 0
```

**三个地址相同·偏移 0**·这就是 container_of 在首字段时退化为指针强转的根本原因。

## 进阶：把 cdev 挪到第二个字段重做

实验性改 `leds-status.c`：

```c
struct status_led_data {
    struct gpio_desc   *gpiod;     /* 故意挪前 */
    struct led_classdev cdev;      /* 不再是首字段 */
};
```

重 build·重做上面 5 步。这次：

```gdb
(gdb) p (char *)&led->cdev - (char *)led
$5 = 8        ← 一个指针的大小·64 位机就是 8
```

`led_cdev` 和 `led` 不再相同·container_of 的减法这时候才**真正生效**。把这个对比讲给学生听·概念瞬间立体。

## 故障排查

- **`No symbol "led_cdev" in current context`**: gdb 没拿到 debug 符号·确认 `make CFLAGS_MODULE=-g` 编模块
- **`add-symbol-file` 找不到模块路径**: 把 .ko 文件拷到 host 当前目录
- **断点没命中**: KASLR 没关·`-append` 加 `nokaslr`
