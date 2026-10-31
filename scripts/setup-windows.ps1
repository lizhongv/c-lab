# Windows 环境检查与安装提示。
# 只检查、不自动安装：安装软件需要管理员权限，交给你决定。
# 用法：powershell -ExecutionPolicy Bypass -File scripts/setup-windows.ps1

$ErrorActionPreference = "Stop"

function Test-Tool($name, $hint) {
    $cmd = Get-Command $name -ErrorAction SilentlyContinue
    if ($cmd) {
        Write-Host "[OK]   $name -> $($cmd.Source)" -ForegroundColor Green
    } else {
        Write-Host "[缺失] $name" -ForegroundColor Yellow
        Write-Host "       $hint"
    }
}

Write-Host "检查 C 工具链..."
Test-Tool "cmake" "安装 CMake: https://cmake.org/download/ ，或 winget install Kitware.CMake"
Test-Tool "gcc"   "安装 MSYS2 后在 MinGW64 shell 中 pacman -S mingw-w64-x86_64-gcc；或使用 Visual Studio 的 cl"
Test-Tool "cl"    "安装 Visual Studio Build Tools 的「使用 C++ 的桌面开发」工作负载"
Test-Tool "ninja" "可选：winget install Ninja-build.Ninja"

Write-Host ""
Write-Host "推荐路线：" -ForegroundColor Cyan
Write-Host "  1. 只学 Linux 语义：安装 WSL2 + Ubuntu，在 WSL 中执行 sudo apt install build-essential cmake"
Write-Host "  2. Windows 原生：安装 Visual Studio Build Tools（MSVC）或 MSYS2（MinGW-w64）"
Write-Host "  3. 装好后运行：powershell -ExecutionPolicy Bypass -File scripts/check.ps1"
