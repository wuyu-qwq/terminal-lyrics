#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <io.h>
#include <conio.h>
#include <fstream>

#include "tinyxml2/tinyxml2.h"

#define DSPLINES 15 // 显示行数
#define ROLLINES 10 // 滚动行数
 
#define NORCOLOR "\033[37m" // 常规颜色
#define ACTCOLOR "\033[32m" // 活动颜色
#define FINCOLOR "\033[90m" // 完成颜色
#define COLOREND "\033[0m"  // 颜色结束符

#define FLUSH_INTERVAL 50 // 刷新间隔


struct CharInfo {
    unsigned int startTime;
    unsigned int endTime;
    std::string character;
    std::string color = NORCOLOR;
};

// 移动控制台光标
void gotoxy(int x, int y) {
	COORD pos;
	pos.X = x, pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// 输出ttml文件列表的一页
// 无页码合法性检验
void outPage(std::vector<std::string>& files, unsigned short pages) {
	--pages;
	system("cls");
	for (unsigned short s=pages*10; s<pages*10+10 && s<files.size(); ++s) std::cout << s-pages*10 << ' ' << files[s] << std::endl;
	std::cout << "\nPage:" << pages+1 << " / " << files.size()/10+1 << std::endl;
}

// 输出一个歌词段落
void outLyrics(std::vector<std::vector<CharInfo>>& lyrics, unsigned short idx, bool clear) {
	std::string p;
	for (unsigned short s=idx; s<idx+DSPLINES; ++s) {
		for (std::vector<CharInfo>::iterator iter=lyrics[s].begin(); iter!=lyrics[s].end(); ++iter) {
			p += (*iter).color + (*iter).character + COLOREND;
		}
		p += '\n';
	}
	if (clear) system("cls"); else gotoxy(0, 0);
	std::cout << p;
}
// 时间字符串解析函数
int parseTime(const char* timeStr) {
	unsigned int minutes, seconds, milliseconds;
    if (sscanf(timeStr, "%d:%d.%d", &minutes, &seconds, &milliseconds)==3) return minutes*60000 + seconds*1000 + milliseconds;
    return 0; // 解析失败返回0
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

int main() {
    std::string filename;
    std::vector<std::vector<CharInfo>> lyrics;
    unsigned short topParaIdx;
    unsigned short curIndex;
    
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
					filename = ".\\music\\"+ttmlFiles[(pages-1)*10-(48-key)];
					break;
				}
			}
    	}
    	Sleep(FLUSH_INTERVAL);
	}
	SetConsoleOutputCP(65001);

	// 读取文件内容
    std::string xmlContent = readFileContent(filename);
    if (xmlContent.empty()) {
        std::cerr << "文件内容为空或读取失败" << std::endl;
        return 1;
    }
    // 处理空格
    size_t subpos = xmlContent.find("</span> ");
    while (subpos != std::string::npos) {
    	xmlContent.replace(subpos, 8, " </span>");
    	subpos = xmlContent.find("</span> ", subpos+8);
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
    	std::vector<CharInfo> single;
        tinyxml2::XMLElement* span = p->FirstChildElement("span");
        unsigned int start, end;
        while (span) {
            const char* beginAttr = span->Attribute("begin");
            const char* endAttr = span->Attribute("end");
            const char* text = span->GetText();
            if (beginAttr && endAttr && text && strlen(text) > 0) {
                start = parseTime(beginAttr);
                end = parseTime(endAttr);
				single.push_back(CharInfo{start, end, text});
            }
            span = span->NextSiblingElement("span");
        }
        lyrics.push_back(single);
        single.clear();
        p = p->NextSiblingElement("p");
    }
    // 验证输出
//	for (std::vector<std::vector<CharInfo>>::iterator i=lyrics.begin(); i!=lyrics.end(); ++i) {
//		for (std::vector<CharInfo>::iterator t=(*i).begin(); t!=(*i).end(); ++t) {
//			std::cout << (*t).startTime << " " << (*t).endTime << " " << (*t).character << std::endl;
//		}
//	}

	// 隐藏光标
	CONSOLE_CURSOR_INFO cursor_info = {1, 0};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

	DWORD startTime = GetTickCount();
	DWORD now = GetTickCount()-startTime;
	system("cls");
//	std::cout << lyrics.back().back().endTime << std::endl;
	while (now <= lyrics.back().back().endTime) {
		DWORD now = GetTickCount()-startTime;
		for (unsigned short s=topParaIdx; s<topParaIdx+DSPLINES; ++s) {
			for (std::vector<CharInfo>::iterator iter=lyrics[s].begin(); iter!=lyrics[s].end(); ++iter) {
				if (now < (*iter).startTime) (*iter).color = NORCOLOR;
				else if (now>=(*iter).startTime && now<=(*iter).endTime) {
					(*iter).color = ACTCOLOR;
					curIndex = s;
				} else (*iter).color = FINCOLOR;
			}
		}
		// 判断是否需要滚动
		if (curIndex-topParaIdx > ROLLINES && topParaIdx+DSPLINES < lyrics.size()) {
			topParaIdx = curIndex - ROLLINES;
			outLyrics(lyrics, topParaIdx, true);
		} else outLyrics(lyrics, topParaIdx, false);
		Sleep(FLUSH_INTERVAL);
	}

    std::cout << "\n\n🎵 播放完成！" << std::endl;
    return 0;
}