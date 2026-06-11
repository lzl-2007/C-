#pragma once

#include <queue>
#include <vector>
#include <cmath>
#include "chunk.h"

struct ScoredChunk {
    float similarity;
    const chunk* chunk_;
    
    // 最小堆需要 greater 比较，所以定义 > 运算符
    bool operator>(const ScoredChunk& other) const {
        return similarity > other.similarity;
    }
};

std::vector<const chunk*> search(
    const std::vector<float>& query_emb,
    const std::vector<chunk>& database,
    int top_k
);