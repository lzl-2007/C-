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
    
    

    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());

    // 分配并设置所有缺失字段
    std::vector<llama_pos> pos(tokens.size());
    std::vector<int32_t> n_seq_id(tokens.size(), 1);
    std::vector<llama_seq_id*> seq_id(tokens.size());
    std::vector<llama_seq_id> seq_id_data(tokens.size(), 0);
    std::vector<int8_t> logits(tokens.size(), 0);

    for (size_t i = 0; i < tokens.size(); i++) {
        pos[i] = i;                    // 位置从 0 开始
        seq_id[i] = &seq_id_data[i];  // 每个 token 属于序列 0
        logits[i] = (i == tokens.size() - 1) ? 1 : 0;  // 只要最后一个输出
    }

    batch.pos = pos.data();
    batch.n_seq_id = n_seq_id.data();
    batch.seq_id = seq_id.data();
    batch.logits = logits.data();


    
    llama_context *ctx = getContext();
    if (ctx == nullptr) {
        llama_batch_free(batch);
        throw std::runtime_error("Context is null!");
    }

    int ret = llama_encode(ctx, batch);

    if (ret != 0 && ret != 1) {
        llama_batch_free(batch);
        throw std::runtime_error("llama_encode failed");
    }

 
  
    std::cout.flush();

    // 测试 1: 获取模型信息
    const llama_model *model = llama_get_model(ctx);
    int emb_dim = llama_n_embd(model);
    
    std::cout.flush();

  
    std::cout.flush();
    const float *all_emb = llama_get_embeddings(ctx);
    
    std::cout.flush();


    std::cout.flush();
    const float *seq_emb = llama_get_embeddings_seq(ctx, 0);
  
    std::cout.flush();

    // 测试 4: 尝试获取第 i 个 token 的 embedding
  
    std::cout.flush();
    const float *ith_emb = llama_get_embeddings_ith(ctx, 0);
    
    std::cout.flush();

    // 测试 5: 尝试读取第一个元素
    if (all_emb != nullptr) {
     
        std::cout.flush();
        float val = all_emb[0];
  
        std::cout.flush();
    }

    // 测试 6: 安全复制
    if (seq_emb != nullptr) {
      
        std::cout.flush();
        std::vector<float> result(seq_emb, seq_emb + emb_dim);
     
        return result;
    } else if (ith_emb != nullptr) {
       
        std::cout.flush();
        std::vector<float> result(ith_emb, ith_emb + emb_dim);
      
        return result;
    } else if (all_emb != nullptr) {
       
        std::cout.flush();
        std::vector<float> result(all_emb, all_emb + emb_dim);
     
        return result;
    } else {
        std::cout << "所有 embedding 指针都是 NULL!" << std::endl;
    }
    llama_batch_free(batch);
    
}