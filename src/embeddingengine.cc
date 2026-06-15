#include "embeddingengine.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <fstream>
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
        i++;
    }
    
    return result;
}

std::vector<float> EmbeddingEngine::generateEmbedding(const std::string& text) {
    
    std::string clean_text = sanitizeUtf8(text);
    
    if (clean_text.empty()) {
        return std::vector<float>(getNEmbd(), 0.0f);
    }
    std::vector<llama_token> tokens = tokenize(clean_text, true);
    int max_tokens = std::min(getConfig().emb_n_ctx, getConfig().emb_n_batch);
    if (static_cast<int>(tokens.size()) > max_tokens) {
        tokens.resize(max_tokens);
    }
    
    if (tokens.empty()) {
        return std::vector<float>(getNEmbd(), 0.0f);
    }
    llama_memory_t memory = llama_get_memory(getContext());
    llama_memory_clear(memory, true);
    
   //开始构造 batch"
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    if (batch.n_tokens > 0) {
        batch.logits[batch.n_tokens - 1] = 1;
    }
   
    
    // 开始 llama_decode
    int ret = llama_decode(getContext(), batch);
    
    if (ret != 0) {
        llama_batch_free(batch);
        throw std::runtime_error("llama_decode failed in generateEmbedding");
    }
    
    const float* emb = nullptr;
    int emb_dim = getNEmbd();
    
    emb = llama_get_embeddings_seq(getContext(), 0);
    if (!emb) {
        emb = llama_get_embeddings(getContext());
    }
    
    if (!emb) {
        llama_batch_free(batch);
        throw std::runtime_error("Failed to get embeddings");
    }
    
    std::vector<float> result(emb, emb + emb_dim);
    llama_batch_free(batch);
    return result;
}