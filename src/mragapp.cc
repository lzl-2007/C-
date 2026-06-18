#include "mragapp.h"
#include "document.h"
#include "vectordatabase.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <functional>

// 全局文档分块函数声明
extern std::vector<chunk> chunks; // 来自 document.h

MragApp::MragApp(const AppConfig& config)
    : config_(config) {
    
    std::cout << "初始化 MragApp..." << std::endl;
    
    try {
        // 初始化嵌入引擎
        std::cout << "初始化嵌入引擎..." << std::endl;
        embedding_engine_ = std::make_unique<EmbeddingEngine>(config_);
        
        // 初始化生成引擎
        std::cout << "初始化生成引擎..." << std::endl;
        generation_engine_ = std::make_unique<GenerationEngine>(config_);
        
        std::cout << "MragApp 初始化完成" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "初始化失败: " << e.what() << std::endl;
        throw;
    }
}

MragApp::~MragApp() {
    std::cout << "清理 MragApp..." << std::endl;
}

bool MragApp::loadDocument(const std::string& filepath) {
    try {
        std::cout << "加载文档: " << filepath << std::endl;
        
        // 清空现有块
        chunks_.clear();
        
        // 使用 StreamingUTF8Splitter 进行分块
        std::cout << "开始文档分块..." << std::endl;
        
        StreamingUTF8Splitter::splitByCharCount(filepath, config_.chunk_size);
        
        // 从全局变量获取分块结果
        std::cout << "分块完成，获取块数据..." << std::endl;
        chunks_ = chunks; // 使用全局变量
        ChunkOverlapProcessor::process(chunks);
        std::cout << "共生成 " << chunks_.size() << " 个文本块" << std::endl;
        
        // 为所有块生成嵌入向量
        std::cout << "开始生成嵌入向量..." << std::endl;
        generateEmbeddingsForAllChunks();
        
        std::cout << "文档加载完成" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载文档失败: " << e.what() << std::endl;
        return false;
    }
}

void MragApp::generateEmbeddingsForAllChunks() {
    
    if (chunks_.empty()) {
        std::cerr << "警告: 没有文本块需要生成嵌入向量" << std::endl;
        return;
    }
    
    std::cout << "为 " << chunks_.size() << " 个文本块生成嵌入向量..." << std::endl;
    
    for (size_t i = 0; i < chunks_.size(); ++i) {
        try {
            
            std::vector<float> embedding = embedding_engine_->generateEmbedding(chunks_[i].text);
            std::cout<<"嵌入向量。。。";
            chunks_[i].embedding = std::move(embedding);
            
            if (i % 100 == 0 || i == chunks_.size() - 1) {
                std::cout << "进度: " << i + 1 << "/" << chunks_.size() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "为块 " << i << " 生成嵌入向量失败: " << e.what() << std::endl;
            // 清空失败的嵌入向量
            chunks_[i].embedding.clear();
        }
    }
    
    // 过滤掉嵌入向量生成失败的块
    auto it = std::remove_if(chunks_.begin(), chunks_.end(),
        [](const chunk& c) { return c.embedding.empty(); });
    chunks_.erase(it, chunks_.end());
    
    std::cout << "嵌入向量生成完成，有效块数: " << chunks_.size() << std::endl;
}

std::vector<float> MragApp::generateQueryEmbedding(const std::string& query) {
    try {
        return embedding_engine_->generateEmbedding(query);
    } catch (const std::exception& e) {
        std::cerr << "生成查询嵌入向量失败: " << e.what() << std::endl;
        throw;
    }
}

std::vector<const chunk*> MragApp::retrieveRelevantChunks(const std::vector<float>& query_emb) {
    if (chunks_.empty()) {
        std::cerr << "警告: 数据库为空，无法检索" << std::endl;
        return {};
    }
    
    if (query_emb.empty()) {
        std::cerr << "错误: 查询嵌入向量为空" << std::endl;
        return {};
    }
    
    try {
        std::cout << "检索相关文本块 (top_k=" << config_.top_k << ")..." << std::endl;
        std::vector<const chunk*> results = search(query_emb, chunks_, config_.top_k);
        std::cout << "检索完成，找到 " << results.size() << " 个相关块" << std::endl;
        return results;
    } catch (const std::exception& e) {
        std::cerr << "检索失败: " << e.what() << std::endl;
        return {};
    }
}

std::string MragApp::query(const std::string& question) {
    if (chunks_.empty()) {
        return "错误: 请先加载文档";
    }
    
    try {
        // 1. 生成查询嵌入向量
        std::cout << "生成查询嵌入向量..." << std::endl;
        std::vector<float> query_emb = generateQueryEmbedding(question);
        
        // 2. 检索相关块
        std::vector<const chunk*> relevant_chunks = retrieveRelevantChunks(query_emb);
        
        // 将指针转换为引用
        std::vector<chunk> chunks_ref;
        chunks_ref.reserve(relevant_chunks.size());
        for (const chunk* c : relevant_chunks) {
            if (c) chunks_ref.push_back(*c);
        }
        
        // 3. 生成回答
        std::cout << "生成回答..." << std::endl;
        return generation_engine_->generateStream(question, chunks_ref, nullptr);
        
    } catch (const std::exception& e) {
        std::cerr << "查询失败: " << e.what() << std::endl;
        return "查询失败: " + std::string(e.what());
    }
}

void MragApp::queryStream(const std::string& question, std::function<void(const std::string&)> callback) {
    if (chunks_.empty()) {
        callback("错误: 请先加载文档");
        return;
    }
    
    try {
        // 1. 生成查询嵌入向量
        std::vector<float> query_emb = generateQueryEmbedding(question);
        
        // 2. 检索相关块
        std::vector<const chunk*> relevant_chunks = retrieveRelevantChunks(query_emb);
        
        // 将指针转换为引用
        std::vector<chunk> chunks_ref;
        chunks_ref.reserve(relevant_chunks.size());
        for (const chunk* c : relevant_chunks) {
            if (c) chunks_ref.push_back(*c);
        }
        
        // 3. 流式生成回答
        generation_engine_->generateStream(question, chunks_ref, callback);
        
    } catch (const std::exception& e) {
        std::cerr << "流式查询失败: " << e.what() << std::endl;
        callback("查询失败: " + std::string(e.what()));
    }
}

bool MragApp::saveChunks(const std::string& filepath) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out) {
        std::cerr << "无法打开文件保存块数据: " << filepath << std::endl;
        return false;
    }
    
    try {
        // 写入块数量
        uint64_t num_chunks = chunks_.size();
        out.write(reinterpret_cast<const char*>(&num_chunks), sizeof(num_chunks));
        
        // 写入每个块
        for (const auto& c : chunks_) {
            c.serialize(out);
        }
        
        std::cout << "保存了 " << chunks_.size() << " 个块到 " << filepath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "保存块数据失败: " << e.what() << std::endl;
        return false;
    }
}

bool MragApp::loadChunks(const std::string& filepath) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in) {
        std::cerr << "无法打开文件加载块数据: " << filepath << std::endl;
        return false;
    }
    
    try {
        // 读取块数量
        uint64_t num_chunks = 0;
        in.read(reinterpret_cast<char*>(&num_chunks), sizeof(num_chunks));
        
        // 清空现有块
        chunks_.clear();
        chunks_.reserve(num_chunks);
        
        // 读取每个块
        for (uint64_t i = 0; i < num_chunks; ++i) {
            chunk c = chunk::deserialize(in);
            chunks_.push_back(std::move(c));
        }
        
        std::cout << "从 " << filepath << " 加载了 " << chunks_.size() << " 个块" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "加载块数据失败: " << e.what() << std::endl;
        return false;
    }
}