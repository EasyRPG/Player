# EasyRPG Player 联机版

[English](../README.md)

多人联机的 C++ 实现，通过此项目可以快速的获得客户端和服务端


## 经常会考虑到的问题

### 怎么样使用 podman 来创建服务器？

尽管该项目使用了 podman，但是你依然可以使用 docker

```
# 进入该项目目录
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

### 如何在 macOS 上运行？

该项目可以在 macOS 上编译和运行，但是 Opus 无法启用

```
# 安装 liblcf 的所有依赖
brew install expat icu4c

# 安装 Player 的所有依赖
brew install libpng libvorbis sdl2 sdl2_mixer pixman freetype

# 编译
ICU_ROOT=/opt/homebrew/opt/icu4c cmake -B build -DPLAYER_BUILD_LIBS=on -DCMAKE_BUILD_TYPE=Debug -DPLAYER_WITH_OPUS=off
cmake --build build

# 如果遇到问题可以恢复到以前的状态
brew unlink icu4c
```

### 如何在 Windows 上运行？

你可以从这里获得预编译二进制：

https://github.com/monokotech/EasyRPG-Multiplayer-Native/releases

找到 `Windows-Build-*.zip` 下载并保存，压缩档里面只有 Player.exe 是你需要的
，将它复制到游戏目录

### 如何使用多语言？

下载 ynotranslations 的 master.zip，然后解压缩：

https://github.com/ynoproject/ynotranslations/archive/refs/heads/master.zip

从 ynotranslations 当中找到你想要的语言，然后将其复制到游戏目录再改名为 `Language`
，之后的菜单会出现新的条目，进入然后选择语言

如果想锁定语言，可以传入 --language \<名字\> 给 Player，这个名字你可以从 Language
目录中找到

### 什么是游戏目录？

游戏目录当中会有很多 .lmu 文件，而且可能还有 RPG\_RT.exe

### 如何限制帧数？

如果要限制帧数，首先得关闭 V-Sync

对于更老的笔记本，限制帧数可以最大程度的降低风扇噪音

按下 F1 -> 将 V-Sync 改为 Off -> 设置 Frame Limter 为 20

### 为什么无法保存设置？

在保存设置的时候不会有任何反应，但是终端会输出一条带有 config.ini 路径的消息
，实际上，设置已经保存

同时，联机设置也已经保存到了 config.ini，也就是说，你可以不带参数的使用客户端命令

### 是否有静音按键?

有的，你可以按 M 键切换静音


## 需求

### 必须的

- [liblcf] 用于 RPG Maker 的数据读取
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


---
[致谢 & 许可](../README.md#credits)
