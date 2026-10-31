#!/usr/bin/env bash
# 一键构建并运行全部测试；Linux / macOS / WSL 使用。
# 用法：bash scripts/check.sh [Debug|Release]
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."
BUILD_TYPE="${1:-Debug}"
BUILD_DIR="build/${BUILD_TYPE}"

cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCLAB_ENABLE_SANITIZERS=ON
cmake --build "${BUILD_DIR}" --parallel
ctest --test-dir "${BUILD_DIR}" --output-on-failure
