# 兆鸣嵌入式

> 三家世界500强，11年一线嵌入式实战。不讲demo，只讲工程化。

---

## 这个仓库是什么

这是**兆鸣嵌入式**视频教程的配套资源仓库，包含：

- **示例代码** — 每期视频的完整代码，可编译可运行
- **学习文档** — 核心知识点总结（PDF）
- **视觉讲义** — 视频中的slides截图（PNG）
- **工具配置** — Source Insight配置、AI编程Skill等

所有代码都能在**PC上直接编译运行**（GCC），没有开发板也能学。

---

## 目录导航

| 目录 | 内容 | 说明 |
|------|------|------|
| [oop-in-c/](oop-in-c/) | **C语言·一个LED讲透面向对象** | 核心系列课程：从struct到Linux内核 |
| [coding-standards/](coding-standards/) | **嵌入式C语言工程化编码规范** | 7章PDF，覆盖架构/设计模式/Clean Code/内存安全 |
| [ai-coding-skill/](ai-coding-skill/) | **AI编程工具编码规范Skill** | 导入Claude Code/Cursor，AI自动遵循工程化标准 |
| [source-insight/](source-insight/) | **Source Insight 4 配置** | 嵌入式工程师看代码神器，附个人配置文件 |

---

## 快速开始

### 1. 克隆仓库

```bash
git clone https://gitee.com/zhaoming-embedded/zhaoming-embedded.git
```

### 2. 编译运行第一个例子

```bash
cd zhaoming-embedded/oop-in-c/code/EP06_封装
gcc -Wall -Wextra -I../common -o demo main.c led.c ../common/platform_pc.c
./demo
```

### 3. 按顺序学习

EP06 → EP07 → EP08 → EP09 → EP10 → ...

每期代码都是上一期的**增量改进**，同一个LED驱动一步步进化。

---

## 适合谁

- 正在学C语言的**大二/大三学生**
- 想从"能跑就行"进阶到"工程化"的**初级嵌入式工程师**
- 面试准备中，想理解**OOP in C / 函数指针 / container_of**的同学
- 想看看**11年一线经验的工程师是怎么写C代码**的人

---

## 配套视频

- **抖音** 搜索「兆鸣嵌入式」
- **B站** 搜索「兆鸣嵌入式」
- **视频号** 搜索「兆鸣嵌入式」

---

## 获取更多

扫码关注公众号 **「兆鸣嵌入式」**，回复「交流」加技术交流群：

![兆鸣嵌入式公众号](兆鸣嵌入式公众号二维码.jpg)

---

## 许可证

[MIT License](LICENSE) — 自由使用、修改、分享。
