#include <conio.h>

#include "main.hpp"

// 输出ttml文件列表的一页
void outPage(std::vector<std::string>& files, unsigned short pages) {
	--pages;
	system("cls");
	for (unsigned short s=pages*10; s<pages*10+10 && s<files.size(); ++s) 
        std::cout << s-pages*10 << ' ' << files[s] << std::endl;
	std::cout << "\nPage:" << pages+1 << " / " << files.size()/10+1 << std::endl;
}

int main(int argc, char* argv[]) {
    SetConsoleOutputCP(65001);
    std::string filepath = argc>1 ? argv[1] : ".\\music";

    // 判断输入参数是文件还是目录
	if (std::filesystem::exists(filepath)) {
        if (std::filesystem::is_directory(filepath)) {
            //寻找ttml文件
            std::vector<std::string> ttmlFiles;
            for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
                if (entry.is_regular_file() && entry.path().extension() == ".ttml") {
                    ttmlFiles.push_back(entry.path().filename().string());
                }
            }
            if (ttmlFiles.empty()) { std::cout << "该目录下没有ttml文件" << std::endl; return 0; }

            // 选择ttml文件
            unsigned short pages = 1;
            outPage(ttmlFiles, 1);
            while (1) {
                if (_kbhit()) {
                    unsigned short key = _getch();
                    // 按下PageDown、右箭头、下箭头时翻页
                    if (key == 224){
                        key = _getch();
                        // 按下PageDown、右箭头、下箭头时下翻一页
                        if ((key==80 || key==81 || key==77) && pages*10<ttmlFiles.size()) outPage(ttmlFiles, ++pages);
                        // 按下PageUp、左箭头、上箭头时上翻一页
                        if ((key==72 || key==73 || key==75) && pages > 1) outPage(ttmlFiles, --pages);
                        // 按下Home键时回到首页
                        if (key == 71) outPage(ttmlFiles, pages=1);
                        // 按下End键时跳到尾页
                        if (key == 79) outPage(ttmlFiles, pages=ttmlFiles.size()/10+1);
                    }
                    // 按下0到9数字键时选择歌曲
                    if (key>=48 && key<=57) {
                        if ((pages-1)*10-(48-key)<ttmlFiles.size()) {
                            filepath = filepath+"\\"+ttmlFiles[(pages-1)*10-(48-key)];
                            break;
                        }
                    }
                }
                Sleep(FLUSH_INTERVAL);
            }
        }
    } else {
        std::cout << "输入的文件或目录不存在！";
        return -1;
    }

    launch(filepath);
    return 0;
}