#include <string>
#include <cstdint>

/// UTF-8 解码，返回下一个 code point，并移动索引
static char32_t utf8_next(const std::string& s, size_t& i) {
    unsigned char c = s[i];

    if (c < 0x80) {
        return s[i++];
    } else if ((c >> 5) == 0x6) {
        char32_t cp = ((s[i] & 0x1F) << 6)
                    | (s[i + 1] & 0x3F);
        i += 2;
        return cp;
    } else if ((c >> 4) == 0xE) {
        char32_t cp = ((s[i] & 0x0F) << 12)
                    | ((s[i + 1] & 0x3F) << 6)
                    | (s[i + 2] & 0x3F);
        i += 3;
        return cp;
    } else {
        char32_t cp = ((s[i] & 0x07) << 18)
                    | ((s[i + 1] & 0x3F) << 12)
                    | ((s[i + 2] & 0x3F) << 6)
                    | (s[i + 3] & 0x3F);
        i += 4;
        return cp;
    }
}

/// 是否是组合字符（宽度 0）
static bool is_combining(char32_t cp) {
    return
        (cp >= 0x0300 && cp <= 0x036F) || // Combining Diacritical Marks
        (cp >= 0x1AB0 && cp <= 0x1AFF) ||
        (cp >= 0x1DC0 && cp <= 0x1DFF) ||
        (cp >= 0x20D0 && cp <= 0x20FF) ||
        (cp >= 0xFE20 && cp <= 0xFE2F);
}

/// 是否是宽字符（宽度 2）
static bool is_wide(char32_t cp) {
    return
        // CJK Unified Ideographs
        (cp >= 0x4E00 && cp <= 0x9FFF) ||
        // CJK Extensions
        (cp >= 0x3400 && cp <= 0x4DBF) ||
        // Hangul
        (cp >= 0xAC00 && cp <= 0xD7A3) ||
        // Hiragana & Katakana
        (cp >= 0x3040 && cp <= 0x30FF) ||
        // Fullwidth forms
        (cp >= 0xFF01 && cp <= 0xFF60) ||
        (cp >= 0xFFE0 && cp <= 0xFFE6) ||
        // Emoji (大致范围，工程折中)
        (cp >= 0x1F300 && cp <= 0x1FAFF);
}

/// 计算字符串在控制台中占据的显示宽度
int console_width(const std::string& s) {
    int width = 0;
    size_t i = 0;

    while (i < s.size()) {
        char32_t cp = utf8_next(s, i);

        if (is_combining(cp)) continue;
        else if (is_wide(cp)) width += 2;
        else width += 1;
    }
    return width;
}
