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
        
        // 非法字节，跳过
        i++;
    }
    
    return result;
}
/*
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
}*/
std::vector<float> EmbeddingEngine::generateEmbedding(const std::string& text) {
    std::cout << "  [1] 开始 sanitizeUtf8" << std::endl;
    std::string clean_text = sanitizeUtf8(text);
    std::cout << "  [2] sanitizeUtf8 完成，长度: " << clean_text.size() << std::endl;
    
    if (clean_text.empty()) {
        std::cout << "  [2.5] 文本为空，返回零向量" << std::endl;
        return std::vector<float>(getNEmbd(), 0.0f);
    }
    
    std::cout << "  [3] 开始 tokenize" << std::endl;

    std::cout << "  [2.6] 调用 tokenize 前，this = " << this << std::endl;
    std::cout << "  [2.7] clean_text 地址: " << &clean_text << std::endl;
    std::cout << "  [2.8] clean_text 内容: " << clean_text << std::endl;
    std::cout << "  [2.9] 准备调用 tokenize" << std::endl;

    

    

    std::vector<llama_token> tokens = tokenize(clean_text, true);

    std::cout << "  [2.10] tokenize 返回" << std::endl;
    std::cout << "  [4] tokenize 完成，token 数量: " << tokens.size() << std::endl;
    
    int max_tokens = std::min(getConfig().emb_n_ctx, getConfig().emb_n_batch);
    std::cout << "  [5] max_tokens = " << max_tokens << std::endl;
    
    if (static_cast<int>(tokens.size()) > max_tokens) {
        tokens.resize(max_tokens);
        std::cout << "  [6] token 被截断到 " << max_tokens << std::endl;
    }
    
    if (tokens.empty()) {
        std::cout << "  [7] token 为空，返回零向量" << std::endl;
        return std::vector<float>(getNEmbd(), 0.0f);
    }
    
    std::cout << "  [8] 开始清空 KV Cache" << std::endl;
    llama_memory_t memory = llama_get_memory(getContext());
    llama_memory_clear(memory, true);
    std::cout << "  [9] KV Cache 清空完成" << std::endl;
    
    std::cout << "  [10] 开始构造 batch" << std::endl;
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    if (batch.n_tokens > 0) {
        batch.logits[batch.n_tokens - 1] = 1;
    }
    std::cout << "  [11] batch 构造完成，n_tokens = " << batch.n_tokens << std::endl;
    
    std::cout << "  [12] 开始 llama_decode" << std::endl;
    int ret = llama_decode(getContext(), batch);
    std::cout << "  [13] llama_decode 返回: " << ret << std::endl;
    
    if (ret != 0) {
        llama_batch_free(batch);
        throw std::runtime_error("llama_decode failed in generateEmbedding");
    }
    
    std::cout << "  [14] 开始获取 embedding" << std::endl;
    const float* emb = nullptr;
    int emb_dim = getNEmbd();
    
    emb = llama_get_embeddings_seq(getContext(), 0);
    if (!emb) {
        std::cout << "  [15] llama_get_embeddings_seq 失败，回退到 llama_get_embeddings" << std::endl;
        emb = llama_get_embeddings(getContext());
    }
    
    if (!emb) {
        llama_batch_free(batch);
        throw std::runtime_error("Failed to get embeddings");
    }
    
    std::cout << "  [16] 获取成功，维度: " << emb_dim << std::endl;
    
    std::vector<float> result(emb, emb + emb_dim);
    llama_batch_free(batch);
    
    std::cout << "  [17] generateEmbedding 完成" << std::endl;
    return result;
}



/*
int main(){
    std::cout << "1. 开始加载配置" << std::endl;
    // 尝试多个可能的配置文件路径
    std::string config_path = "config.json";
    AppConfig config;
    
    // 先尝试当前目录
    std::ifstream test1(config_path);
    if (test1.is_open()) {
        test1.close();
        config = AppConfig::load(config_path);
    } else {
        // 尝试上一级目录
        config_path = "C:/Users/lzl67/Desktop/mrag/config.json";
        std::ifstream test2(config_path);
        if (test2.is_open()) {
            test2.close();
            config = AppConfig::load(config_path);
        } else {
            // 最后使用默认配置
            std::cout << "提示: 未找到配置文件，使用默认配置" << std::endl;
            config = AppConfig(); // 使用默认构造函数
        }
    }
    
    std::cout << "2. 配置加载完成" << std::endl;
    std::cout << "   嵌入模型: " << config.emb_model_path << std::endl;
    std::cout << "   生成模型: " << config.gen_model_path << std::endl;
    
    std::cout << "3. 创建 EmbeddingEngine" << std::endl;
    EmbeddingEngine engine(config);
    
    std::cout << "4. EmbeddingEngine 创建成功" << std::endl;
    
    std::string test_text = "你好";
    std::cout << "5. 测试文本: " << test_text << std::endl;
    
    std::cout << "6. 开始生成 embedding..." << std::endl;
    std::vector<float> emb = engine.generateEmbedding(test_text);
    
    std::cout << "7. 生成完成，维度: " << emb.size() << std::endl;
    
    std::cout << "8. 程序正常结束" << std::endl;
    return 0;


    std::string a="dsfcfsvbgdv dfsv但是分手厨房测";
    AppConfig d;
    EmbeddingEngine b(d);
    std::vector<float> c=b.generateEmbedding(a);
    for (auto i :c){
        std::cout<<i<<" ";
    }

}*/