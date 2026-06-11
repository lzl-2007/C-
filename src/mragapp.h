#pragma once
#include "config.h"
#include "chunk.h"
#include "vectordatabase.h"
#include "embeddingengine.h"
#include "generationengine.h"
#include <string>
#include <memory>
#include <vector>
#include <functional>

class MragApp {
public:
    // 构造函数
    MragApp(const AppConfig& config);
    
    // 析构函数
    ~MragApp();
    
    // 禁用拷贝
    MragApp(const MragApp&) = delete;
    MragApp& operator=(const MragApp&) = delete;
    
    // 加载文档并处理
    bool loadDocument(const std::string& filepath);
    
    // 查询接口
    std::string query(const std::string& question);
    
    // 流式查询接口
    void queryStream(const std::string& question, std::function<void(const std::string&)> callback);
    
    // 获取配置
    const AppConfig& getConfig() const { return config_; }
    
    // 获取块数量
    size_t getChunkCount() const { return chunks_.size(); }
    
    // 保存块数据到文件
    bool saveChunks(const std::string& filepath);
    
    // 从文件加载块数据
    bool loadChunks(const std::string& filepath);
    
private:
    AppConfig config_;
    std::unique_ptr<EmbeddingEngine> embedding_engine_;
    std::unique_ptr<GenerationEngine> generation_engine_;
    std::vector<chunk> chunks_;
    
    // 为所有块生成嵌入向量
    void generateEmbeddingsForAllChunks();
    
    // 内部辅助方法
    std::vector<float> generateQueryEmbedding(const std::string& query);
    std::vector<const chunk*> retrieveRelevantChunks(const std::vector<float>& query_emb);
};