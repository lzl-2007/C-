#include "llamamodel.h"
#include <stdexcept>
#include <iostream>
LlamaModelBase::LlamaModelBase(const AppConfig& config, ModelMode mode)
    : config_(config),      // 初始化 config_ 成员
      model_(nullptr),
      ctx_(nullptr),
      mode_(mode) {
    loadModel(config_);
    createContext(config_);
}
void LlamaModelBase::loadModel(const AppConfig& config) {
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = config.n_gpu_layers;  
    
    const std::string& model_path = (mode_ == ModelMode::Embedding) 
        ? config.emb_model_path   // 不是 config.models.embedding_path
        : config.gen_model_path;   // 不是 config.models.generation_path
    
    model_ = llama_model_load_from_file(model_path.c_str(), model_params);
    if (!model_) {
        throw std::runtime_error("Failed to load model: " + model_path);
    }
}

// createContext 函数
void LlamaModelBase::createContext(const AppConfig& config) {
    llama_context_params ctx_params = llama_context_default_params();
    
    if (mode_ == ModelMode::Embedding) {
        ctx_params.embeddings = true;
        ctx_params.n_ctx = config.emb_n_ctx;     
        ctx_params.n_batch = config.emb_n_batch;
        ctx_params.n_ubatch = config.emb_n_ubatch;
    } else {
        ctx_params.embeddings = false;
        ctx_params.n_ctx = config.gen_n_ctx;      
        ctx_params.n_batch = config.gen_n_batch;
        ctx_params.n_ubatch = config.gen_n_ubatch;
    }
    
    ctx_ = llama_init_from_model(model_, ctx_params);
    if (!ctx_) {
        throw std::runtime_error("Failed to create context");
    }
}
LlamaModelBase::~LlamaModelBase() {
    if (ctx_) {
        llama_free(ctx_);
        ctx_ = nullptr;
    }
    if (model_) {
        llama_free_model(model_);
        model_ = nullptr;
    }
}
std::vector<llama_token> LlamaModelBase::tokenize(const std::string& text, bool add_special) const {
    if (!model_) {
        throw std::runtime_error("tokenize: model_ is null");
    }
    
    const llama_vocab* vocab = llama_model_get_vocab(model_);
    if (!vocab) {
        throw std::runtime_error("tokenize: vocab is null");
    }
    // 空文本直接返回
    if (text.empty()) {
        return {};
    }
    
    // 第一次调用：获取需要的 token 数量
    std::cout << "    [tokenize] 第一次调用，获取所需大小" << std::endl;
    int n_tokens = llama_tokenize(vocab, text.c_str(), text.size(), nullptr, 0, add_special, true);
    std::cout << "    [tokenize] 第一次返回: n_tokens = " << n_tokens << std::endl;
    
    if (n_tokens < 0) {
        std::cout<<"wrong";
        throw std::runtime_error("Tokenization failed: " + std::to_string(n_tokens));
    }
    
    
    std::vector<llama_token> tokens(n_tokens);
    
    int actual = llama_tokenize(vocab, text.c_str(), text.size(), tokens.data(), tokens.size(), add_special, true);
    
    if (actual < 0) {
        throw std::runtime_error("Tokenization failed on second call");
    }
    if (actual != n_tokens) {
        tokens.resize(actual);
    }
    return tokens;
}