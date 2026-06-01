#pragma once
#include<iostream>
#include<cstring>
#include<vector>
struct chunk{
    size_t id;
    std::string text;
    std::string metadata;
    std::vector<float> embeddingl;
};
#include <iostream>
#include <string>
#include <vector>


class UTF8Helper {
public:
    static std::string getLastBytesSafe(const std::string& str, size_t maxBytes) {
        if (str.empty() || maxBytes == 0) return "";
        if (str.size() <= maxBytes) return str;

        // 从末尾向前数 maxBytes 个字节作为起点，然后向前调整到合法字符边界
        size_t startPos = str.size() - maxBytes;
        startPos = adjustToCharStart(str, startPos);
        return str.substr(startPos);
    }

private:
    // 判断一个字节是否为 UTF-8 字符的起始字节
    static bool isStartByte(unsigned char c) {
        return (c & 0x80) == 0 ||          // ASCII
               (c & 0xC0) == 0xC0;         // 多字节首字节 (110xxxxx, 1110xxxx, 11110xxx)
    }

    // 从给定位置向前调整到字符起始位置（不切断字符）
    static size_t adjustToCharStart(const std::string& str, size_t pos) {
        if (pos >= str.size()) return str.size();
        if (pos == 0) return 0;

        // 如果当前字节不是起始字节，则向前移动直到找到起始字节
        while (pos > 0 && !isStartByte(static_cast<unsigned char>(str[pos]))) {
            --pos;
        }
        // 如果第一个字节仍不是起始字节（损坏数据），返回 0
        if (!isStartByte(static_cast<unsigned char>(str[pos]))) return 0;
        return pos;
    }
};

// 负责处理 chunk 序列的类
class ChunkOverlapProcessor {
public:
    static void process(std::vector<chunk>& chunks) {
        if (chunks.size() < 2) return;  // 少于两个无需处理

        const size_t OVERLAP_BYTES = 80;

        for (size_t i = 1; i < chunks.size(); ++i) {
            // 获取前一个 chunk 的尾部最多 50 字节（保证 UTF-8 完整性）
            std::string suffix = UTF8Helper::getLastBytesSafe(chunks[i-1].text, OVERLAP_BYTES);
            
            // 如果确实有内容，则插入到当前 chunk 的 text 开头
            if (!suffix.empty()) {
                chunks[i].text = suffix + chunks[i].text;
            }
        }
    }
};

