#include "main.hpp"

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(65001);
    if (argc <= 1) {
        std::cout << "Please input the ttml file path in argument!";
        return -1;
    }

    launch(argv[1]);
}