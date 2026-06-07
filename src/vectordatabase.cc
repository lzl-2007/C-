#include <queue>
#include <vector>
#include <cmath>
#include "chunk.h" 
#include "vectordatabase.h"
// 假设 Chunk 定义在这里

// 检索结果：相似度 + Chunk指针

float compute_norm(const std::vector<float>& vec) {
    float sum = 0.0f;
    for (float v : vec) {
        sum += v * v;
    }
    return std::sqrt(sum);
}


std::vector<const chunk*> search(
    const std::vector<float>& query_emb,
    const std::vector<chunk>& database,
    int top_k
) {
    if (top_k <= 0 || database.empty()) {
        return {};
    }
    
    // 计算 query 向量的长度
    float query_norm = 0.0f;
    for (float v : query_emb) {
        query_norm += v * v;
    }
    query_norm = std::sqrt(query_norm);
    
    // 最小堆：堆顶是相似度最小的
    std::priority_queue<ScoredChunk, std::vector<ScoredChunk>, std::greater<ScoredChunk>> heap;
    
    // 遍历所有 Chunk
    for (const chunk& chunk1 : database) {
        // 计算点积
        float dot = 0.0f;
        for (size_t i = 0; i < query_emb.size(); ++i) {
            dot += query_emb[i] * chunk1.embedding[i];
        }
        
        // 计算余弦相似度
        float similarity = dot / (query_norm * compute_norm(chunk1.embedding));
        
        // 用最小堆维护 top_k
        if (heap.size() < static_cast<size_t>(top_k)) {
            heap.push({similarity, &chunk1});
        } else if (similarity > heap.top().similarity) {
            heap.pop();
            heap.push({similarity, &chunk1});
        }
    }
    
    // 从堆中取出结果（堆顶是最小的，需要反转顺序）
    std::vector<const chunk*> result;
    result.reserve(heap.size());
    while (!heap.empty()) {
        result.push_back(heap.top().chunk_);
        heap.pop();
    }
    
    // 反转，让相似度高的在前面
    std::reverse(result.begin(), result.end());
    
    return result;
}