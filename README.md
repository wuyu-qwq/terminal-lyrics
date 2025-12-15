<div align=center>

# Terminal Lyrics

English / [简体中文](https://github.com/wuyu-qwq/terminal-lyrics/blob/main/README_zh.md)

</div>

### Introduction

A project for playing TTML word-by-word lyrics in the terminal. Currently only supports Windows.

The TTML word-by-word lyrics supported by this project follow Apple's [TTML lyrics file format](https://help.apple.com/itc/videoaudioassetguide/#/itcd7579a252) specification.

### Usage

Place your prepared `.ttml` files into the `music` directory, then use the keys to select and play a file.

key bindings：

- `PageUp`/`Left Arrow`/`Up Arrow`：previous page

- `PageDown`/`Right Arrow`/`Down Arrow`：next page

- `Home`：jump to the first page

- `End`：jump to the last page

- Number Keys `0-9`：select a file

### Build

This project is built with TDM GCC 9.2. Using other compilers may produce problematic binaries.

Command：

```
g++ main.cpp .\tinyxml2\tinyxml2.cpp -o lyrics
```

You can use the provided `compile.cmd` script to build and run directly.

### Third-Party Libraries

- [tinyxml2](https://github.com/leethomason/tinyxml2)

### License

[MIT LICENSE](https://github.com/wuyu-qwq/terminal-lyrics/blob/main/LICENSE)