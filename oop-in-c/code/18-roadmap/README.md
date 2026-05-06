# ch18 · 全书地图回顾

配套书章节：[`book/04-工程威力/18-全书地图.md`](../../../book/04-工程威力/18-全书地图.md)

## 看点

`pc/main.c` 在屏幕上 replay 一颗 LED 走过的全部演化阶段：

- Stage 1（ch01）：三份独立函数，复制粘贴
- Stage 2（ch01）：struct + me 指针
- Stage 3（ch06-ch11）：继承 + ops 表 + 多态 dispatch
- Stage 4（ch12-ch15）：向上转型 + 全局句柄 + 板级初始化
- Stage 5（ch17）：链接自动注册（指向 ch17 完整 demo）

不教新概念，只是让读者亲手运行一遍自己走过的路。

## 跑

```
cd pc
make
./demo
```
