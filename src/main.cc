#include "mragapp.h"
#include "config.h"
#include "../include/llama/llama.h"
#include <iostream>
#include <string>
#include <memory>
#include <cstdlib>
#include <clocale>
#include <cstring>

// 空日志回调，屏蔽 llama.cpp 底层日志
static void empty_log_callback(enum ggml_log_level level, const char * text, void * user_data) {
    // 什么也不做，完全屏蔽日志
}

// 显示使用说明
static void print_usage(const std::string& program_name) {
    std::cout << "\nMRAG (Multi-modal Retrieval Augmented Generation) 系统\n";
    std::cout << "用法:\n";
    std::cout << "  " << program_name << " --ingest <小说TXT路径> <输出数据库路径>\n";
    std::cout << "  " << program_name << " --chat <数据库路径>\n";
    std::cout << "  " << program_name << " --help\n";
    std::cout << "\n模式说明:\n";
    std::cout << "  --ingest  离线建库模式：处理小说文件，生成向量数据库\n";
    std::cout << "  --chat    在线问答模式：加载数据库，进入交互式问答\n";
    std::cout << "\n示例:\n";
    std::cout << "  " << program_name << " --ingest king3.txt king3.db\n";
    std::cout << "  " << program_name << " --chat king3.db\n";
    std::cout << "\n配置文件:\n";
    std::cout << "  使用默认配置文件: C:/Users/lzl67/Desktop/mrag/config.json\n";
    std::cout << "  如需自定义配置，请修改该文件\n";
}

// 离线建库模式
static int ingest_mode(const std::string& config_path, 
                      const std::string& novel_path, 
                      const std::string& db_path) {
    std::cout << "========================================\n";
    std::cout << "        MRAG 离线建库模式\n";
    std::cout << "========================================\n\n";
    
    std::cout << "小说文件: " << novel_path << std::endl;
    std::cout << "数据库文件: " << db_path << std::endl;
    
    try {
        // 1. 加载配置
        std::cout << "\n[1/4] 加载配置文件..." << std::endl;
        AppConfig config = AppConfig::load(config_path);
        std::cout << "✓ 配置加载完成" << std::endl;
        
        // 2. 初始化 MRAG 应用
        std::cout << "[2/4] 初始化 MRAG 应用..." << std::endl;
        std::unique_ptr<MragApp> app;
        try {
            app = std::make_unique<MragApp>(config);
            std::cout << "✓ MRAG 应用初始化成功" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "✗ 初始化失败: " << e.what() << std::endl;
            std::cerr << "请检查模型路径:\n";
            std::cerr << "  嵌入模型: " << config.emb_model_path << "\n";
            std::cerr << "  生成模型: " << config.gen_model_path << "\n";
            return 1;
        }
        
        // 3. 加载和处理文档
        std::cout << "[3/4] 加载和处理小说文件..." << std::endl;
        if (!app->loadDocument(novel_path)) {
            std::cerr << "✗ 文档处理失败" << std::endl;
            return 1;
        }
        std::cout << "✓ 文档处理完成，共 " << app->getChunkCount() << " 个文本块" << std::endl;
        
        // 4. 保存数据库
        std::cout << "[4/4] 保存向量数据库..." << std::endl;
        if (!app->saveChunks(db_path)) {
            std::cerr << "✗ 数据库保存失败" << std::endl;
            return 1;
        }
        std::cout << "✓ 数据库保存成功: " << db_path << std::endl;
        
        std::cout << "\n========================================\n";
        std::cout << "        离线建库完成！\n";
        std::cout << "========================================\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ 建库过程异常: " << e.what() << std::endl;
        return 1;
    }
}

// 在线问答模式
static int chat_mode(const std::string& config_path, 
                    const std::string& db_path) {
    std::cout << "========================================\n";
    std::cout << "        MRAG 在线问答模式\n";
    std::cout << "========================================\n\n";
    
    std::cout << "数据库文件: " << db_path << std::endl;
    
    try {
        // 1. 加载配置
        std::cout << "\n[1/3] 加载配置文件..." << std::endl;
        AppConfig config = AppConfig::load(config_path);
        std::cout << "✓ 配置加载完成" << std::endl;
        
        // 2. 初始化 MRAG 应用
        std::cout << "[2/3] 初始化 MRAG 应用..." << std::endl;
        std::unique_ptr<MragApp> app;
        try {
            app = std::make_unique<MragApp>(config);
            std::cout << "✓ MRAG 应用初始化成功" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "✗ 初始化失败: " << e.what() << std::endl;
            std::cerr << "请检查模型路径:\n";
            std::cerr << "  嵌入模型: " << config.emb_model_path << "\n";
            std::cerr << "  生成模型: " << config.gen_model_path << "\n";
            return 1;
        }
        
        // 3. 加载数据库
        std::cout << "[3/3] 加载向量数据库..." << std::endl;
        if (!app->loadChunks(db_path)) {
            std::cerr << "✗ 数据库加载失败" << std::endl;
            return 1;
        }
        std::cout << "✓ 数据库加载完成，共 " << app->getChunkCount() << " 个文本块" << std::endl;
        
        // 4. 进入交互式问答
        std::cout << "\n========================================\n";
        std::cout << "        进入交互式问答\n";
        std::cout << "========================================\n";
        std::cout << "输入 'quit' 或 'exit' 退出\n";
        std::cout << "========================================\n\n";
        
        while (true) {
            std::cout << "\n> ";
            std::string question;
            std::getline(std::cin, question);
            
            // 去除前后空格
            size_t start = question.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) {
                continue; // 空行
            }
            size_t end = question.find_last_not_of(" \t\r\n");
            question = question.substr(start, end - start + 1);
            
            // 检查退出命令
            if (question == "quit" || question == "exit") {
                std::cout << "退出问答系统\n";
                break;
            }
            
            // 正常查询
            std::cout << "\n思考中...\n";
            std::cout << "----------------------------------------\n";
            
            try {
                std::string answer = app->query(question);
                std::cout << answer << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "✗ 查询失败: " << e.what() << std::endl;
            }
            
            std::cout << "----------------------------------------\n";
        }
        
        std::cout << "\n========================================\n";
        std::cout << "        问答会话结束\n";
        std::cout << "========================================\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n✗ 问答过程异常: " << e.what() << std::endl;
        return 1;
    }
}

int main(int argc, char** argv) {
    // 设置本地化
    std::setlocale(LC_NUMERIC, "C");
    
    // 屏蔽 llama.cpp 底层日志
    llama_log_set(empty_log_callback, nullptr);
    
    // 固定配置文件路径
    std::string config_path = "C:/Users/lzl67/Desktop/mrag/config.json";
    
    // 检查参数数量
    if (argc < 2) {
        std::cerr << "错误: 需要指定运行模式\n";
        print_usage(argv[0]);
        return 1;
    }
    
    std::string mode = argv[1];
    
    if (mode == "--help" || mode == "-h") {
        print_usage(argv[0]);
        return 0;
    } else if (mode == "--ingest") {
        if (argc != 4) {
            std::cerr << "错误: --ingest 模式需要两个参数\n";
            std::cerr << "用法: " << argv[0] << " --ingest <小说TXT路径> <输出数据库路径>\n";
            return 1;
        }
        return ingest_mode(config_path, argv[2], argv[3]);
    } else if (mode == "--chat") {
        if (argc != 3) {
            std::cerr << "错误: --chat 模式需要一个参数\n";
            std::cerr << "用法: " << argv[0] << " --chat <数据库路径>\n";
            return 1;
        }
        return chat_mode(config_path, argv[2]);
    } else {
        std::cerr << "错误: 未知模式 '" << mode << "'\n";
        print_usage(argv[0]);
        return 1;
    }
}