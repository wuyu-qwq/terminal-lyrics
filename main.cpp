#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <windows.h>
#include <chrono>
#include "tinyxml2/tinyxml2.h"

struct CharInfo {
    double startTime;   // 开始时间（秒）
    double endTime;     // 结束时间（秒）
    char character[100];  // 单字
    CharInfo* next;     // 指向下一个节点的指针
};

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

// 新增函数：创建一个字符节点并添加到链表末尾
void addCharNode(CharInfo*& head, CharInfo*& tail, double start, double end, const char* text) {
    CharInfo* newNode = new CharInfo;
    
    newNode->startTime = start;
    newNode->endTime = end;
    // 安全复制 UTF-8 字符（最多3字节汉字）
    strncpy(newNode->character, text, sizeof(newNode->character) - 1);
    newNode->character[sizeof(newNode->character) - 1] = '\0';
    newNode->next = nullptr;

    if (!head) {
        head = tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
}

// 移动控制台光标
void gotoxy(int x, int y) {
	COORD pos;
	pos.X = x, pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void addNewlineNode(CharInfo*& head, CharInfo*& tail) {
    addCharNode(head, tail, 0.0, 0.0, "\n");
}

bool outLyrics(CharInfo*& cur, short lines) {
	system("cls");
	short count;
	bool b = true;
	CharInfo* _ptr = cur;
    while (count <= 10) {
    	if (lines > 0) {
    		std::cout << _ptr->character << std::flush;
		} else std::cout << "\033[90m" << _ptr->character << "\033[0m" << std::flush;;
    	_ptr = _ptr->next;
    	if (_ptr->character[0] == '\n') {
    		if (b) {
    			cur = _ptr->next;
    			b = false;
			}
    		++count;
    		if (lines > 0) --lines;
		}
	}
	return _ptr->next == nullptr ? true : false;
}

int main() {
	short posy;
	// 设置控制台输出编码
	SetConsoleOutputCP(65001);
	
    // 文件名
    const char* filename = "apoint.ttml";
    
    // 读取文件内容
    std::string xmlContent = readFileContent(filename);
    if (xmlContent.empty()) {
        std::cerr << "文件内容为空或读取失败" << std::endl;
        return 1;
    }

    // 初始化链表
    CharInfo* head = nullptr;
    CharInfo* tail = nullptr;

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

        while (span) {
            const char* beginAttr = span->Attribute("begin");
            const char* endAttr = span->Attribute("end");
            const char* text = span->GetText();

            if (beginAttr && endAttr && text && strlen(text) > 0) {
                double start = parseTime(beginAttr);
                double end = parseTime(endAttr);

                addCharNode(head, tail, start, end, text);
                hasChars = true;
            }
            span = span->NextSiblingElement("span");
        }

        // 如果这个段落有内容，在结尾加一个换行符
        if (hasChars) {
            addNewlineNode(head, tail);
        }

        p = p->NextSiblingElement("p");
    }

    // 验证输出
//    std::cout << "解析结果:" << std::endl;
//    int count = 0;
//    CharInfo* current = head;
//    while (current) {
//        count++;
//        printf("字: %s, 开始: %.3f秒, 结束: %.3f秒\n", 
//               current->character, 
//               current->startTime, 
//               current->endTime);
//        current = current->next;
//    }
//    std::cout << "总计解析 " << count << " 个字" << std::endl;

	// === 正式逐字输出歌词 ===

    std::cout << "\n\n===== 开始逐字播放歌词 =====" << std::endl;
    std::cout << "请确保控制台字体支持中文（如Consolas、宋体等）\n";
    std::cout << "按 Enter 键开始...";
    std::cin.get();

    auto programStartTick = GetTickCount();  // 当前系统启动以来的毫秒数

    CharInfo* cur = head;
    CharInfo* roll = head;
    
	outLyrics(roll, 1);
	gotoxy(0, 0);
	
	cur = head;
	bool rollfin = false;
	while (cur != nullptr) {
        DWORD targetMillis = static_cast<DWORD>((cur->startTime) * 1000.0);
        DWORD elapsed = GetTickCount() - programStartTick;
        if (targetMillis > elapsed) {
            Sleep(targetMillis - elapsed);
        }

        // 输出这个字（UTF-8）
//        std::cout << cur->character << std::flush;
		//改变颜色
		//gotoxy(posx, posy);
		std::cout << cur->character << std::flush;
		if (cur->character[0] == '\n') {
			// 如果光标大于第四行，则滚动歌词
			if (posy >= 4 && !rollfin) {
				rollfin = outLyrics(roll, 4);
				gotoxy(0, 4);
			} else posy++;
		}

        cur = cur->next;
    }

    std::cout << "\n\n🎵 播放完成！" << std::endl;

    // 释放链表内存
    while (head) {
        CharInfo* temp = head;
        head = head->next;
        delete temp;
    }

    return 0;
}