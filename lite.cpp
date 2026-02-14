#include "main.hpp"

int main() {
    SetConsoleOutputCP(65001);
    
    int argcW;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argcW);
    if (argcW <= 1) {
        std::cout << "Please input the ttml file path in argument!";
        return -1;
    }

    launch(argvW[1]);
}