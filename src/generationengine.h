#pragma once
#include "llamamodel.h"
#include "chunk.h"
#include <string>
#include <vector>
#include <functional>

class GenerationEngine : public LlamaModelBase {
public:
    GenerationEngine(const AppConfig& config);
    ~GenerationEngine();

    // 生成回答（流式输出）
    // 参数：
    //   query: 用户问题
    //   chunks: 检索到的相关文本块
    //   callback: 流式输出回调函数，每次生成一个 token 时调用
    // 返回：完整的回答文本
    std::string generateStream(
        const std::string& query,
        const std::vector<chunk>& chunks,
        std::function<void(const std::string&)> callback = nullptr
    );

    // 生成回答（非流式，返回完整文本）
    std::string generate(
        const std::string& query,
        const std::vector<chunk>& chunks
    );

private:
    // 格式化 chunks 为上下文字符串（带出处标注）
    std::string formatContext(const std::vector<chunk>& chunks) const;
    
    // 组装 ChatML 格式的 prompt
    std::string buildPrompt(const std::string& query, const std::string& context) const;
    
    // 检查 token 是否是结束标记
    bool isEogToken(llama_token token) const;
    
    // 采样一个 token
    llama_token sampleToken(llama_sampler* sampler);
    
    // 解码 token 为字符串
    std::string tokenToPiece(llama_token token);
    
    // 推理一个 batch
    bool decodeBatch(llama_batch& batch);
};

