# 环境配置

这本书所有代码是纯 C 加 GCC 加 make，没有任何第三方依赖。

## Windows

推荐 MSYS2，最现代、最完整、和 Linux 体验一致。

搜索引擎搜 MSYS2 官网下载 `msys2-x86_64-*.exe`，默认安装到 `C:\msys64`。启动 MSYS2 MinGW 64-bit 终端：

```bash
pacman -Syu                                    # 更新，首次会要求重启终端
pacman -S mingw-w64-x86_64-gcc make            # 装 GCC + make
```

把 `C:\msys64\mingw64\bin` 加到系统 PATH，普通 cmd / PowerShell 也能用。验证：

```cmd
gcc --version
make --version
```

备选方案 MinGW，更轻量但旧。搜索引擎搜 MinGW Installer 安装，只勾 `mingw32-base`，PATH 加 `C:\MinGW\bin`。`make` 在 MinGW 里叫 `mingw32-make`，可手动改名或建别名。

只是想跑跑 demo 的，每个 EP 代码包都附带预编译好的 `demo.exe`，双击即可运行。

## macOS

```bash
xcode-select --install     # 装 Apple Clang，GCC 兼容
gcc --version
```

想用真正的 GNU GCC：

```bash
brew install gcc make
gcc-13 --version           # Homebrew GCC 名字带版本号
```

## Linux

Debian / Ubuntu：

```bash
sudo apt update
sudo apt install -y gcc make git
gcc --version
```

RHEL / CentOS / Fedora：

```bash
sudo yum install -y gcc make git
# 或新版：sudo dnf install -y gcc make git
gcc --version
```

Arch / Manjaro：

```bash
sudo pacman -S base-devel git
```

## WSL

Windows 用户也可以用 WSL2 加 Ubuntu，体验和 Linux 一致，不用折腾 MSYS2。PowerShell 管理员：

```powershell
wsl --install -d Ubuntu
```

进入 Ubuntu 后按 Linux 方式装 GCC。

## 验证

```bash
git clone https://gitee.com/zhao-chengbo/zhaoming_embedded.git
cd zhaoming_embedded/oop-in-c/code/EP06_封装
make
./demo
```

看到一连串 `[GPIO] Pin13 -> HIGH (ON)` 之类的输出，环境就 OK 了。

## 常见问题

| 报错 | 原因 | 解决 |
|---|---|---|
| `gcc: command not found` | GCC 没装或不在 PATH | 见上文对应平台的安装步骤 |
| `make: command not found` | make 没装，Windows 常见 | MSYS2 装 `make`，MinGW 用 `mingw32-make` |
| `fatal error: stdio.h: No such file` | GCC 装得不全，缺 base 包 | 重装并选完整开发套件 |
| 中文路径报错 | 终端编码问题 | 把仓库克隆到纯英文路径 `D:\code\zhaoming` |
| WSL 里访问 Windows 文件超慢 | WSL2 跨文件系统性能差 | 仓库放在 WSL 自己的 `~/` 目录下 |

还有问题到 [Gitee Issues](https://gitee.com/zhao-chengbo/zhaoming_embedded/issues) 提一个。
