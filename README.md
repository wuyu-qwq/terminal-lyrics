<div align=center>

# Terminal Lyrics

</div>

一个用于在终端中播放ttml逐字歌词的项目，暂时只支持Windows平台

### 编译

本项目使用TDM GCC 9.2构建，若使用其他编译器可能会出现运行效果不一致的情况

命令：

```
g++ main.cpp .\tinyxml2\tinyxml2.cpp -o lyrics
```

可直接使用compile.cmd脚本编译并运行