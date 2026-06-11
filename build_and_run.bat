@echo off
echo ========================================
echo MRAG 项目构建和运行脚本
echo ========================================

REM 设置变量
set BUILD_DIR=build
set SOURCE_DIR=.
set TARGET=mrag.exe

echo.
echo 1. 清理并创建构建目录...
if exist %BUILD_DIR% (
    rmdir /s /q %BUILD_DIR%
)
mkdir %BUILD_DIR%

echo.
echo 2. 配置 CMake...
cd %BUILD_DIR%
cmake .. -G "Visual Studio,17,2022" -A x64
if %errorlevel% neq 0 (
    echo CMake 配置失败!
    cd ..
    exit /b 1
)

echo.
echo 3. 编译项目...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo 编译失败!
    cd ..
    exit /b 1
)

echo.
echo 4. 检查可执行文件...
if exist Release\%TARGET% (
    echo 编译成功: Release\%TARGET%
) else if exist %TARGET% (
    echo 编译成功: %TARGET%
) else (
    echo 错误: 未找到可执行文件!
    cd ..
    exit /b 1
)

cd ..
echo.
echo ========================================
echo 构建完成！可以使用以下命令运行：
echo   %BUILD_DIR%\Release\%TARGET% king3.txt
echo 或
echo   %BUILD_DIR%\Release\%TARGET% -c config.json king3.txt
echo ========================================

REM 如果提供了参数，则直接运行
if "%1" neq "" (
    echo.
    echo 5. 运行程序...
    echo 参数: %*
    %BUILD_DIR%\Release\%TARGET% %*
)