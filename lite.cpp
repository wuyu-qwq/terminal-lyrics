#include "main.hpp"

int main(int argc, char *argv[]) {
#if defined (_WIN32)
    SetConsoleOutputCP(65001);    
    
    int argcW;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argcW);
    if (argcW <= 1) {
#elif defined (_linux_)
    if (argc <= 1) {
#endif
        std::cout << "Please input the ttml file path in argument!";
        return -1;
    }


#if defined (_WIN32)
    launch(argvW[1]);
#elif defined (__linux__)
    launch(argv[1]);
#endif
}