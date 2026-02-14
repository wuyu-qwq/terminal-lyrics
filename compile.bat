@echo off
:MAIN_MENU
chcp 65001
cls
echo [1] 编译完整版

echo [2] 编译lite
echo [3] 编译发行版
echo.
set /p choice="请输入选项编号: "

if "%choice%"=="1" goto CMD_A
if "%choice%"=="2" goto CMD_B
if "%choice%"=="3" goto CMD_C

echo 无效输入！请重新选择
timeout /t 2 >nul
goto MAIN_MENU

:CMD_A
cls
echo 正在编译完整版
g++ .\main.cpp .\tinyxml2\tinyxml2.cpp -o lyrics
echo 编译完成，请检查报错信息
pause
.\lyrics.exe
goto END

:CMD_B
cls
echo 正在编译lite版
g++ .\lite.cpp .\tinyxml2\tinyxml2.cpp -o lyrics
echo 编译完成，请检查报错信息
goto END

:CMD_C
cls
echo 正在编译完整版
g++ -DNDEBUG -O2 -s .\main.cpp .\tinyxml2\tinyxml2.cpp -o lyrics
echo 正在编译lite版
g++ -DNDEBUG -O2 -s .\lite.cpp .\tinyxml2\tinyxml2.cpp -o lite
echo 编译完成

:END
pause
