#include "document.h"
#include "chunk.h"
#include <iostream>
#include <fstream>

// 定义全局变量（在头文件中声明为 extern）
std::vector<chunk> chunks;

// 测试函数（原 main 函数重命名）
void testDocumentProcessing() {              
    StreamingUTF8Splitter::splitByCharCount("../king3.txt", 500);
    ChunkOverlapProcessor::process(chunks);
    try{
        std::ofstream chunk_file("../chunks.txt", std::ios::binary);
        for (auto i :chunks){
            i.embedding.push_back(0);
            i.embedding.push_back(1);
            i.serialize(chunk_file);
        }
        std::cout<<"成功写入文件";
    }
    catch(...){

    }
}