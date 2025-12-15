<div align=center>

# Terminal Lyrics

[English](https://github.com/wuyu-qwq/terminal-lyrics/blob/main/README.md) / 简体中文

</div>

### 介绍

一个用于在终端中播放TTML逐字歌词的项目，暂时只支持Windows平台。

本项目支持的TTML逐字歌词遵循评苹果的[TTML歌词文件格式](https://help.apple.com/itc/videoaudioassetguide/#/itcd7579a252)标准。

### 使用

将你准备的`.ttml`文件放入`music`目录中，然后使用按键选择一个文件播放。

按键说明：

- `PageUp`/`左箭头`/`上箭头`：上一页

- `PageDown`/`右箭头`/`下箭头`：下一页

- `Home`：跳转到第一页

- `End`：跳转到最后一页

- `数字0-9`：选择文件

### 编译

本项目使用TDM GCC 9.2构建，若使用其他编译器可能会导致编译出的程序出现问题。

命令：

```
g++ main.cpp .\tinyxml2\tinyxml2.cpp -o lyrics
```

可直接使用compile.cmd脚本编译并运行。

### 使用的第三方库

- [tinyxml2](https://github.com/leethomason/tinyxml2)

### 许可证

[MIT 许可证](https://github.com/wuyu-qwq/terminal-lyrics/blob/main/LICENSE)