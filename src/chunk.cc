#include "chunk.h"
#include<vector>
#include<iostream>

// 测试函数（原 main 函数重命名）
void testChunkSerialization() {
    std::vector<chunk> chunks;
    try{
        std::ifstream chunk_file("../chunks.txt", std::ios::binary);
        while(chunk_file.peek() != EOF){
            chunk temp;
            temp=temp.deserialize(chunk_file);
            chunks.push_back(temp);
        }

        std::cout<<"成功读取文件";
    }
    catch(...){

    }
    for (auto i :chunks){
        std::cout<<i.id<<i.metadata<<" "<<i.text<<std::endl;
        for (auto j:i.embedding){
            std::cout<<j;
        }
    }
}