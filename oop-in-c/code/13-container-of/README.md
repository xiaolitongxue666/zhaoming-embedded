# ch13 · container_of 的地址魔法 · 向下转型

配套书章节：[`book/04-工程威力/13-container_of.md`](../../../book/04-工程威力/13-container_of.md)

## 看点

- `container_of(ptr, type, member)` 的 PC 友好实现（见 `pc/container_of.h`）
- GPIO 子类故意把 base 放到第二个位置（前面挡了一个 `magic` 字段），证明 container_of 与 base 在 struct 里的位置无关
- 跑出来打印 `offsetof(struct led_gpio, base) = 4`，但每次操作仍能正确还原 magic、pin、on_level

## 跑一遍

```
cd pc
make
./demo
```
