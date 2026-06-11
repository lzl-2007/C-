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
        
        // 从嵌套结构中提取数据
        // 1. 模型路径和GPU层数（在 models 对象中）
        if (j.contains("models")) {
            const json& models = j["models"];
            if (models.contains("embedding")) config.emb_model_path = models["embedding"];
            if (models.contains("generation")) config.gen_model_path = models["generation"];
            if (models.contains("n_gpu_layers")) config.n_gpu_layers = models["n_gpu_layers"];
        }
        
        // 2. 文档切分参数（在 document 对象中）
        if (j.contains("document")) {
            const json& document = j["document"];
            if (document.contains("chunk_size")) config.chunk_size = document["chunk_size"];
            if (document.contains("overlap_size")) config.overlap_size = document["overlap_size"];
        }
        
        // 3. 检索参数（在 retrieval 对象中）
        if (j.contains("retrieval")) {
            const json& retrieval = j["retrieval"];
            if (retrieval.contains("top_k")) config.top_k = retrieval["top_k"];
        }
        
        // 4. 生成模型参数（在 generation 对象中）
        if (j.contains("generation")) {
            const json& generation = j["generation"];
            if (generation.contains("n_ctx")) config.gen_n_ctx = generation["n_ctx"];
            if (generation.contains("n_batch")) config.gen_n_batch = generation["n_batch"];
            if (generation.contains("n_ubatch")) config.gen_n_ubatch = generation["n_ubatch"];
            if (generation.contains("max_output_tokens")) config.max_output_tokens = generation["max_output_tokens"];
            if (generation.contains("temperature")) config.temperature = generation["temperature"];
            if (generation.contains("top_k")) config.top_k_sampler = generation["top_k"];
            if (generation.contains("top_p")) config.top_p = generation["top_p"];
        }
        
        // 5. 嵌入模型参数（在 embedding 对象中）
        if (j.contains("embedding")) {
            const json& embedding = j["embedding"];
            if (embedding.contains("n_ctx")) config.emb_n_ctx = embedding["n_ctx"];
            if (embedding.contains("n_batch")) config.emb_n_batch = embedding["n_batch"];
            if (embedding.contains("n_ubatch")) config.emb_n_ubatch = embedding["n_ubatch"];
        }
        
        std::cout << "配置文件加载成功: " << config_path << std::endl;
        
        // 调试输出，确认配置已正确加载
        std::cout << "加载的配置:" << std::endl;
        std::cout << "  emb_model_path: " << config.emb_model_path << std::endl;
        std::cout << "  gen_model_path: " << config.gen_model_path << std::endl;
        std::cout << "  n_gpu_layers: " << config.n_gpu_layers << std::endl;
        std::cout << "  emb_n_ctx: " << config.emb_n_ctx << std::endl;
        std::cout << "  emb_n_batch: " << config.emb_n_batch << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "配置文件解析失败: " << e.what() << std::endl;
        std::cerr << "使用默认配置" << std::endl;
    }
    
    return config;
}
// python convert_hf_to_gguf.py Qwen/Qwen3-Embedding-4B   --outfile C:/Users/lzl67/Desktop/mrag/models/Qwen3-Embedding-4B-f16.gguf   --outtype f16