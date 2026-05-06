# 怎么参与

这个仓库是一本永久免费在线书 + 它的配套代码。欢迎一切让书更准确、代码更干净的贡献。

## 报告错误

到 [Gitee Issues](https://gitee.com/zhao-chengbo/zhaoming_embedded/issues) 开一个 Issue。

写清楚四件事：

1. 你看的是哪一章 / 哪个代码包 / 哪个文件的第几行
2. 你期望看到什么
3. 你实际看到什么
4. 编译环境（操作系统 + GCC 版本，敲 `gcc -v` 把第一行贴上来即可）

代码包跑不通的 Issue 优先级最高。

## 改进建议

如果你认为某一章讲得不够透、哪个例子不够典型、哪个类比反而让人糊涂，照样开 Issue 说出来。具体到段落和句子最好。

不接受的建议：

- 加挑战题 / 习题 / 课后练习。不是这本书的风格。
- 加更多 emoji 装饰。同上。
- 把章节拆得更细。本书已经按"一章一概念"切到最小粒度，再拆会失去叙事。

## 直接贡献代码或章节

Fork 仓库，新建分支，提 PR。提 PR 之前先开 Issue 说一下你想做什么，避免我们撞车。

代码必须满足：

- `gcc -Wall -Wextra` 0 警告 0 错误
- `./demo` 跑完正常 return 0
- 跨章节 struct 字段名一致。比如 `Led_t` 在 EP06 已经定义了 `pin` 字段，后续章节加 static 隐藏可以，改名不可以。
- main.c 末尾加 `printf("\nPress Enter to exit...\n"); getchar();`，让 Windows 双击的同学能看到完整输出
- printf 输出全部纯 ASCII。中文走注释，不走 stdout，避免 GBK 终端乱码。

章节文档必须满足：

- 朴素工程师的语气。参考标杆是 Linus 的内核邮件、Crafting Interpreters 的散文体。
- 不放营销语言、不放煽情开头、不放鸡汤式金句小标题。
- 引用 Linux 内核给出具体文件 + 行号 + tag，比如 `drivers/i2c/busses/i2c-rk3x.c@v6.6:1234`。
- 引用其它项目（Zephyr / RT-Thread / GObject 等）同上。

## 我审 PR 的节奏

我有正职工作和视频在做，PR 一般 3 - 7 天内回。如果一周没动静，到 Issue 里 ping 我一下。

技术性 PR 我会就事论事改。文风类 PR 我可能整段重写以保持全书一致，不要因此觉得被冒犯。
