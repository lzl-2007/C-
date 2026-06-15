#include "chunk.h"
#include<vector>
#include<iostream>

// 序列化：将对象写入二进制流
void chunk::serialize(std::ofstream& out) const {
    // 写入 id（固定8字节）
    out.write(reinterpret_cast<const char*>(&id), sizeof(id));
    
    // 写入 text：先长度（4字节），再内容
    uint64_t  text_len = static_cast<uint64_t >(text.size());
    out.write(reinterpret_cast<const char*>(&text_len), sizeof(text_len));
    out.write(text.data(), text_len);
    
    // 写入 metadata：同样方式
    uint64_t  meta_len = static_cast<uint64_t >(metadata.size());
    out.write(reinterpret_cast<const char*>(&meta_len), sizeof(meta_len));
    out.write(metadata.data(), meta_len);

    // 写入 embedding：先维度（4字节），再浮点数组
    uint64_t  embedding_len = static_cast<uint64_t >(embedding.size());
    out.write(reinterpret_cast<const char*>(&embedding_len), sizeof(embedding_len));
    if (embedding_len > 0) {
        // 将 float 数组的字节直接写入
        out.write(reinterpret_cast<const char*>(embedding.data()), 
                    embedding_len * sizeof(float));
    }
}

// 反序列化：从二进制流读取并构造对象
chunk chunk::deserialize(std::ifstream& in) {
    chunk ch;
    // 读取 id
    in.read(reinterpret_cast<char*>(&ch.id), sizeof(ch.id));
    
    // 读取 text
    uint64_t  text_len;
    in.read(reinterpret_cast<char*>(&text_len), sizeof(text_len));
    ch.text.resize(text_len);
    in.read(&ch.text[0], text_len);
    
    // 读取 metadata
    uint64_t  meta_len;
    in.read(reinterpret_cast<char*>(&meta_len), sizeof(meta_len));
    ch.metadata.resize(meta_len);
    in.read(&ch.metadata[0], meta_len);
    
    // 读取 embedding
    uint64_t embedding_len;
    in.read(reinterpret_cast<char*>(&embedding_len), sizeof(embedding_len));
    ch.embedding.resize(embedding_len);
    if (embedding_len > 0) {
        in.read(reinterpret_cast<char*>(ch.embedding.data()), 
                embedding_len * sizeof(float));
    }

    return ch;
}

void ChunkOverlapProcessor::process(std::vector<chunk>& chunks) {
    if (chunks.size() < 2) return;  // 少于两个无需处理

    const uint64_t  OVERLAP_BYTES = 80;

    for (uint64_t  i = 1; i < chunks.size(); ++i) {
        // 获取前一个 chunk 的尾部最多 50 字节（保证 UTF-8 完整性）
        std::string suffix = UTF8Helper::getLastBytesSafe(chunks[i-1].text, OVERLAP_BYTES);
        
        // 如果确实有内容，则插入到当前 chunk 的 text 开头
        if (!suffix.empty()) {
            chunks[i].text = suffix + chunks[i].text;
        }
    }
}