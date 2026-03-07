#include "main.hpp"

int main(int argc, char *argv[]) {
	#if defined (_WIN32)
		SetConsoleOutputCP(65001);    
		
		int argcW;
		LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argcW);
		if (argcW <= 1) {
			std::cout << "Please input the ttml file path in argument!";
			return -1;
		}
		launch(argvW[1]);
	#elif defined (__linux__)
		if (argc <= 1) {
			std::cout << "Please input the ttml file path in argument!";
			return -1;
		}
		launch(argv[1]);
	#endif
}