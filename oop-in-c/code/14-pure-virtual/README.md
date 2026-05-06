# ch14 · 虚函数不实现会怎样 · 纯虚 / 虚 / 接口

配套书章节：[`book/04-工程威力/14-纯虚与抽象类.md`](../../../book/04-工程威力/14-纯虚与抽象类.md)

## 看点

- `led_on / led_off`：必填（C++ 纯虚函数对应物）。子类 ops 没填，统一接口里 `assert` 失败。
- `led_set_brightness`：选填（C++ 虚函数对应物）。子类没填，统一接口走默认行为，安静返回。
- `sensor_read / calibrate / self_test`：全必填（C++ 接口对应物）。三个 op 全部 assert，少一个就报错。

GPIO 子类故意只填 on / off，演示选填策略。PWM 子类三件套全填。

## 跑

```
cd pc
make
./demo
```
