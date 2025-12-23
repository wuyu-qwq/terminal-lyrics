<div align=center>

# Terminal Lyrics

English / [简体中文](https://github.com/wuyu-qwq/terminal-lyrics/blob/main/README_zh.md)

</div>

## Introduction

A project for playing TTML word-by-word lyrics in the terminal. Currently only supports Windows.

The TTML word-by-word lyrics supported by this project follow Apple's [TTML lyrics file format](https://help.apple.com/itc/videoaudioassetguide/#/itcd7579a252) specification.

## Usage

You can directly specify a TTML file as a runtime argument, and the program will play it immediately. For example:

```
.\lyrics.exe .\music\ME!.ttml
```

You can also pass a directory containing TTML files as an argument. For example:

```
.\lyrics.exe .\music
```

In this case, the program will enter a file selection mode, where you can use the keyboard to choose the song you want to play.

**If no arguments are provided at runtime, the program will use the `music` directory under the current working directory as the default TTML file directory.**

### key bindings：

key bindings：

- `PageUp`/`Left Arrow`/`Up Arrow`：previous page

- `PageDown`/`Right Arrow`/`Down Arrow`：next page

- `Home`：jump to the first page

- `End`：jump to the last page

- Number Keys `0-9`：select a file

## Build

Command：

```
g++ main.cpp .\tinyxml2\tinyxml2.cpp -o lyrics
```

You can use the provided `compile.cmd` script to build and run directly.

## Third-Party Libraries

- [tinyxml2](https://github.com/leethomason/tinyxml2)

## License

[MIT LICENSE](https://github.com/wuyu-qwq/terminal-lyrics/blob/main/LICENSE)