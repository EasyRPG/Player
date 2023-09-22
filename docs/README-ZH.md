# EasyRPG Player 联机版

[English](../README.md)

多人联机的 C++ 实现，通过此项目可以快速的获得客户端和服务端。

该项目是从另一个项目中对比找出多人联机的部分，然后用 patch 导入的这些代码。所以这是一个基于 EasyRPG/Player 的项目，并从 ynoproject/ynoclient 导入了联机代码，详情请见该项目的首次提交 44347b6 (Initial commit. Add multiplayer changes)。

你可能有这样一个疑问，服务端是基于什么？因为复用了客户端的代码，所以服务端像是一面客户端的镜子（指代码上）。移除了 C2S 和 S2C 的命名空间也将这两种数据包类型合二为一成为双向可用的数据包类型，因此现在的 messages.h 可读性提高了很多。以后可能会尝试使用二进制数据结构来再次重构数据包类，以解决数据中不能包含特定的定界符问题。

如果你对此项目很感兴趣，可以克隆该仓库到本地去浏览代码。如果不知道如何使用 Git 请参考：[Git使用手册](https://git-scm.com/book/zh/v2)

另外很多更新会先提交到 master\_tmp 分支，请注意切换。


## 如何使用

### 开始之前

如果你需要使用该程序，但是不能编译，请转到：[Releases](https://github.com/monokotech/EasyRPG-Multiplayer-Native/releases) 下载预编译二进制，你需要选择与自己目前使用的系统合适的构架去下载，例如你需要区分是 x86\_64 或是 aarch64，通常来说，Android手机 选择 arm64-v8a，旧的 Android手机 选择 armeabi-v7a，最新的苹果电脑或笔记本则是选择 arm64。

打开终端，windows 是 cmd.exe，macOS 则是 Terminal.app

然后用文件管理器拖拽游戏文件夹到终端（[什么是游戏文件夹？请看下文的 FAQ](#什么是游戏文件夹)）

在文件夹路径前添加 `cd `，cmd.exe 则是 `cd /d `（/d 是同时切换驱动器的意思）进入该文件夹

此时需要找到 Player 的位置

然后运行 `<绝对路径>/Player.exe` 或是 macOS 的 `<绝对路径>/EasyRPG Player.app/Contents/MacOS/EasyRPG\ Player`

首次运行会全屏显示（这是 EasyRPG Player 的默认行为）

如果你不喜欢，可以按 F1 选择 Video -> Fullscreen 为 OFF

最后返回按一次 \<Save Settings\>，再返回到菜单

之后可以按 F10 调出聊天界面，F9 可以切换通知（这是一个在 Player 内的通知层）

### 客户端命令

`!help`

调出使用方法

`!server [on, off]` （别名 !srv）

通过此命令可以运行服务器

也就是说只要能运行 Player.exe 就可以启动服务器

可以在局域网内（例如同一个 WiFi 或 VPN）启动服务器，不管是在手机上还是电脑上

`!connect [address]` (别名 !c)

通过此命令可以连接服务器

输入内网ip，或是公网ip，也可以是域名来连接服务器

直到状态一直保持 Connnected 才表示可用

`!disconnect` (别名 !d)

断开连接

`!name [text]`

如果设置 !name \<unknown\> 会还原为空字符

`!chat [LOCAL, GLOBAL, CRYPT] [CRYPT Password]`

切换聊天范围

输入 !chat CRYPT [密码] 就会生成一个 256bit 的密钥

此时只有所有使用了相同密码的客户端才可以看到发送的消息

如果当前是 CRYPT 范围，可以换成其它范围，例如 GLOBAL

再次输入 !chat CRYPT 则无需密码，可以切换回来

什么是 CRYPT （加密聊天）[请看下文的 FAQ](#什么是-chat-crypt)

`!log [LOCAL, GLOBAL, CRYPT]`

输入 !log 对应的范围来显示或隐藏这个范围的所有消息

`!immersive` （别名 !imm）

切换沉浸模式

以上命令都需要通过 F1 -> \<Save Settings\> 来记住

!connect, !name, !chat, !immersive 命令的设置都被会保存

### 使用 Podman 或 Docker 来运行服务器

尽管该项目使用了 podman，但是你依然可以使用 docker 去做同样的事

```
# 进入该项目文件夹
cd EasyRPG-Multiplayer-Native

# 创建镜像
podman build --build-arg TAG="$(git describe --tags)" -t epmp_img .

# 创建容器
podman create --name epmp_container -p 6500:6500 epmp_img

# 启动容器
podman restart epmp_container

# 删除容器
podman stop epmp_container && podman rm epmp_container
```

### 使用命令行来运行服务器

Player.exe --server --bind-address 0.0.0.0[:端口号] --config-path /路径/到/文件夹

Player.exe 请根据系统替换成对应的可执行文件

为服务器单独编译的可执行文件

easyrpg-player-server --bind-address 0.0.0.0[:端口号] --config-path /路径/到/文件.ini

### 在 linux 上编译

Arch Linux

```
# 安装编译工具
pacman -S gcc-libs cmake ccache

# 安装 liblcf 的所有依赖
pacman -S expat icu

# 安装 Player 的所有依赖
# 第一行是必须的，第二行是可选的
pacman -S sdl2 pixman libpng zlib fmt
pacman -S harfbuzz mpg123 wildmidi libvorbis opusfile libsndfile libxmp speexdsp fluidsynth

# 编译
# 可以通过调整 `-DBUILD_CLIENT=on -DBUILD_SERVER=off` 来决定是否编译独立的服务器
cmake -B build -DBUILD_CLIENT=on -DBUILD_SERVER=off -DPLAYER_BUILD_LIBS=on -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build build -j${$(getconf _NPROCESSORS_ONLN):-2}
```

### 在 macOS 上编译

```
# 安装 homebrew
请看：https://brew.sh/zh-cn/

# 安装编译工具
brew install cmake ccache

# 安装 liblcf 的所有依赖
brew install expat icu4c

# 安装 Player 的所有依赖
# 第一行是必须的，第二行是可选的
brew install sdl2 pixman libpng zlib fmt
brew install freetype mpg123 wildmidi libvorbis opusfile libsndfile speexdsp

# 编译
ICU_ROOT=$(brew --prefix)/opt/icu4c cmake -B build -DPLAYER_BUILD_LIBS=on -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPLAYER_WITH_OPUS=off -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build build -j${$(getconf _NPROCESSORS_ONLN):-2}
```

### 在 Windows 上编译

请参考：[Guide: How To Build EasyRPG Player on windows](https://community.easyrpg.org/t/guide-how-to-build-easyrpg-player-on-windows/1174)

### 更多编译例子

可以查看 .github/workflows/releases.yml 文件

## FAQ

### 什么是游戏文件夹？

游戏文件夹中会有很多 .lmu 文件，而且可能还有 RPG\_RT.exe

### 什么是 !chat CRYPT

使用端对端加密可以防止聊天消息被中间人捕获，例如服务器，VPN服务器 或 ISP

加密采用的是 AES-256-GCM Authenticated Encryption

密码会先通过 PBKDF2 迭代 600000 次获得 256bit 的密钥

然后密钥再通过 CRC32 获得一个整型传给服务器去匹配设置了相同密码的客户端

此密钥以 Base64 保存到配置文件


实现方法在这里：[chatui.cpp](https://github.com/monokotech/EasyRPG-Multiplayer-Native/blob/9b63310aa53409b5ac1c549c1a4601e5468c05f2/src/multiplayer/chatui.cpp#L881)

### 如何使用多语言？

下载 ynotranslations 的 master.zip，然后解压缩：

https://github.com/ynoproject/ynotranslations/archive/refs/heads/master.zip

从 ynotranslations 当中找到你想要的语言，然后将其复制到游戏文件夹再改名为 `Language`
，重启 Player 之后，菜单会出现新的条目，进入然后选择语言

如果想锁定语言，可以传入 --language \<名字\> 给 Player，这个名字你可以从 Language 文件夹中找到

### 如何限制帧数？

如果要限制帧数，首先得关闭 V-Sync

对于更老的笔记本，限制帧数可以最大程度的降低风扇噪音

按下 F1 -> 将 V-Sync 改为 Off -> 设置 Frame Limter 为 20

然后返回选择 \<Save Settings\>

### 为什么无法保存设置？

在保存设置的时候不会有任何反应，但是终端会输出一条带有 config.ini 路径的消息
，实际上，设置已经保存

同时，联机设置也已经保存到了 config.ini，也就是说，你可以不带参数的使用客户端命令

### 是否有静音按键?

有的，你可以按 M 键切换静音


## 需求

### 必须的

- [liblcf] 用于 RPG Maker 的数据读取
- [sock++] 用于多人联机
- [crypto++] 用于多人联机的聊天
- SDL2 用于控制图像和声音的输出
- Pixman 用于底层像素操作
- libpng 用于 PNG 图像支持
- zlib 用于 XYZ 图像支持
- fmtlib 用于内部日志格式化

### 可选的

- FreeType2 用于字符到点阵图的支持 (+ HarfBuzz 用于 Unicode 支持)
- mpg123 用于 MP3 音频支持
- WildMIDI 用于 MIDI 音频支持
- Libvorbis / Tremor 用于 Ogg Vorbis 音频支持
- opusfile 用于 Opus 音频支持
- libsndfile 用于 WAVE 音频支持
- libxmp 用于 tracker music 支持
- SpeexDSP 用于音频重采样

SDL 1.2 仍然支持，但已过时

[liblcf]: https://github.com/EasyRPG/liblcf
[sock++]: https://github.com/fpagliughi/sockpp
[crypto++]: https://www.cryptopp.com/wiki/Main_Page


---
[致谢 & 许可](../README.md#credits)
