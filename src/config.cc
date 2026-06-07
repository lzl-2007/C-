#include "config.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

AppConfig AppConfig::load(const std::string& config_path) {
    AppConfig config;
    
    std::ifstream file(config_path);
    if (!file.is_open()) {
        std::cout << "提示: 未找到配置文件 " << config_path 
                  << "，使用默认配置" << std::endl;
        return config;
    }
    
    try {
        json j;
        file >> j;
        
        // 模型路径
        if (j.contains("emb_model_path")) config.emb_model_path = j["emb_model_path"];
        if (j.contains("gen_model_path")) config.gen_model_path = j["gen_model_path"];
        
        // GPU 层数
        if (j.contains("n_gpu_layers")) config.n_gpu_layers = j["n_gpu_layers"];
        
        // 文档切分
        if (j.contains("chunk_size")) config.chunk_size = j["chunk_size"];
        if (j.contains("overlap_size")) config.overlap_size = j["overlap_size"];
        
        // 检索参数
        if (j.contains("top_k")) config.top_k = j["top_k"];
        
        // 生成模型上下文
        if (j.contains("gen_n_ctx")) config.gen_n_ctx = j["gen_n_ctx"];
        if (j.contains("gen_n_batch")) config.gen_n_batch = j["gen_n_batch"];
        if (j.contains("gen_n_ubatch")) config.gen_n_ubatch = j["gen_n_ubatch"];
        if (j.contains("max_output_tokens")) config.max_output_tokens = j["max_output_tokens"];
        
        // 采样参数
        if (j.contains("temperature")) config.temperature = j["temperature"];
        if (j.contains("top_k_sampler")) config.top_k_sampler = j["top_k_sampler"];
        if (j.contains("top_p")) config.top_p = j["top_p"];
        
        // 嵌入模型上下文
        if (j.contains("emb_n_ctx")) config.emb_n_ctx = j["emb_n_ctx"];
        if (j.contains("emb_n_batch")) config.emb_n_batch = j["emb_n_batch"];
        if (j.contains("emb_n_ubatch")) config.emb_n_ubatch = j["emb_n_ubatch"];
        
        std::cout << "配置文件加载成功: " << config_path << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "配置文件解析失败: " << e.what() << std::endl;
        std::cerr << "使用默认配置" << std::endl;
    }
    
    return config;
}