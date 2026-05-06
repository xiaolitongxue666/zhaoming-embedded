# 怎么参与

这个仓库是一本永久免费在线书 + 它的配套代码。欢迎贡献。

## 报告错误

到 [Gitee Issues](https://gitee.com/zhao-chengbo/zhaoming_embedded/issues) 或 [GitHub Issues](https://github.com/ZhaoChengBo/zhaoming-embedded/issues) 开 Issue。

附上章节 / 文件 / 行号，期望和实际，编译环境（`gcc -v` 第一行）。代码包跑不通的优先处理。

## 改进建议

某一章讲得不够透、例子不典型、类比反而让人糊涂，开 Issue 说出来，具体到段落最好。

## 提 PR

Fork，新建分支，提 PR。提之前先开 Issue 说一下方向，避免撞车。

代码包要满足：

- `gcc -Wall -Wextra` 0 警告 0 错
- `./demo` 正常 return 0
- 跨章节 struct 字段名前后一致
- main.c 末尾加 `printf("\nPress Enter to exit...\n"); getchar();`
- printf 输出纯 ASCII，中文走注释

引用 Linux 内核 / Zephyr / RT-Thread / GObject 等项目要给出文件、行号、tag，例如 `drivers/i2c/busses/i2c-rk3x.c@v6.6:1234`。

## 节奏

PR 一般一周内回。一周没动静在 Issue 里 ping 我。

技术性 PR 就事论事。文风类 PR 可能整段重写以保持全书一致。
