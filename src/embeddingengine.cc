#include "embeddingengine.h"
#include <algorithm>
#include <cstring>
#include<iostream>
EmbeddingEngine::EmbeddingEngine(const AppConfig& config)
    : LlamaModelBase(config, ModelMode::Embedding) {
}

std::string EmbeddingEngine::sanitizeUtf8(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = text[i];
        
        // ASCII
        if (c < 0x80) {
            result.push_back(c);
            i++;
            continue;
        }
        
        // 2字节 UTF-8
        if ((c & 0xE0) == 0xC0 && i + 1 < text.size()) {
            unsigned char c2 = text[i+1];
            if ((c2 & 0xC0) == 0x80) {
                result.push_back(c);
                result.push_back(c2);
                i += 2;
                continue;
            }
        }
        
        // 3字节 UTF-8 (汉字)
        if ((c & 0xF0) == 0xE0 && i + 2 < text.size()) {
            unsigned char c2 = text[i+1];
            unsigned char c3 = text[i+2];
            if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80) {
                result.push_back(c);
                result.push_back(c2);
                result.push_back(c3);
                i += 3;
                continue;
            }
        }
        
        // 4字节 UTF-8 (Emoji等)
        if ((c & 0xF8) == 0xF0 && i + 3 < text.size()) {
            unsigned char c2 = text[i+1];
            unsigned char c3 = text[i+2];
            unsigned char c4 = text[i+3];
            if ((c2 & 0xC0) == 0x80 && (c3 & 0xC0) == 0x80 && (c4 & 0xC0) == 0x80) {
                result.push_back(c);
                result.push_back(c2);
                result.push_back(c3);
                result.push_back(c4);
                i += 4;
                continue;
            }
        }
        
        // 非法字节，跳过
        i++;
    }
    
    return result;
}

std::vector<float> EmbeddingEngine::generateEmbedding(const std::string& text) {
    // 1. 清理残缺 UTF-8 序列
    std::string clean_text = sanitizeUtf8(text);
    if (clean_text.empty()) {
        return std::vector<float>(llama_model_n_embd(getModel()), 0.0f);
    }
    
    // 2. Tokenize 并截断
    std::vector<llama_token> tokens = tokenize(clean_text, true);
    
    int max_tokens = std::min(getConfig().emb_n_ctx, getConfig().emb_n_batch);
    if (static_cast<int>(tokens.size()) > max_tokens) {
        tokens.resize(max_tokens);
    }
    
    if (tokens.empty()) {
        return std::vector<float>(llama_model_n_embd(getModel()), 0.0f);
    }
    
    // 3. 清空 KV Cache
    llama_memory_t memory = llama_get_memory(getContext());
    llama_memory_clear(memory, true);
    
    // 4. 构造 batch（最后一个 token 的 logits 为 1）
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    // 确保最后一个 token 的 logits 为 1（默认就是1，但显式设置）
    if (batch.n_tokens > 0) {
        batch.logits[batch.n_tokens - 1] = 1;
    }
    
    // 5. 执行推理
    int ret = llama_decode(getContext(), batch);
    if (ret != 0) {
        llama_batch_free(batch);
        throw std::runtime_error("llama_decode failed in generateEmbedding");
    }
    
    // 6. 获取嵌入向量
    const float* emb = nullptr;
    int emb_dim = llama_model_n_embd(getModel());
    
    // 优先使用 llama_get_embeddings_seq
    emb = llama_get_embeddings_seq(getContext(), 0);
    if (!emb) {
        // 回退到 llama_get_embeddings
        emb = llama_get_embeddings(getContext());
    }
    
    if (!emb) {
        llama_batch_free(batch);
        throw std::runtime_error("Failed to get embeddings");
    }
    
    // 7. 复制到 vector 并返回
    std::vector<float> result(emb, emb + emb_dim);
    
    // 释放 batch 资源
    llama_batch_free(batch);
    
    return result;
}
int main(){
    std::string a="dsfcfsvbgdv dfsv但是分手厨房测";
    AppConfig d;
    EmbeddingEngine b(d);
    std::vector<float> c=b.generateEmbedding(a);
    for (auto i :c){
        std::cout<<i<<" ";
    }

}