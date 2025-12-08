#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include <io.h>
#include <conio.h>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <sstream>
#include "tinyxml2/tinyxml2.h"

// 输出ttml文件列表的一页
// 无页码合法性检验
void outPage(std::vector<std::string>* files, unsigned short pages) {
	--pages;
	system("cls");
	for (unsigned short s = pages * 10; s < pages * 10 + 10 && s < (*files).size(); ++s)
		std::cout << s - pages * 10 << ' ' << (*files)[s] << std::endl;
	std::cout << "\nPage:" << pages + 1 << " / " << ((*files).size() + 9) / 10 << std::endl;
}

// 更健壮的时间字符串解析函数，支持 mm:ss.xxx 与 hh:mm:ss.xxx 等常见变体
double parseTime(const char* timeStr) {
	if (!timeStr) return 0.0;
	int colonCount = 0;
	for (const char* p = timeStr; *p; ++p) if (*p == ':') ++colonCount;
	int h = 0, m = 0, s = 0, ms = 0;
	if (colonCount == 2) {
		// hh:mm:ss.ms
		if (sscanf(timeStr, "%d:%d:%d.%d", &h, &m, &s, &ms) >= 3) {
			double frac = ms ? (ms / 1000.0) : 0.0;
			return h * 3600.0 + m * 60.0 + s + frac;
		}
	} else if (colonCount == 1) {
		// mm:ss.ms or mm:ss
		if (sscanf(timeStr, "%d:%d.%d", &m, &s, &ms) >= 2) {
			double frac = ms ? (ms / 1000.0) : 0.0;
			return m * 60.0 + s + frac;
		}
		if (sscanf(timeStr, "%d:%d", &m, &s) == 2) {
			return m * 60.0 + s;
		}
	}
	// only seconds or fallback
	if (sscanf(timeStr, "%d.%d", &s, &ms) >= 1) {
		double frac = ms ? (ms / 1000.0) : 0.0;
		return s + frac;
	}
	return 0.0;
}

// 读取文件内容到字符串
std::string readFileContent(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "无法打开文件: " << filename << std::endl;
		return "";
	}
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::string content(size, '\0');
	file.read(&content[0], size);
	file.close();
	return content;
}

// 移动控制台光标 (绝对坐标)
void gotoxy(int x, int y) {
	COORD pos;
	pos.X = (SHORT)x;
	pos.Y = (SHORT)y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

struct Span {
	double start;
	double end;
	std::string text;
};

struct Paragraph {
	std::vector<Span> spans;
};

// 把段落渲染到字符串（带颜色的 ANSI 序列）而不是直接输出
std::string renderParagraphToString(const Paragraph& para, double t, bool dimPast) {
	const char* COLOR_RESET = "\x1b[0m";
	const char* COLOR_ACTIVE = "\x1b[1;32m"; // 亮绿色
	const char* COLOR_DIM = "\x1b[90m";      // 灰色
	const char* COLOR_NORMAL = "\x1b[37m";   // 白色

	std::ostringstream oss;
	for (const auto& s : para.spans) {
		bool active = (t + 1e-6 >= s.start) && (t <= s.end + 1e-6);
		if (active) {
			oss << COLOR_ACTIVE << s.text << COLOR_RESET;
		} else if (dimPast) {
			oss << COLOR_DIM << s.text << COLOR_RESET;
		} else {
			oss << COLOR_NORMAL << s.text << COLOR_RESET;
		}
	}
	return oss.str();
}

// 计算段落中最早的开始时间，用于判断（若需要）
double paragraphFirstStart(const Paragraph& p) {
	double v = 1e300;
	bool any = false;
	for (const auto& s : p.spans) {
		v = std::min(v, s.start);
		any = true;
	}
	return any ? v : 1e300;
}

int main() {
	short posy;
	std::string filename;
	std::vector<Paragraph> paragraphs;

	// 寻找ttml文件
	struct _finddata_t fileInfo;
	std::vector<std::string> ttmlFiles;
	intptr_t handle = _findfirst(".\\music\\*.ttml", &fileInfo);
	if (handle != -1L) do ttmlFiles.push_back(fileInfo.name); while (_findnext(handle, &fileInfo) == 0);
	else {
		std::cout << "没有找到ttml文件，请先将ttml歌词文件放到music目录" << std::endl;
		return 0;
	}
	_findclose(handle);

	// 选择ttml文件（按页面和0-9键）
	unsigned short pages = 1;
	outPage(&ttmlFiles, 1);
	while (1) {
		if (_kbhit()) {
			unsigned short key = _getch();
			// 翻页和选择逻辑
			if (key == 224) {
				key = _getch();
				if ((key == 80 || key == 81 || key == 77) && pages * 10 < ttmlFiles.size()) outPage(&ttmlFiles, ++pages);
				if ((key == 72 || key == 73 || key == 75) && pages > 1) outPage(&ttmlFiles, --pages);
				if (key == 71) outPage(&ttmlFiles, pages = 1);
				if (key == 79) outPage(&ttmlFiles, pages = ttmlFiles.size() / 10 + 1);
			}
			if (key >= 48 && key <= 57) {
				int idx = (pages - 1) * 10 + (key - 48);
				if (idx < (int)ttmlFiles.size()) {
					filename = ".\\music\\" + ttmlFiles[idx];
					break;
				}
			}
		}
		Sleep(10);
	}

	// 让控制台使用 UTF-8 输出
	SetConsoleOutputCP(CP_UTF8);

	// 读取文件内容
	std::string xmlContent = readFileContent(filename);
	if (xmlContent.empty()) {
		std::cerr << "文件内容为空或读取失败" << std::endl;
		return 1;
	}

	// 解析 XML，并按段落（p）组装每个段落的 span 列表（保持在文件中的顺序）
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

	tinyxml2::XMLElement* p = div->FirstChildElement("p");
	while (p) {
		Paragraph para;
		tinyxml2::XMLElement* span = p->FirstChildElement("span");
		bool hasChars = false;
		while (span) {
			const char* beginAttr = span->Attribute("begin");
			const char* endAttr = span->Attribute("end");
			const char* text = span->GetText();
			if (beginAttr && endAttr && text && strlen(text) > 0) {
				double start = parseTime(beginAttr);
				double end = parseTime(endAttr);
				para.spans.push_back(Span{ start, end, std::string(text) });
				hasChars = true;
			}
			span = span->NextSiblingElement("span");
		}
		if (hasChars) paragraphs.push_back(std::move(para));
		p = p->NextSiblingElement("p");
	}

	if (paragraphs.empty()) {
		std::cout << "未解析到任何歌词内容" << std::endl;
		return 0;
	}

	// 开启 ANSI 控制序列支持（用于颜色）
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut != INVALID_HANDLE_VALUE) {
		DWORD mode = 0;
		if (GetConsoleMode(hOut, &mode)) {
			mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			SetConsoleMode(hOut, mode);
		}
	}

	// 隐藏光标以减少闪烁视觉
	CONSOLE_CURSOR_INFO oldCCI;
	GetConsoleCursorInfo(hOut, &oldCCI);
	CONSOLE_CURSOR_INFO cci = oldCCI;
	cci.bVisible = FALSE;
	SetConsoleCursorInfo(hOut, &cci);

	// 计算整首歌的结束时间（用于退出）
	double songEnd = 0.0;
	for (const auto& para : paragraphs) {
		for (const auto& s : para.spans) songEnd = std::max(songEnd, s.end);
	}

	// 播放前提示
	std::cout << "\n\n===== 开始歌词播放 =====" << std::endl;
	std::cout << "播放时支持一段内同时多个位置变色（多时间轴），并会在屏幕达到滚动阈值时滚动。\n";
	std::cout << "按 Enter 键开始...";
	std::cin.get();

	using clock = std::chrono::steady_clock;
	auto startTimePoint = clock::now();

	// 显示窗口参数：窗口总行数和当前行在窗口中的行号（参考原来的 posy >= 4 行滚动逻辑）
	const int WINDOW_LINES = 10;
	const int CURRENT_ROW_IN_WINDOW = 4; // current paragraph displayed at this row index (0-based)
	int topIndex = 0;
	int curParagraphIdx = -1;

	// 缓存上一次渲染的可见行，用于差分更新，减少IO和闪烁
	std::vector<std::string> prevRendered(WINDOW_LINES, "");
	std::vector<std::string> curRendered(WINDOW_LINES, "");

	// 主刷新循环：每 20ms 重绘可见窗口（只更新变更行）
	const std::chrono::milliseconds TICK(20);
	while (true) {
		auto now = clock::now();
		std::chrono::duration<double> elapsed = now - startTimePoint;
		double t = elapsed.count(); // 秒

		// 更新当前段落索引：选取最后一个其任一 span 的 start <= t 的段落（保持原文件顺序）
		int lastIdx = -1;
		for (int i = 0; i < (int)paragraphs.size(); ++i) {
			for (const auto& s : paragraphs[i].spans) {
				if (s.start <= t + 1e-6) { lastIdx = i; break; }
			}
		}
		if (lastIdx != -1) curParagraphIdx = lastIdx;

		// 当到达一段的下一行时滚动：确保当前行位于窗口的 CURRENT_ROW_IN_WINDOW 行
		if (curParagraphIdx >= 0) {
			int desiredTop = curParagraphIdx - CURRENT_ROW_IN_WINDOW;
			if (desiredTop < 0) desiredTop = 0;
			if (desiredTop != topIndex) {
				// 当 topIndex 变化时需要清空 prevRendered 来强制重绘窗口
				topIndex = desiredTop;
				std::fill(prevRendered.begin(), prevRendered.end(), std::string());
			}
		}

		// 生成当前窗口的渲染内容（字符串）
		for (int r = 0; r < WINDOW_LINES; ++r) {
			int paragraphIndex = topIndex + r;
			if (paragraphIndex >= (int)paragraphs.size()) {
				curRendered[r].clear();
			} else {
				bool dimPast = (curParagraphIdx >= 0) && (paragraphIndex < curParagraphIdx);
				curRendered[r] = renderParagraphToString(paragraphs[paragraphIndex], t, dimPast);
			}
		}

		// 差分更新：只更新与 prevRendered 不同的行
		for (int r = 0; r < WINDOW_LINES; ++r) {
			if (curRendered[r] != prevRendered[r]) {
				// move cursor to line r, column 0
				gotoxy(0, r);
				// 清除行并写入新的内容 (ANSI: erase line from cursor: \x1b[2K then carriage return)
				// 使用 \x1b[2K 清除整行，然后回到行首并输出内容
				std::cout << "\x1b[2K" << '\r' << curRendered[r];
				// 如果内容比之前短，确保删除残留：执行清除到行尾
				std::cout << "\x1b[K";
				std::cout.flush();
				prevRendered[r] = curRendered[r];
			}
		}

		// 退出条件：播放时间超过 songEnd + 0.5s
		if (t > songEnd + 0.5) break;

		std::this_thread::sleep_until(now + TICK);
	}

	// 恢复光标可见性
	SetConsoleCursorInfo(hOut, &oldCCI);

	std::cout << "\n\n🎵 播放完成！" << std::endl;
	return 0;
}