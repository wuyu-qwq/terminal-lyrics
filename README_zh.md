<div align=center>

# Terminal Lyrics

[English](https://github.com/wuyu-qwq/terminal-lyrics/blob/main/README.md) / 简体中文

</div>

## 介绍

一个用于在终端中播放TTML逐字歌词的项目，暂时只支持Windows平台。

本项目支持的TTML逐字歌词遵循评苹果的[TTML歌词文件格式](https://help.apple.com/itc/videoaudioassetguide/#/itcd7579a252)标准。

## 使用

在程序的运行参数中直接指定一个TTML文件，程序会直接播放它，例如：

```
.\lyrics.exe .\music\ME!.ttml
```

你也可以在参数中传入你存放ttml文件的目录，例如：

```
.\lyrics.exe .\music
```

此时程序会进入文件选择阶段，你需要使用键盘上的按键来选择你需要播放的歌曲。

**如果你在运行时不传入任何参数，程序会默认使用当前目录下的`music`目录当作ttml文件目录**

### 按键说明：

- `PageUp`/`左箭头`/`上箭头`：上一页

- `PageDown`/`右箭头`/`下箭头`：下一页

- `Home`：跳转到第一页

- `End`：跳转到最后一页

- `数字0-9`：选择文件

## 编译

命令：

```
g++ main.cpp .\tinyxml2\tinyxml2.cpp -o lyrics
```

可直接使用`compile.cmd`脚本编译并运行。

## 使用的第三方库

- [tinyxml2](https://github.com/leethomason/tinyxml2)

## 许可证

[MIT 许可证](https://github.com/wuyu-qwq/terminal-lyrics/blob/main/LICENSE)