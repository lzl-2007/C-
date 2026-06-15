#include "generationengine.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>

GenerationEngine::GenerationEngine(const AppConfig& config)
    : LlamaModelBase(config, ModelMode::Generation) {
}

GenerationEngine::~GenerationEngine() {
    // 基类析构函数会释放资源
}

std::string GenerationEngine::formatContext(const std::vector<chunk>& chunks) const {
    if (chunks.empty()) {
        return "没有找到chunks";
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < chunks.size(); ++i) {
        oss << "[" << i + 1 << "] " << chunks[i].metadata << ":\n";
        oss << chunks[i].text << "\n\n";
    }
    return oss.str();
}

std::string GenerationEngine::buildPrompt(const std::string& query, const std::string& context) const {
    std::ostringstream oss;
    oss << "<|im_start|>system\n";
    oss << "你是一个严谨的小说阅读助手。请你【必须并且只能】根据下面提供的[参考上下文]来回答用户的问题。\n";
    oss << "如果上下文中有答案，请详细回答并引用出处；如果上下文中没有提到相关信息，请直接回答\"根据当前片段无法确定答案\"。\n";
    oss << "<|im_end|>\n";
    oss << "<|im_start|>user\n";
    oss << "[参考上下文]：\n";
    oss << context << "\n";
    oss << "用户问题：" << query << "\n";
    oss << "<|im_end|>\n";
    oss << "<|im_start|>assistant\n";
    return oss.str();
}

bool GenerationEngine::isEogToken(llama_token token) const {
    const llama_vocab* vocab = getVocab();
    return llama_vocab_is_eog(vocab, token);
}

llama_token GenerationEngine::sampleToken(llama_sampler* sampler) {
    return llama_sampler_sample(sampler, getContext(), -1);
}

std::string GenerationEngine::tokenToPiece(llama_token token) {
    const llama_vocab* vocab = getVocab();
    char buf[256];
    int n = llama_token_to_piece(vocab, token, buf, sizeof(buf), 0, true);
    if (n < 0) {
        return "";
    }
    return std::string(buf, n);
}

bool GenerationEngine::decodeBatch(llama_batch& batch) {
    int ret = llama_decode(getContext(), batch);
    if (ret != 0) {
        std::cerr << "llama_decode failed with code: " << ret << std::endl;
        return false;
    }
    return true;
}

std::string GenerationEngine::generateStream(
    const std::string& query,
    const std::vector<chunk>& chunks,
    std::function<void(const std::string&)> callback
) {
    //清空 KV Cache
    llama_memory_t memory = llama_get_memory(getContext());
    llama_memory_clear(memory, true);
    
    //格式化上下文
    std::string context = formatContext(chunks);
    
    //组装 prompt
    std::string prompt = buildPrompt(query, context);
    
    //Tokenize
    std::vector<llama_token> prompt_tokens = tokenize(prompt, false);
    if (prompt_tokens.empty()) {
        throw std::runtime_error("Failed to tokenize prompt");
    }
    
    //Prompt Prefill：将所有 token 装入 batch，仅最后一个 token 开启 logits
    llama_batch batch = llama_batch_init(prompt_tokens.size(), 0, 1);
    for (size_t i = 0; i < prompt_tokens.size(); ++i) {
        batch.token[i] = prompt_tokens[i];
        batch.pos[i] = i;
        batch.seq_id[i][0] = 0;
        batch.n_seq_id[i] = 1;
        // 只有最后一个 token 需要 logits 用于采样
        batch.logits[i] = (i == prompt_tokens.size() - 1) ? 1 : 0;
    }
    batch.n_tokens = prompt_tokens.size();
    
    // 执行 prefill
    if (!decodeBatch(batch)) {
        llama_batch_free(batch);
        throw std::runtime_error("Prefill failed");
    }
    
    //初始化采样链
    llama_sampler_chain_params sampler_params = llama_sampler_chain_default_params();
    llama_sampler* sampler = llama_sampler_chain_init(sampler_params);
    
    llama_sampler_chain_add(sampler, llama_sampler_init_top_k(getConfig().top_k_sampler));
    llama_sampler_chain_add(sampler, llama_sampler_init_top_p(getConfig().top_p, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(getConfig().temperature));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
    
    //自回归生成循环
    std::string response;
    int n_ctx = llama_n_ctx(getContext());
    int n_generated = 0;
    int max_tokens = getConfig().max_output_tokens;
    llama_token current_token = prompt_tokens.back();
    
    // 准备单个 token 的 batch
    llama_batch single_batch = llama_batch_init(1, 0, 1);
    
    for (int i = 0; i < max_tokens; ++i) {
        // 采样
        current_token = sampleToken(sampler);
        
        // 检查结束标记
        if (isEogToken(current_token)) {
            break;
        }
        
        // 解码并输出
        std::string piece = tokenToPiece(current_token);
        response += piece;
        if (callback) {
            callback(piece);
        }
        
        // 准备下一个 batch（只包含新生成的 token）
        single_batch.token[0] = current_token;
        single_batch.pos[0] = n_generated + prompt_tokens.size();
        single_batch.seq_id[0][0] = 0;
        single_batch.n_seq_id[0] = 1;
        single_batch.logits[0] = 1;  // 需要 logits 继续采样
        single_batch.n_tokens = 1;
        
        // 解码
        if (!decodeBatch(single_batch)) {
            std::cerr << "Decode failed at step " << i << std::endl;
            break;
        }
        
        n_generated++;
        
        // 检查上下文空间
        int n_ctx_used = llama_memory_seq_pos_max(memory, 0) + 1;
        if (n_ctx_used >= n_ctx - 10) {
            std::cerr << "Warning: Context limit reached" << std::endl;
            break;
        }
    }
    
    // 8. 释放 sampler 与 batch
    llama_sampler_free(sampler);
    llama_batch_free(batch);
    llama_batch_free(single_batch);
    
    return response;
}

std::string GenerationEngine::generate(
    const std::string& query,
    const std::vector<chunk>& chunks
) {
    std::string result;
    generateStream(query, chunks, [&](const std::string& piece) {
        result += piece;
        std::cout << piece;
        std::cout.flush();
    });
    std::cout << std::endl;
    return result;
}