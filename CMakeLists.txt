cmake_minimum_required(VERSION 3.14)    # 第1行：必须最先
project(simple-chat) 
set(TARGET mrag)

set(CUDA_PATH "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.8")
set(CUDA_LIB_PATH "${CUDA_PATH}/lib/x64")
set(CUDA_INCLUDE_PATH "${CUDA_PATH}/include")

add_executable(${TARGET} src/main.cc)


set(LLAMA_LIBRARY "C:/Users/lzl67/Desktop/llama.cpp-master1/examples/simple-chat/llama.lib")



if(EXISTS ${LLAMA_LIBRARY})
    message(STATUS "Found llama.lib at: ${LLAMA_LIBRARY}")
    target_link_libraries(${TARGET} PRIVATE ${LLAMA_LIBRARY})
    target_link_libraries(${TARGET} PRIVATE "C:/Users/lzl67/Desktop/mrag/lib/ggml.lib")
    target_link_libraries(${TARGET} PRIVATE "C:/Users/lzl67/Desktop/mrag/lib/ggml-base.lib")
    target_link_libraries(${TARGET} PRIVATE "C:/Users/lzl67/Desktop/mrag/lib/ggml-cpu.lib")
    target_link_libraries(${TARGET} PRIVATE "C:/Users/lzl67/Desktop/mrag/lib/ggml-cuda.lib")

    target_link_libraries(${TARGET} PRIVATE
    ${CUDA_LIB_PATH}/cudart.lib      # CUDA 运行时库（必需）
    ${CUDA_LIB_PATH}/cuda.lib        # CUDA 驱动 API
    ${CUDA_LIB_PATH}/cublas.lib      # CUDA 线性代数库（必需）
    ws2_32
    winmm
    )
    target_include_directories(${TARGET} PRIVATE
    "${CUDA_INCLUDE_PATH}"
    )

else()
    message(FATAL_ERROR "llama.lib not found!")
endif()

target_compile_features(${TARGET} PRIVATE cxx_std_17)