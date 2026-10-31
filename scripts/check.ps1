# 一键构建并运行全部测试（Windows PowerShell）。
# 用法：powershell -ExecutionPolicy Bypass -File scripts/check.ps1
param(
    [ValidateSet("Debug", "Release")]
    [string]$BuildType = "Debug"
)

$ErrorActionPreference = "Stop"
Set-Location (Join-Path $PSScriptRoot "..")
$buildDir = Join-Path "build" $BuildType

cmake -S . -B $buildDir -DCMAKE_BUILD_TYPE=$BuildType
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build $buildDir --config $BuildType --parallel
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

ctest --test-dir $buildDir --build-config $BuildType --output-on-failure
exit $LASTEXITCODE
