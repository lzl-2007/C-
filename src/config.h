#pragma once

#include <string>
#include <cstddef>
//默认参数
struct AppConfig {
    // 模型路径
    std::string emb_model_path = "C:/Users/lzl67/Desktop/mrag/models/bge-small-zh-v1.5-f16.gguf";
    std::string gen_model_path = "C:/Users/lzl67/Desktop/mrag/models/qwen2.5-7b-instruct-q4_k_m.gguf";
    
    // GPU 层数
    int n_gpu_layers = 99;
    
    // 文档切分
    size_t chunk_size = 500;
    size_t overlap_size = 50;
    
    // 检索参数
    int top_k = 3;
    
    // 生成模型上下文
    int gen_n_ctx = 4096;
    int gen_n_batch = 4096;
    int gen_n_ubatch = 512;
    int max_output_tokens = 512;
    
    // 采样参数
    float temperature = 0.7f;
    int top_k_sampler = 40;
    float top_p = 0.9f;
    
    // 嵌入模型上下文
    int emb_n_ctx = 512;
    int emb_n_batch = 512;
    int emb_n_ubatch = 512;
    
    // 从 JSON 文件加载配置
    static AppConfig load(const std::string& config_path = "C:/Users/lzl67/Desktop/mrag//config.json");
};

