#include "mragapp.h"
#include "config.h"
#include <iostream>
#include <string>
#include <memory>
#include <cstdlib>
#include <clocale>

// 显示使用说明
static void print_usage(const std::string& program_name) {
    std::cout << "\nMRAG (Multi-modal Retrieval Augmented Generation) 系统\n";
    std::cout << "用法:\n";
    std::cout << "  " << program_name << " [-h] [-c config.json] [document.txt]\n";
    std::cout << "\n选项:\n";
    std::cout << "  -h, --help            显示此帮助信息\n";
    std::cout << "  -c, --config FILE     指定配置文件路径 (默认: ../config.json)\n";
    std::cout << "  document.txt          要处理的文档文件路径\n";
    std::cout << "\n示例:\n";
    std::cout << "  " << program_name << " -c myconfig.json king3.txt\n";
    std::cout << "  " << program_name << " king3.txt\n";
    std::cout << "\n交互模式:\n";
    std::cout << "  加载文档后，输入查询问题，系统将基于文档内容生成回答\n";
    std::cout << "  输入 'quit' 或 'exit' 退出程序\n";
}

int main(int argc, char** argv) {
    // 设置本地化
    std::setlocale(LC_NUMERIC, "C");
    
    std::string config_path = "C:/Users/lzl67/Desktop/mrag/config.json";
    std::string document_path;
    
    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_path = argv[++i];
            } else {
                std::cerr << "错误: -c 选项需要配置文件路径\n";
                return 1;
            }
        } else if (arg[0] == '-') {
            std::cerr << "错误: 未知选项 " << arg << "\n";
            print_usage(argv[0]);
            return 1;
        } else {
            // 第一个非选项参数视为文档路径
            if (document_path.empty()) {
                document_path = arg;
            } else {
                std::cerr << "警告: 忽略额外参数 " << arg << "\n";
            }
        }
    }
    
    if (document_path.empty()) {
        std::cerr << "错误: 请指定要处理的文档文件\n";
        print_usage(argv[0]);
        return 1;
    }
    
    std::cout << "========================================\n";
    std::cout << "        MRAG 系统启动\n";
    std::cout << "========================================\n\n";
    
    try {
        // 1. 加载配置
        std::cout << "[1/4] 加载配置文件..." << std::endl;
        AppConfig config = AppConfig::load(config_path);
        std::cout << "配置加载完成\n";
        
        // 2. 初始化 MRAG 应用
        std::cout << "[2/4] 初始化 MRAG 应用..." << std::endl;
        std::unique_ptr<MragApp> app;
        try {
            app = std::make_unique<MragApp>(config);
            std::cout << "MRAG 应用初始化成功\n";
        } catch (const std::exception& e) {
            std::cerr << "初始化失败: " << e.what() << std::endl;
            std::cerr << "请检查模型路径和配置:\n";
            std::cerr << "  嵌入模型: " << config.emb_model_path << "\n";
            std::cerr << "  生成模型: " << config.gen_model_path << "\n";
            return 1;
        }
        
        // 3. 加载和处理文档
        std::cout << "[3/4] 加载和处理文档: " << document_path << std::endl;
        if (!app->loadDocument(document_path)) {
            std::cerr << "文档处理失败，请检查文件路径和格式\n";
            return 1;
        }
        std::cout << "文档处理完成，共 " << app->getChunkCount() << " 个文本块\n";
        
        // 4. 进入交互式查询模式
        std::cout << "[4/4] 进入交互式查询模式\n";
        std::cout << "========================================\n";
        std::cout << "输入 'quit' 或 'exit' 退出\n";
        std::cout << "输入 'save' 保存块数据\n";
        std::cout << "输入 'load' 加载块数据\n";
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
                std::cout << "退出 MRAG 系统\n";
                break;
            }
            
            // 检查保存命令
            if (question == "save") {
                std::cout << "请输入保存文件名: ";
                std::string save_path;
                std::getline(std::cin, save_path);
                if (app->saveChunks(save_path)) {
                    std::cout << "保存成功\n";
                } else {
                    std::cout << "保存失败\n";
                }
                continue;
            }
            
            // 检查加载命令
            if (question == "load") {
                std::cout << "请输入加载文件名: ";
                std::string load_path;
                std::getline(std::cin, load_path);
                if (app->loadChunks(load_path)) {
                    std::cout << "加载成功，共 " << app->getChunkCount() << " 个文本块\n";
                } else {
                    std::cout << "加载失败\n";
                }
                continue;
            }
            
            // 正常查询
            std::cout << "\n思考中...\n";
            std::cout << "----------------------------------------\n";
            
            try {
                std::string answer = app->query(question);
                std::cout << answer << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "查询失败: " << e.what() << std::endl;
            }
            
            std::cout << "----------------------------------------\n";
        }
        
        std::cout << "\n========================================\n";
        std::cout << "        MRAG 系统结束\n";
        std::cout << "========================================\n";
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\n程序运行异常: " << e.what() << std::endl;
        return 1;
    }
}