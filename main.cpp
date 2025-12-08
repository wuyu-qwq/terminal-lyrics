#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <io.h>
#include <conio.h>
#include <fstream>
#include "tinyxml2/tinyxml2.h"

// 输出ttml文件列表的一页
// 无页码合法性检验
void outPage(std::vector<std::string>* files, unsigned short pages) {
	--pages;
	system("cls");
	for (unsigned short s=pages*10; s<pages*10+10; ++s) std::cout << s-pages*10 << ' ' << (*files)[s] << std::endl;
	std::cout << "\nPage:" << pages+1 << " / " << (*files).size()/10+1 << std::endl;
}

// 时间字符串解析函数
double parseTime(const char* timeStr) {
    int minutes = 0;
    int seconds = 0;
    int milliseconds = 0;
    
    // 解析时间字符串
    if (sscanf(timeStr, "%d:%d.%d", &minutes, &seconds, &milliseconds) == 3) {
        return minutes * 60.0 + seconds + milliseconds / 1000.0;
    }
    // 兼容处理可能存在的格式变体
    if (sscanf(timeStr, "%d:%d.%d", &minutes, &seconds, &milliseconds) == 3) {
        return minutes * 60.0 + seconds + milliseconds / 1000.0;
    }
    return 0.0; // 解析失败返回0
}

// 读取文件内容到字符串
std::string readFileContent(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return "";
    }
    
    // 获取文件大小
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // 分配内存并读取内容
    std::string content(size, '\0');
    file.read(&content[0], size);
    file.close();
    
    return content;
}

// 移动控制台光标
void gotoxy(int x, int y) {
	COORD pos;
	pos.X = x, pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

int main() {
	struct CharInfo {
	    double startTime;
	    double endTime;
	    const char* character;
	};
	short posy;
    std::string filename;
    std::vector<CharInfo> lyrics;
    
	// 寻找ttml文件
	struct _finddata_t fileInfo;
	std::vector<std::string> ttmlFiles;
	intptr_t handle = _findfirst(".\\music\\*.ttml", &fileInfo);
	if (handle != -1L) do ttmlFiles.push_back(fileInfo.name); while (_findnext(handle, &fileInfo) == 0); else {
		std::cout << "没有找到ttml文件，请先将ttml歌词文件放到music目录" << std::endl;
		return 0;
	}
	//for (auto b = ttmlFiles.begin(); b != ttmlFiles.end(); ++b) std::cout << *b << std::endl;
	_findclose(handle);
	
	// 选择ttml文件
	unsigned short pages = 1;
	outPage(&ttmlFiles, 1);
	while (1) {
		if (_kbhit()) {
			unsigned short key = _getch();
			// 按下PageDown、右箭头、下箭头时翻页
			if (key == 224){
				key = _getch();
				// 按下PageDown、右箭头、下箭头时下翻一页
				if ((key==80 || key==81 || key==77) && pages*10<ttmlFiles.size()) outPage(&ttmlFiles, ++pages);
				// 按下PageUp、左箭头、上箭头时上翻一页
				if ((key==72 || key==73 || key==75) && pages > 1) outPage(&ttmlFiles, --pages);
				// 按下Home键时回到首页
				if (key == 71) outPage(&ttmlFiles, pages=1);
				// 按下End键时跳到尾页
				if (key == 79) outPage(&ttmlFiles, pages=ttmlFiles.size()/10+1);
			} 
			// 按下0到9数字键时选择歌曲
			if (key>=48 && key<=57) {
				if ((pages-1)*10-(48-key)<ttmlFiles.size()) {
					filename = ".\\music\\"+ttmlFiles[(pages-1)*10-(48-key)];
					break;
				}
			}
    	}
    	Sleep(10);
	}
	SetConsoleOutputCP(65001);

	// 读取文件内容
    std::string xmlContent = readFileContent(filename);
    if (xmlContent.empty()) {
        std::cerr << "文件内容为空或读取失败" << std::endl;
        return 1;
    }
    // 解析XML ...
    tinyxml2::XMLDocument doc;
    if (doc.Parse(xmlContent.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "XML解析失败! 错误代码: " << doc.ErrorID() << std::endl;
        return 1;
    }
    tinyxml2::XMLElement* tt = doc.FirstChildElement("tt");
    if (!tt) {
        std::cerr << "未找到根元素 <tt>" << std::endl;
        return 1;
    }
    tinyxml2::XMLElement* body = tt->FirstChildElement("body");
    if (!body) {
        std::cerr << "未找到 <body> 元素" << std::endl;
        return 1;
    }
    tinyxml2::XMLElement* div = body->FirstChildElement("div");
    if (!div) {
        std::cerr << "未找到 <div> 元素" << std::endl;
        return 1;
    }
    // 遍历所有 <p> 段落
    tinyxml2::XMLElement* p = div->FirstChildElement("p");
    while (p) {
        tinyxml2::XMLElement* span = p->FirstChildElement("span");
        bool hasChars = false;
        double start, end;
        while (span) {
            const char* beginAttr = span->Attribute("begin");
            const char* endAttr = span->Attribute("end");
            const char* text = span->GetText();
            if (beginAttr && endAttr && text && strlen(text) > 0) {
                start = parseTime(beginAttr);
                end = parseTime(endAttr);
				lyrics.push_back(CharInfo{start, end, text});
                hasChars = true;
            }
            span = span->NextSiblingElement("span");
        }

        // 如果这个段落有内容，在结尾加一个换行符
        if (hasChars) {
            lyrics.push_back(CharInfo{start, end, "\n"});
        }

        p = p->NextSiblingElement("p");
    }
    
    // 验证输出
    for (std::vector<CharInfo>::iterator iter=lyrics.begin(); iter!=lyrics.end(); ++iter) {
    	std::cout << (*iter).startTime << " " << (*iter).endTime << " " << (*iter).character << std::endl;
	}

    std::cout << "\n\n🎵 播放完成！" << std::endl;

    return 0;
}