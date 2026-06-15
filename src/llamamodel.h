#pragma once
#include "../include/llama/llama.h"
#include "config.h"
#include <vector>
#include <string>

enum class ModelMode {
    Embedding,
    Generation
};
class LlamaModelBase {
public:
    // 构造函数
    LlamaModelBase(const AppConfig& config, ModelMode mode);
    
    // 析构函数
    ~LlamaModelBase();
    
    // 禁用拷贝
    LlamaModelBase(const LlamaModelBase&) = delete;
    LlamaModelBase& operator=(const LlamaModelBase&) = delete;
    
    // 允许移动
    LlamaModelBase(LlamaModelBase&& other) noexcept;
    LlamaModelBase& operator=(LlamaModelBase&& other) noexcept;
    
    // 获取上下文
    llama_context* getContext() const { return ctx_; }
    llama_model* getModel() const { return model_; }
    const llama_vocab* getVocab() const { return llama_model_get_vocab(model_); }
    
    int getNEmbd() const { return llama_model_n_embd(model_); }
    const AppConfig& getConfig() const { return config_; } 
    

    // tokenize 工具方法
    std::vector<llama_token> tokenize(const std::string& text, bool add_bos = true) const;
    
private:
    llama_model* model_;
    llama_context* ctx_;
    ModelMode mode_;
    AppConfig config_;    
    
    void loadModel(const AppConfig& config);
    void createContext(const AppConfig& config);
};

