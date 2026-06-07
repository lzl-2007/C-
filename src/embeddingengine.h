#pragma once

#include "llamamodel.h"
#include <vector>
#include <string>

class EmbeddingEngine : public LlamaModelBase {
public:
    EmbeddingEngine(const AppConfig& config);
    
    // 生成文本的嵌入向量
    std::vector<float> generateEmbedding(const std::string& text);
    
private:
    // 清理残缺 UTF-8 序列
    std::string sanitizeUtf8(const std::string& text);
};

