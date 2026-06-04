#include "document.h"
#include "chunk.h"




int main() {              
    StreamingUTF8Splitter a;
    a.splitByCharCount("../king3.txt","chunk",500);
    ChunkOverlapProcessor b;
    b.process(chunks);
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
    
    return 0;
}