#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <io.h>
#include <conio.h>
#include <fstream>
#include "tinyxml2/tinyxml2.h"

// Êä³öttmlÎÄ¼þÁÐ±íµÄÒ»Ò³
// ÎÞÒ³ÂëºÏ·¨ÐÔ¼ìÑé
void outPage(std::vector<std::string>* files, unsigned short pages) {
	--pages;
	system("cls");
	for (unsigned short s=pages*10; s<pages*10+10; ++s) std::cout << s-pages*10 << ' ' << (*files)[s] << std::endl;
	std::cout << "\nPage:" << pages+1 << " / " << (*files).size()/10+1 << std::endl;
}

struct CharInfo {
    double startTime;   // ¿ªÊ¼Ê±¼ä£¨Ãë£©
    double endTime;     // ½áÊøÊ±¼ä£¨Ãë£©
    char character[100];  // µ¥×Ö
    CharInfo* next;     // Ö¸ÏòÏÂÒ»¸ö½ÚµãµÄÖ¸Õë
};

// Ê±¼ä×Ö·û´®½âÎöº¯Êý
double parseTime(const char* timeStr) {
    int minutes = 0;
    int seconds = 0;
    int milliseconds = 0;
    
    // ½âÎöÊ±¼ä×Ö·û´®
    if (sscanf(timeStr, "%d:%d.%d", &minutes, &seconds, &milliseconds) == 3) {
        return minutes * 60.0 + seconds + milliseconds / 1000.0;
    }
    // ¼æÈÝ´¦Àí¿ÉÄÜ´æÔÚµÄ¸ñÊ½±äÌå
    if (sscanf(timeStr, "%d:%d.%d", &minutes, &seconds, &milliseconds) == 3) {
        return minutes * 60.0 + seconds + milliseconds / 1000.0;
    }
    return 0.0; // ½âÎöÊ§°Ü·µ»Ø0
}

// ¶ÁÈ¡ÎÄ¼þÄÚÈÝµ½×Ö·û´®
std::string readFileContent(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "ÎÞ·¨´ò¿ªÎÄ¼þ: " << filename << std::endl;
        return "";
    }
    
    // »ñÈ¡ÎÄ¼þ´óÐ¡
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    // ·ÖÅäÄÚ´æ²¢¶ÁÈ¡ÄÚÈÝ
    std::string content(size, '\0');
    file.read(&content[0], size);
    file.close();
    
    return content;
}

// ÐÂÔöº¯Êý£º´´½¨Ò»¸ö×Ö·û½Úµã²¢Ìí¼Óµ½Á´±íÄ©Î²
void addCharNode(CharInfo*& head, CharInfo*& tail, double start, double end, const char* text) {
    CharInfo* newNode = new CharInfo;
    
    newNode->startTime = start;
    newNode->endTime = end;
    // °²È«¸´ÖÆ UTF-8 ×Ö·û£¨×î¶à3×Ö½Úºº×Ö£©
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

// ÒÆ¶¯¿ØÖÆÌ¨¹â±ê
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
	// ÎÄ¼þÃû
    std::string filename;
    
	// Ñ°ÕÒttmlÎÄ¼þ
	struct _finddata_t fileInfo;
	std::vector<std::string> ttmlFiles;
	intptr_t handle = _findfirst(".\\music\\*.ttml", &fileInfo);
	if (handle != -1L) do ttmlFiles.push_back(fileInfo.name); while (_findnext(handle, &fileInfo) == 0); else {
		std::cout << "Ã»ÓÐÕÒµ½ttmlÎÄ¼þ£¬ÇëÏÈ½«ttml¸è´ÊÎÄ¼þ·Åµ½musicÄ¿Â¼" << std::endl;
		return 0;
	}
	//for (auto b = ttmlFiles.begin(); b != ttmlFiles.end(); ++b) std::cout << *b << std::endl;
	_findclose(handle);
	
	// Ñ¡ÔñttmlÎÄ¼þ
	unsigned short pages = 1;
	outPage(&ttmlFiles, 1);
	while (1) {
		if (_kbhit()) {
			unsigned short key = _getch();
			// °´ÏÂPageDown¡¢ÓÒ¼ýÍ·¡¢ÏÂ¼ýÍ·Ê±·­Ò³
			if (key == 224){
				key = _getch();
				// °´ÏÂPageDown¡¢ÓÒ¼ýÍ·¡¢ÏÂ¼ýÍ·Ê±ÏÂ·­Ò»Ò³
				if ((key==80 || key==81 || key==77) && pages*10<ttmlFiles.size()) outPage(&ttmlFiles, ++pages);
				// °´ÏÂPageUp¡¢×ó¼ýÍ·¡¢ÉÏ¼ýÍ·Ê±ÉÏ·­Ò»Ò³
				if ((key==72 || key==73 || key==75) && pages > 1) outPage(&ttmlFiles, --pages);
				// °´ÏÂHome¼üÊ±»Øµ½Ê×Ò³
				if (key == 71) outPage(&ttmlFiles, pages=1);
				// °´ÏÂEnd¼üÊ±Ìøµ½Î²Ò³
				if (key == 79) outPage(&ttmlFiles, pages=ttmlFiles.size()/10+1);
			} 
			// °´ÏÂ0µ½9Êý×Ö¼üÊ±Ñ¡Ôñ¸èÇú
			if (key>=48 && key<=57) {
				if ((pages-1)*10-(48-key)<ttmlFiles.size()) filename = ".\\music\\"+ttmlFiles[(pages-1)*10-(48-key)];
				break;
			}
    	}
	}
	SetConsoleOutputCP(65001);

	// ¶ÁÈ¡ÎÄ¼þÄÚÈÝ
    std::string xmlContent = readFileContent(filename);
    if (xmlContent.empty()) {
        std::cerr << "ÎÄ¼þÄÚÈÝÎª¿Õ»ò¶ÁÈ¡Ê§°Ü" << std::endl;
        return 1;
    }

    // ³õÊ¼»¯Á´±í
    CharInfo* head = nullptr;
    CharInfo* tail = nullptr;

    // ½âÎöXML ...
    tinyxml2::XMLDocument doc;
    if (doc.Parse(xmlContent.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "XML½âÎöÊ§°Ü! ´íÎó´úÂë: " << doc.ErrorID() << std::endl;
        return 1;
    }

    tinyxml2::XMLElement* tt = doc.FirstChildElement("tt");
    if (!tt) {
        std::cerr << "Î´ÕÒµ½¸ùÔªËØ <tt>" << std::endl;
        return 1;
    }

    tinyxml2::XMLElement* body = tt->FirstChildElement("body");
    if (!body) {
        std::cerr << "Î´ÕÒµ½ <body> ÔªËØ" << std::endl;
        return 1;
    }

    tinyxml2::XMLElement* div = body->FirstChildElement("div");
    if (!div) {
        std::cerr << "Î´ÕÒµ½ <div> ÔªËØ" << std::endl;
        return 1;
    }

    // ±éÀúËùÓÐ <p> ¶ÎÂä
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

        // Èç¹ûÕâ¸ö¶ÎÂäÓÐÄÚÈÝ£¬ÔÚ½áÎ²¼ÓÒ»¸ö»»ÐÐ·û
        if (hasChars) {
            addNewlineNode(head, tail);
        }

        p = p->NextSiblingElement("p");
    }

    // ÑéÖ¤Êä³ö
//    std::cout << "½âÎö½á¹û:" << std::endl;
//    int count = 0;
//    CharInfo* current = head;
//    while (current) {
//        count++;
//        printf("×Ö: %s, ¿ªÊ¼: %.3fÃë, ½áÊø: %.3fÃë\n", 
//               current->character, 
//               current->startTime, 
//               current->endTime);
//        current = current->next;
//    }
//    std::cout << "×Ü¼Æ½âÎö " << count << " ¸ö×Ö" << std::endl;

	// === ÕýÊ½Öð×ÖÊä³ö¸è´Ê ===

    std::cout << "\n\n===== ¿ªÊ¼Öð×Ö²¥·Å¸è´Ê =====" << std::endl;
    std::cout << "ÇëÈ·±£¿ØÖÆÌ¨×ÖÌåÖ§³ÖÖÐÎÄ£¨ÈçConsolas¡¢ËÎÌåµÈ£©\n";
    std::cout << "°´ Enter ¼ü¿ªÊ¼...";
    std::cin.get();

    auto programStartTick = GetTickCount();  // µ±Ç°ÏµÍ³Æô¶¯ÒÔÀ´µÄºÁÃëÊý

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

        // Êä³öÕâ¸ö×Ö£¨UTF-8£©
//        std::cout << cur->character << std::flush;
		//¸Ä±äÑÕÉ«
		//gotoxy(posx, posy);
		std::cout << cur->character << std::flush;
		if (cur->character[0] == '\n') {
			// Èç¹û¹â±ê´óÓÚµÚËÄÐÐ£¬Ôò¹ö¶¯¸è´Ê
			if (posy >= 4 && !rollfin) {
				rollfin = outLyrics(roll, 4);
				gotoxy(0, 4);
			} else posy++;
		}

        cur = cur->next;
    }

    std::cout << "\n\n?? ²¥·ÅÍê³É£¡" << std::endl;

    // ÊÍ·ÅÁ´±íÄÚ´æ
    while (head) {
        CharInfo* temp = head;
        head = head->next;
        delete temp;
    }

    return 0;
}
