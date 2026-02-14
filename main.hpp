#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

#include "tinyxml2/tinyxml2.h"
#include "getStringWidthInTerminal.hpp"

#define DSPLINES 15 // 显示行数
#define ROLLINES 10 // 滚动行数
//#define WIDTH    60 // 歌词宽度
 
#define NORCOLOR "\033[37m" // 常规颜色
#define ACTCOLOR "\033[32m" // 活动颜色
#define FINCOLOR "\033[90m" // 完成颜色
#define COLOREND "\033[0m"  // 颜色结束符

#define FLUSH_INTERVAL 20 // 刷新间隔

unsigned short width;

struct CharInfo {
    unsigned int startTime;
    unsigned int endTime;
    std::string character;
    std::string color = NORCOLOR; // 当前字符的颜色（实时计算）
};

struct Para {
    bool paraPos;              // 段落位置，true为居右
    unsigned short status = 1; // 这个段落的状态（0播放完毕1未播放）
    unsigned short length;     // 这行歌词在控制台中占据的字符宽度
    unsigned int startTime;
    unsigned int endTime;
    std::vector<CharInfo> characters;
    std::string text;
};

// 输出一个歌词片段
void outLyrics(std::vector<Para>& lyrics, unsigned short idx) {
	std::string p;
	for (unsigned short s=idx; s<idx+DSPLINES; ++s) {
        if (lyrics[s].paraPos) p.append(width-lyrics[s].length, ' ');
        if (!lyrics[s].status) p += FINCOLOR + lyrics[s].text + COLOREND;
        else if (lyrics[s].status == 1) p += NORCOLOR + lyrics[s].text + COLOREND;
        else if (s < lyrics.size())
            for (CharInfo& iter : lyrics[s].characters)
                p += iter.color + iter.character + COLOREND;
		p += "\033[K\n";
	}
	std::cout << p << "\033[H" << std::flush;
}

// 时间字符串解析函数（增强版）
// 支持格式：hh:mm:ss.mmm, mm:ss.mmm, s.sss, s
int parseTime(const char* timeStr) {
    if (!timeStr) return 0;
    std::string s = timeStr;
    // trim
    auto ltrim = [](std::string &str){
        size_t p = str.find_first_not_of(" \t\r\n");
        if (p == std::string::npos) str.clear();
        else if (p) str.erase(0, p);
    };
    auto rtrim = [](std::string &str){
        size_t p = str.find_last_not_of(" \t\r\n");
        if (p == std::string::npos) str.clear();
        else if (p+1 < str.size()) str.erase(p+1);
    };
    ltrim(s); rtrim(s);
    if (s.empty()) return 0;

    try {
        size_t colonCount = std::count(s.begin(), s.end(), ':');

        auto parseSecAndFrac = [](const std::string &part) -> std::pair<int,int> {
            // 返回 {seconds, milliseconds}
            size_t dot = part.find('.');
            if (dot == std::string::npos) {
                int sec = std::stoi(part);
                return { sec, 0 };
            } else {
                std::string secPart = part.substr(0, dot);
                std::string fracPart = part.substr(dot+1);
                int sec = secPart.empty() ? 0 : std::stoi(secPart);
                // 只取前 3 位小数并补齐
                if (fracPart.size() > 3) fracPart = fracPart.substr(0,3);
                while (fracPart.size() < 3) fracPart.push_back('0');
                int ms = std::stoi(fracPart);
                return { sec, ms };
            }
        };

        if (colonCount == 2) {
            // hh:mm:ss(.mmm)
            size_t p1 = s.find(':');
            size_t p2 = s.find(':', p1+1);
            std::string hStr = s.substr(0, p1);
            std::string mStr = s.substr(p1+1, p2-p1-1);
            std::string rest = s.substr(p2+1);
            int h = hStr.empty() ? 0 : std::stoi(hStr);
            int m = mStr.empty() ? 0 : std::stoi(mStr);
            auto [sec, ms] = parseSecAndFrac(rest);
            long totalMs = (long)h*3600*1000 + (long)m*60*1000 + (long)sec*1000 + ms;
            return (int)totalMs;
        } else if (colonCount == 1) {
            // mm:ss(.mmm)
            size_t p = s.find(':');
            std::string mStr = s.substr(0, p);
            std::string rest = s.substr(p+1);
            int m = mStr.empty() ? 0 : std::stoi(mStr);
            auto [sec, ms] = parseSecAndFrac(rest);
            long totalMs = (long)m*60*1000 + (long)sec*1000 + ms;
            return (int)totalMs;
        } else {
            // seconds or seconds.fraction
            // 使用 stod 以支持小数秒
            double secf = std::stod(s);
            if (secf < 0) return 0;
            long totalMs = (long)std::llround(secf * 1000.0);
            return (int)totalMs;
        }
    } catch (...) {
        return 0; // 解析失败返回0
    }
}

int launch(std::filesystem::path filepath) {
    std::vector<Para> lyrics;
    unsigned short topParaIdx = 0;
    unsigned short curIndex   = 0;

	// 读取文件内容
    std::ifstream file(filepath, std::ios::binary);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string xmlContent(size, '\0');
    if (!file.is_open()) xmlContent = "";
    file.read(&xmlContent[0], size);
    file.close();
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
    if (!tt) { std::cerr << "格式错误: 缺少 <tt>" << std::endl; return 1; }
    tinyxml2::XMLElement* body = tt->FirstChildElement("body");
    if (!body) { std::cerr << "格式错误: 缺少 <body>" << std::endl; return 1; }
    tinyxml2::XMLElement* div = body->FirstChildElement("div");
    if (!div) { std::cerr << "格式错误: 缺少 <div>" << std::endl; return 1; }

    tinyxml2::XMLElement* p = div->FirstChildElement("p");
    while (p) {
        Para singlePara;
    	std::vector<CharInfo> single;
        
        const char* agentAttrRaw = p->Attribute("ttm:agent");
        std::string agentAttr = agentAttrRaw ? agentAttrRaw : "";
        singlePara.paraPos = (agentAttr == "v1");

        tinyxml2::XMLElement* span = p->FirstChildElement("span");
        while (span) {
            const char* beginAttr = span->Attribute("begin");
            const char* endAttr   = span->Attribute("end");
            const char* text      = span->GetText();
            
            // 确保文本存在且不为空
            if (beginAttr && endAttr && text && strlen(text) > 0) {
                unsigned int start = parseTime(beginAttr);
                unsigned int end   = parseTime(endAttr);
    			single.push_back(CharInfo{start, end, text});
            }
            span = span->NextSiblingElement("span");
        }
        
        if (!single.empty()) {
            const char* agentAttrRaw = p->Attribute("ttm:agent");
            const char* ParaBegin    = p->Attribute("begin");
            const char* ParaEnd      = p->Attribute("end");
            std::string agentAttr    = agentAttrRaw ? agentAttrRaw : "";

            for (CharInfo& iter : single) singlePara.text += iter.character;
            singlePara.paraPos       = (agentAttr == "v2");
            singlePara.startTime     = parseTime(ParaBegin);
            singlePara.endTime       = parseTime(ParaEnd);
            singlePara.characters    = single;
            singlePara.length        = console_width(singlePara.text);

            lyrics.push_back(singlePara);
        }
        
        p = p->NextSiblingElement("p");
    }

    if (lyrics.empty()) {
        std::cout << "未解析到有效歌词数据！" << std::endl;
        return 0;
    }

    // 计算歌词的最大宽度
    for (Para& iter : lyrics) if (iter.paraPos && iter.length > width) 
        width = iter.length;
    width += 5;

    // 隐藏控制台光标
	CONSOLE_CURSOR_INFO cursor_info = {1, 0};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

	const DWORD startTime = GetTickCount();
	DWORD now;
	std::cout << "\033[2J\033[H";

    // 计算结束时间 (最后一行最后一个字的结束时间)
    unsigned int finalTime = 0;
    if (!lyrics.empty() && !lyrics.back().characters.empty()) 
        finalTime = lyrics.back().characters.back().endTime;

	while ((now = GetTickCount() - startTime) <= finalTime + 1000) {
		for (unsigned short s=topParaIdx; s<topParaIdx+DSPLINES; ++s) {
            if (s >= lyrics.size()) break;
            if (!lyrics[s].status) continue;
            if (now > lyrics[s].endTime) {
                lyrics[s].status = 0;
                continue;
            } else if (now < lyrics[s].startTime) lyrics[s].status = 1;
            else lyrics[s].status = 2;

			for (CharInfo& iter : lyrics[s].characters) {
				if (now < iter.startTime) iter.color = NORCOLOR;
				else if (now>=iter.startTime && now<=iter.endTime) {
					iter.color = ACTCOLOR;
					curIndex = s;
				} else iter.color = FINCOLOR;
			}
		}

		if (curIndex-topParaIdx > ROLLINES && topParaIdx+DSPLINES < lyrics.size()) {
			topParaIdx = curIndex - ROLLINES;
			outLyrics(lyrics, topParaIdx);
		} else outLyrics(lyrics, topParaIdx);
		Sleep(FLUSH_INTERVAL);
	}

    for (unsigned short s=1; s-1<=DSPLINES; ++s) std::cout << "\n";

    // 重新显示控制台光标
	cursor_info = {1, 1};
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursor_info);

    return 0;
}
