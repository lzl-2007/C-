#include "document.h"
#include "chunk.h"
#include <iostream>
#include <fstream>

// 定义全局变量（在头文件中声明为 extern）
std::vector<chunk> chunks;

void StreamingUTF8Splitter::splitByCharCount(const std::string& inputFile,
                                  uint64_t  charsPerChunk) {
        std::regex pattern(R"(第.{1,12}回)");
        std::string chapter=""; 
        std::ifstream inFile(inputFile, std::ios::binary);
        if (!inFile) {
            throw std::runtime_error("无法打开输入文件");
        }
        
        // 使用智能指针管理缓冲区
        auto buffer = std::make_unique<char[]>(BUFFER_SIZE + MAX_CHAR_SIZE);
        std::string leftover;  // 跨缓冲区的字符残留
        uint64_t  index=0;
        uint64_t  chunkIndex = 0;
        uint64_t  charCountInChunk = 0;
        std::string currentChunk;
        currentChunk.reserve(charsPerChunk * 3);  //为每个chunk预留内存
        while (inFile) {
            // 读取数据块
            inFile.read(buffer.get(), BUFFER_SIZE);
            uint64_t  bytesRead = inFile.gcount();
            if (bytesRead == 0) break;
            
            // 合并上次残留的字节
            std::string data = leftover + std::string(buffer.get(), bytesRead);
            leftover.clear();
            
            // 处理当前块
            uint64_t pos = 0;
            while (pos < data.size()) {
                // 找到下一个字符边界
                uint64_t  nextBoundary = findNextBoundary(data, pos);
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
                        uint64_t  nextBoundary = findNextBoundary(data, pos);
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