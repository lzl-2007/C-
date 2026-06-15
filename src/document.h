#pragma once
#include<cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <string>
#include <cstddef>
#include <regex>
#include <cstdint> 
#include "chunk.h"

extern std::vector<chunk> chunks; 

class StreamingUTF8Splitter {
private:
    static const uint64_t  BUFFER_SIZE = 2000 * 1024;  
    static const uint64_t  MAX_CHAR_SIZE = 4;        
    static const uint64_t  OVERLAP_SIZE=50;
public:
    static void StreamingUTF8Splitter::splitByCharCount(const std::string& inputFile,uint64_t  charsPerChunk); 
private:
    static uint64_t  findNextBoundary(const std::string& data, uint64_t  start) {
        if (start >= data.size()) {
            return data.size();
        }
        uint64_t  pos = start;
        // 如果当前位置不是起始字节，先移动到下一个起始字节
        while (pos < data.size() && !isUTF8StartByte(static_cast<unsigned char>(data[pos]))) {
            pos++;
        }
        // 如果已经到达末尾
        if (pos >= data.size()) {
            return data.size();
        }
        // 跳过当前完整字符，到达下一个字符的起始
        uint64_t  charLen = getCharLength(static_cast<unsigned char>(data[pos]));
        pos += charLen;
        // 确保 pos 指向下一个字符的起始（可能到达末尾）
        return pos;
    }
    static bool isUTF8StartByte(unsigned char c) {
        return (c & 0x80) == 0 || (c & 0xC0) == 0xC0;
    }
    static uint64_t  getCharLength(unsigned char firstByte) {
        if ((firstByte & 0x80) == 0) return 1;
        if ((firstByte & 0xE0) == 0xC0) return 2;
        if ((firstByte & 0xF0) == 0xE0) return 3;
        if ((firstByte & 0xF8) == 0xF0) return 4;
        return 1;  // 无效
    }
};