# MRAG 系统 - Multi-modal Retrieval Augmented Generation

一个基于检索增强生成（RAG）的小说问答系统，使用本地大语言模型进行智能文档问答。

## 📋 功能特性

- **📚 文档处理**：支持大型文本文件（如小说）的智能分块处理
- **🔍 向量检索**：基于余弦相似度的语义检索，找到最相关的文本片段
- **🤖 智能问答**：使用本地大语言模型，基于检索内容生成准确回答
- **⚡ 离线运行**：完全本地化运行，无需网络连接
- **🎯 两阶段流程**：清晰的离线建库 + 在线问答分离架构

## 🚀 快速开始

### 环境要求

- **操作系统**：Windows 
- **编译器**：支持 C++17 的编译器
- **构建工具**：CMake 3.14+
- **GPU支持**：可选，支持 CUDA 加速（需要 NVIDIA GPU）

### 编译步骤

#### Windows（使用 Visual Studio）
```bash
# 1. 克隆或进入项目目录
cd C:\Users\lzl67\Desktop\mrag

# 2. 创建构建目录并编译
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release


```


### 模型下载

系统需要两个预训练模型：

1. **嵌入模型**：用于文本向量化
   - 推荐：`bge-small-zh-v1.5-f16.gguf`
   - 下载地址：[HuggingFace](https://huggingface.co/Xenova/bge-small-zh-v1.5-gguf)

2. **生成模型**：用于回答生成
   - 推荐：`qwen2.5-7b-instruct-q4_k_m.gguf`
   - 下载地址：[HuggingFace](https://huggingface.co/Qwen/Qwen2.5-7B-Instruct-GGUF)

**下载后放置位置**：
```
C:\Users\lzl67\Desktop\mrag\models\
├── bge-small-zh-v1.5-f16.gguf      # 嵌入模型
└── qwen2.5-7b-instruct-q4_k_m.gguf # 生成模型
```

### 配置文件

系统使用 `config.json` 管理配置：
```json
{
    "models": {
        "embedding": "模型路径",
        "generation": "模型路径",
        "n_gpu_layers": 99
    },
    "document": { "chunk_size": 500, "overlap_size": 50 },
    "retrieval": { "top_k": 3 },
    "generation": {
        "n_ctx": 4096, "n_batch": 4096,
        "max_output_tokens": 512,
        "temperature": 0.7
    },
    "embedding": { "n_ctx": 512, "n_batch": 512 }
}
```

## 📖 使用指南

### 1. 离线建库模式（只需执行一次）

处理小说文件，生成向量数据库：

```bash
# 基本用法
mrag.exe --ingest <小说TXT路径> <输出数据库路径>

# 示例：处理《三国演义》小说
mrag.exe --ingest king3.txt king3.db

# 处理进度显示
========================================
       MRAG 离线建库模式
========================================

小说文件: king3.txt
数据库文件: king3.db

[1/4] 加载配置文件...
✓ 配置加载完成

[2/4] 初始化 MRAG 应用...
✓ MRAG 应用初始化成功

[3/4] 加载和处理小说文件...
✓ 文档处理完成，共 1524 个文本块

[4/4] 保存向量数据库...
✓ 数据库保存成功: king3.db

========================================
       离线建库完成！
========================================
```

### 2. 在线问答模式

加载数据库，进入交互式问答：

```bash
# 基本用法
mrag.exe --chat <数据库路径>

# 示例：使用《三国演义》数据库
mrag.exe --chat king3.db

# 交互界面
========================================
       MRAG 在线问答模式
========================================

数据库文件: king3.db

[1/3] 加载配置文件...
✓ 配置加载完成

[2/3] 初始化 MRAG 应用...
✓ MRAG 应用初始化成功

[3/3] 加载向量数据库...
✓ 数据库加载完成，共 1524 个文本块

========================================
       进入交互式问答
========================================
输入 'quit' 或 'exit' 退出
========================================

> 诸葛亮的主要事迹有哪些？

思考中...
----------------------------------------
根据小说内容，诸葛亮的主要事迹包括：
1. 隆中对，为刘备制定三分天下战略
2. 火烧博望坡、新野
3. 赤壁之战借东风
4. 七擒孟获，平定南中
5. 六出祁山，北伐中原
6. 发明木牛流马、诸葛连弩
...
----------------------------------------

> exit
退出问答系统
```

### 3. 帮助信息

```bash
mrag.exe --help

MRAG (Multi-modal Retrieval Augmented Generation) 系统
用法:
  mrag.exe --ingest <小说TXT路径> <输出数据库路径>
  mrag.exe --chat <数据库路径>
  mrag.exe --help

模式说明:
  --ingest  离线建库模式：处理小说文件，生成向量数据库
  --chat    在线问答模式：加载数据库，进入交互式问答
```

## 🛠️ 命令行选项

| 选项 | 参数 | 说明 |
|------|------|------|
| `--ingest` | `<input> <output>` | 离线建库：处理文本文件生成数据库 |
| `--chat` | `<database>` | 在线问答：加载数据库进行交互 |
| `--help` | - | 显示使用说明 |

## 📊 性能建议

### 硬件要求
- **最低配置**：8GB RAM，4核 CPU
- **推荐配置**：16GB+ RAM，NVIDIA GPU（支持CUDA）
- **大型文档**：处理超过10MB的文档建议16GB+内存

### 模型选择
- **轻量级**：`qwen2.5-1.5b-instruct` + `bge-small-zh`
- **平衡型**：`qwen2.5-7b-instruct` + `bge-small-zh`
- **高质量**：`qwen2.5-14b-instruct` + `bge-large-zh`

### 配置调优
```json
{
    "document": {
        "chunk_size": 500,      // 文本块大小（字符数）
        "overlap_size": 50      // 块间重叠（避免信息割裂）
    },
    "retrieval": {
        "top_k": 3              // 检索相关块数量
    },
    "generation": {
        "temperature": 0.7,     // 创造性（0.1-1.0）
        "max_output_tokens": 512 // 回答长度限制
    }
}
```

## 🔧 故障排除

### 常见问题

**Q: 编译失败，找不到 llama.h**
```bash
# 确保 include 目录包含 llama 头文件
# 正确结构：
mrag/include/llama/llama.h
```

**Q: 模型加载失败**
```bash
# 检查 config.json 中的模型路径
# 确保模型文件存在且格式正确
```

**Q: 内存不足**
```bash
# 减少 chunk_size（如从 500 改为 доктри
# 使用量化模型（q4_k_m, q5_k_m）
# 增加系统虚拟内存
```

**Q: 回答质量不高**
```bash
# 增加 top_k（如从 3 改为 5）
# 调整 temperature（如从 0.7 改为 0.3）
# 检查文档分块是否合理
```

## 📁 项目结构

```
mrag/
├── CMakeLists.txt          # 构建配置
├── config.json             # 系统配置文件
├── README.md              # 本文档
├── DESIGN.md              # 设计文档
├── build_and_run.bat      # Windows构建脚本
├── test_commands.bat      # 测试脚本
├── include/               # 第三方库头文件
│   └── llama/            # llama.cpp 头文件
├── lib/                   # 预编译库文件
│   ├── llama.lib         # llama 库
│   └── ggml*.lib         # ggml 相关库
├── models/               # 模型文件
│   ├── bge-small-zh-v1.5-f16.gguf
│   └── qwen2.5-7b-instruct-q4_k_m.gguf
├── src/                  # 源代码
│   ├── main.cc           # 主程序入口
│   ├── mragapp.h/cc      # 应用主类
│   ├── config.h/cc       # 配置管理
│   ├── chunk.h/cc        # 数据结构
│   ├── document.h/cc     # 文档处理
│   ├── llamamodel.h/cc   # 模型基类
│   ├── embeddingengine.h/cc # 嵌入引擎
│   ├── generationengine.h/cc # 生成引擎
│   └── vectordatabase.h/cc  # 向量检索
└── test/                 # 测试文件（示例）
    ├── king3.txt        # 示例小说
    └── king3.db         # 示例数据库
```




## 📞 支持

如有问题，请查看：
1. 本 README 文档
2. DESIGN.md 设计文档
3. 代码注释和日志输出