# 兆鸣嵌入式 · 开源内容仓库

公众号「兆鸣嵌入式」。这里是我把工程经验整理出来公开的地方。第一本书《C 语言面向对象编程·嵌入式实战》已经全部发布。

## 已发布

### 《C 语言面向对象编程·嵌入式实战》

[**在线阅读：zhaochengbo.github.io/zhaoming-embedded**](https://zhaochengbo.github.io/zhaoming-embedded/)

从一颗 LED 写到 Linux 内核 4000 万行代码，讲清楚封装、继承、多态在 C 里怎么落到工业代码。

- 20 章正文 + 4 附录 + 序曲 + 前言 + 尾声
- 每章配套独立可编译的 PC 代码包，全部 0 警告 exit=0
- 附录 B / C 是完整的 STM32 + Linux 工业级工程（PC mock 跑通）
- 配套 19 期 B 站视频，搜「兆鸣嵌入式」

永久免费在线·MIT License·不出版纸质书。

## 仓库结构

```
zhaoming-embedded/
├── book/                  在线书源码 (markdown)
├── oop-in-c/              第一本书每章配套代码包
├── practice/              个人手写练习区（学习计划见 practice/LEARNING_PLAN.md）
├── industrial/            工业代码：Platform 层抽象、电机驱动骨架、完整 STM32 / Linux 工程
├── ...                    后续主题独立目录
└── README.md              本文件
```

## 跑代码

国内 Gitee 镜像，速度快：

```bash
git clone https://gitee.com/zhao-chengbo/zhaoming_embedded.git
cd zhaoming_embedded/oop-in-c/code/01-three-leds/pc
make
./demo
```

也可以用 GitHub：

```bash
git clone https://github.com/ZhaoChengBo/zhaoming-embedded.git
```

Windows 装 MinGW，Linux `sudo apt install gcc make`。Windows 不想装环境，双击 `oop-in-c/code/01-three-leds/pc/demo.exe` 也能跑。

## 关注作者

| 平台 | 信息 |
|---|---|
| 公众号 | **兆鸣嵌入式** |
| 个人微信 | **zmqrs001** |
| GitHub | [github.com/ZhaoChengBo/zhaoming-embedded](https://github.com/ZhaoChengBo/zhaoming-embedded) |
| Gitee | [gitee.com/zhao-chengbo/zhaoming_embedded](https://gitee.com/zhao-chengbo/zhaoming_embedded) |
| 抖音 | 搜「兆鸣嵌入式」 |
| B 站 | 搜「兆鸣嵌入式」 |
| 视频号 | 搜「兆鸣嵌入式」 |

公众号回复「交流」加技术群。

<img src="book/assets/zhaoming_qrcode.jpg" alt="兆鸣嵌入式公众号二维码" width="240" />

## 反馈与勘误

发现错误、有改进建议、想贡献一章，到 [GitHub Issues](https://github.com/ZhaoChengBo/zhaoming-embedded/issues) 或 [Gitee Issues](https://gitee.com/zhao-chengbo/zhaoming_embedded/issues) 提一个，附章节、你的理解、你认为的问题。我会回。

## 许可证

[MIT](LICENSE)。自由使用、修改、分享、商用。
