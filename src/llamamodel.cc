// llamamodelbase.cc 中的 loadModel 函数
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
    model_params.n_gpu_layers = config.n_gpu_layers;  // 直接访问，不是 config.models.n_gpu_layers
    
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
        //ctx_params.pooling_type = LLAMA_POOLING_TYPE_MEAN;
        ctx_params.n_ctx = config.emb_n_ctx;      // 不是 config.embedding.n_ctx
        ctx_params.n_batch = config.emb_n_batch;
        ctx_params.n_ubatch = config.emb_n_ubatch;
    } else {
        ctx_params.embeddings = false;
        ctx_params.n_ctx = config.gen_n_ctx;      // 不是 config.generation.n_ctx
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
/*
std::vector<llama_token> LlamaModelBase::tokenize(const std::string& text, bool add_bos) const {
    const llama_vocab* vocab = llama_model_get_vocab(model_);
    
    // 第一次调用：获取需要的 token 数量
    int n_tokens = llama_tokenize(vocab, text.c_str(), text.size(), nullptr, 0, add_bos, false);
    if (n_tokens < 0) {
        throw std::runtime_error("Tokenization failed");
    }
    
    // 第二次调用：填充 token 数组
    std::vector<llama_token> tokens(n_tokens);
    n_tokens = llama_tokenize(vocab, text.c_str(), text.size(), tokens.data(), tokens.size(), add_bos, false);
    if (n_tokens < 0) {
        throw std::runtime_error("Tokenization failed");
    }
    
    return tokens;
}*/
/*
std::vector<llama_token> LlamaModelBase::tokenize(const std::string& text, bool add_bos) const {
    std::cout << "    [tokenize] 进入函数" << std::endl;
    
    if (!model_) {
        throw std::runtime_error("tokenize: model_ is null");
    }
    std::string processed_text = text;
    std::cout << "    [tokenize] 获取 vocab" << std::endl;
    const llama_vocab* vocab = llama_model_get_vocab(model_);
    if (!vocab) {
        throw std::runtime_error("tokenize: vocab is null");
    }
    
    std::cout << "    [tokenize] 使用 common_tokenize (如果可用)" << std::endl;
    
    // 方法 A: 使用 common_tokenize（需要包含 common.h）
    // std::vector<llama_token> tokens = common_tokenize(ctx, text, add_bos);
    
    // 方法 B: 手动指定 special = true
    std::cout << "    [tokenize] 第一次 llama_tokenize (special = true)" << std::endl;
    int n_tokens =llama_tokenize(vocab, processed_text.c_str(), processed_text.size(), nullptr, 0, add_bos, true);
    std::cout << "    [tokenize] 第一次返回: n_tokens = " << n_tokens << std::endl;
    
    if (n_tokens < 0) {
        // 尝试 special = false
        std::cout << "    [tokenize] 重试 special = false" << std::endl;
        n_tokens = llama_tokenize(vocab, processed_text.c_str(), processed_text.size(), nullptr, 0, add_bos, false);
        std::cout << "    [tokenize] 重试返回: n_tokens = " << n_tokens << std::endl;
        
        if (n_tokens < 0) {
            throw std::runtime_error("Tokenization failed with both special=true and false");
        }
    }
    
    
    std::vector<llama_token> tokens = tokenize(processed_text, false);  // add_bos = false
    
    //std::vector<llama_token> tokens(n_tokens);
    n_tokens = llama_tokenize(vocab, processed_text.c_str(), processed_text.size(), tokens.data(), tokens.size(), add_bos, true);
    if (n_tokens < 0) {
        throw std::runtime_error("Tokenization failed on second call");
    }
    
    std::cout << "    [tokenize] 完成，token 数量: " << tokens.size() << std::endl;
    return tokens;
}*/
std::vector<llama_token> LlamaModelBase::tokenize(const std::string& text, bool add_special) const {
    std::cout << "    [tokenize] 进入函数" << std::endl;
    
    if (!model_) {
        throw std::runtime_error("tokenize: model_ is null");
    }
    
    std::cout << "    [tokenize] 获取 vocab" << std::endl;
    const llama_vocab* vocab = llama_model_get_vocab(model_);
    if (!vocab) {
        throw std::runtime_error("tokenize: vocab is null");
    }
    
    if (vocab) {
        int vocab_type = llama_vocab_type(vocab);  // 获取 vocab 类型
        std::cout << "vocab type: " << vocab_type << std::endl;
        
        // 常见类型：
        // 0 = LLAMA_VOCAB_TYPE_NONE
        // 1 = LLAMA_VOCAB_TYPE_SPM (SentencePiece)
        // 2 = LLAMA_VOCAB_TYPE_BPE (Byte Pair Encoding)
        // 3 = LLAMA_VOCAB_TYPE_WPM (WordPiece) ← BGE 模型应该是这个
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
    
    // 分配 tokens 数组
    std::vector<llama_token> tokens(n_tokens);
    
    // 第二次调用：填充 tokens
    int actual = llama_tokenize(vocab, text.c_str(), text.size(), tokens.data(), tokens.size(), add_special, true);
    std::cout << "    [tokenize] 第二次返回: actual = " << actual << std::endl;
    
    if (actual < 0) {
        throw std::runtime_error("Tokenization failed on second call");
    }
    
    // 调整到实际大小（正常情况下 actual == n_tokens）
    if (actual != n_tokens) {
        tokens.resize(actual);
    }
    
    std::cout << "    [tokenize] 完成，token 数量: " << tokens.size() << std::endl;
    return tokens;
}