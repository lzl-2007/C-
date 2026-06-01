#include "document.h"
#include "chunk.h"
#include<cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <string>
#include <cstddef>
#include <regex>
std::vector<chunk> chunks;
class StreamingUTF8Splitter {
private:
    static const size_t BUFFER_SIZE = 2000 * 1024;  
    static const size_t MAX_CHAR_SIZE = 4;        
    static const size_t OVERLAP_SIZE=50;
public:
    static void splitByCharCount(const std::string& inputFile,
                                  const std::string& outputPrefix,
                                  size_t charsPerChunk) {
        std::regex pattern(R"(第.{1,12}回)");
        std::string chapter=""; 
        std::ifstream inFile(inputFile, std::ios::binary);
        if (!inFile) {
            throw std::runtime_error("无法打开输入文件");
        }
        
        // 使用智能指针管理缓冲区
        auto buffer = std::make_unique<char[]>(BUFFER_SIZE + MAX_CHAR_SIZE);
        std::string leftover;  // 跨缓冲区的字符残留
        size_t index=0;
        size_t chunkIndex = 0;
        size_t charCountInChunk = 0;
        std::string currentChunk;
        currentChunk.reserve(charsPerChunk * 3);  //为每个chunk预留内存
        while (inFile) {
            // 读取数据块
            inFile.read(buffer.get(), BUFFER_SIZE);
            size_t bytesRead = inFile.gcount();
            if (bytesRead == 0) break;
            
            // 合并上次残留的字节
            std::string data = leftover + std::string(buffer.get(), bytesRead);
            leftover.clear();
            
            // 处理当前块
            size_t pos = 0;
            while (pos < data.size()) {
                // 找到下一个字符边界
                size_t nextBoundary = findNextBoundary(data, pos);
                if (nextBoundary == data.size()) {
                    // 字符不完整，留到下次处理
                    leftover = data.substr(pos);
                    break;
                }
                
                
                // 提取完整字符
                std::string oneChar = data.substr(pos, nextBoundary - pos);
                if (oneChar!="\r" && oneChar!="\n"){
                    currentChunk += oneChar;
                    charCountInChunk++;
                }
                
                if (std::regex_search(currentChunk, pattern)){
                    chapter.clear();
                    pos=nextBoundary;
                    while(1){
                        size_t nextBoundary = findNextBoundary(data, pos);
                        if (nextBoundary == data.size()) {
                            leftover = data.substr(pos);
                            break;
                        }
                        std::string oneChar = data.substr(pos, nextBoundary - pos);
                        chapter+=oneChar;
                        pos=nextBoundary;
                        if (oneChar=="\n" || oneChar=="\r")break;
                    }
                    std::cout<<chapter;
                
                }
                // 达到切割点
                if (charCountInChunk >= charsPerChunk || std::regex_search(currentChunk, pattern)) {
                    //writeChunk(currentChunk, outputPrefix, chunkIndex++);
                    chunk temp;
                    temp.id=index++;
                    temp.metadata=chapter;
                    temp.text=currentChunk;
                    chunks.push_back(temp);
                    currentChunk.clear();
                    charCountInChunk = 0;
                }
                
                pos = nextBoundary;
            }
        }
        // 写入最后一个不完整的片段
        if (!currentChunk.empty()) {
            chunk temp;
            temp.id=index++;
            temp.metadata=chapter;
            temp.text=currentChunk;
            chunks.push_back(temp);
            currentChunk.clear();
        }
    }
    
private:
    static size_t findNextBoundary(const std::string& data, size_t start) {
        if (start >= data.size()) {
            return data.size();
        }
        size_t pos = start;
        // 如果当前位置不是起始字节，先移动到下一个起始字节
        while (pos < data.size() && !isUTF8StartByte(static_cast<unsigned char>(data[pos]))) {
            pos++;
        }
        // 如果已经到达末尾
        if (pos >= data.size()) {
            return data.size();
        }
        // 跳过当前完整字符，到达下一个字符的起始
        size_t charLen = getCharLength(static_cast<unsigned char>(data[pos]));
        pos += charLen;
        // 确保 pos 指向下一个字符的起始（可能到达末尾）
        return pos;
    }
    static bool isUTF8StartByte(unsigned char c) {
        return (c & 0x80) == 0 || (c & 0xC0) == 0xC0;
    }
    static size_t getCharLength(unsigned char firstByte) {
        if ((firstByte & 0x80) == 0) return 1;
        if ((firstByte & 0xE0) == 0xC0) return 2;
        if ((firstByte & 0xF0) == 0xE0) return 3;
        if ((firstByte & 0xF8) == 0xF0) return 4;
        return 1;  // 无效
    }
};

int main() {              
    StreamingUTF8Splitter a;
    a.splitByCharCount("../king3.txt","chunk",500);
    ChunkOverlapProcessor b;
    b.process(chunks);
    for (auto i :chunks){
        std::cout<<i.id<<":\n";
        std::cout<<i.metadata<<":\n";
        std::cout<<i.text<<std::endl;
    }
    return 0;
}