# 路线图：M0 工程骨架 + M1 WebServer 主线

> 更新：2026-11-01
> 范围：本文件给出 M0 与 M1 的详细目录、验收标准与交付节奏；第一章讲义见 `projects/01-webserver/docs/00-baseline.md`。

## M0 工程骨架（已完成）

| # | 交付物 | 验收 |
| --- | --- | --- |
| 1 | 顶层 CMake（C11、警告即错误、Sanitizer 开关） | `cmake -S . -B build && cmake --build build` 通过 |
| 2 | `common/` 跨项目基础库（日志）与单元测试 | `ctest` 通过 |
| 3 | 目录规范：`projects/NN-name/{docs,stages,tests}` | 与 CONTRIBUTING 模板一致 |
| 4 | GitHub Actions：Linux / Windows / macOS | 三平台构建与测试通过 |
| 5 | 一键脚本：`scripts/check.sh`、`scripts/setup-windows.ps1` | 新机器按 README 能跑通 |
| 6 | 许可证、第三方素材、贡献指南、行为准则 | 文件齐全，链接可达 |

## M1 WebServer 主线（进行中）

### 章节总表

| 章 | 讲义 | 代码阶段 | 核心改动 | 自动化验收 |
| --- | --- | --- | --- | --- |
| 00 | `docs/00-baseline.md` | `stages/00-baseline/` | 原始代码 + 缺陷复现脚本 | `tests/repro_p0.py` 逐项复现 |
| 01 | `docs/01-portable.md` | `stages/01-portable/` | CMake + 平台抽象（POSIX / Winsock）+ UTF-8 | ✅ Linux/Windows 编译；`ctest` 通过 |
| 02 | `docs/02-http.md` | `stages/02-http/` | 请求行与头部解析、响应构造、MIME、错误码 | 报文级集成测试（合法/非法/边界） |
| 03 | `docs/03-security.md` | `stages/03-security/` | 根目录沙箱、路径规范化、URL 解码、请求上限 | 穿越用例全拒；解码用例全过 |
| 04 | `docs/04-concurrency.md` | `stages/04-concurrency/` | 线程池或 epoll/kqueue、连接上限、优雅退出 | 并发压测；慢连接不阻塞 |
| 05 | `docs/05-robustness.md` | `stages/05-robustness/` | 部分发送、断连、超时、日志、配置 | 断连/半包/大文件不崩 |
| 06 | `docs/06-quality.md` | `stages/06-final/` | 单元 + 集成 + fuzz + CI | `make check` 全绿，覆盖率达标 |

### 每章交付物（统一模板）

1. `docs/NN-<stage>.md`：目标、上一版问题、改动要点、代码走读、验收命令、练习、延伸阅读。
2. `stages/NN-<stage>/`：可独立构建运行的完整代码，不是 diff 片段。
3. `tests/`：该章新增行为的回归用例；能自动跑的进 CTest。
4. `CHANGELOG.md` 对应条目。

### 验收标准（M1 完成）

- 三平台 CI 全绿；`-Wall -Wextra -Wpedantic -Werror` 无警告。
- 第 00 章的 7 个 P0 缺陷全部有复现用例，并在后续章节全部修复、转为回归测试。
- 章节 02 的报文测试覆盖：请求行畸形、头部超长、无扩展名、未知方法、HEAD、404、501/405、Content-Length 边界。
- 章节 03 的穿越用例覆盖：`../`、绝对路径、URL 编码、Windows 盘符、符号链接。
- 章节 04 的压测：100 并发 + 慢连接，P95 响应时间与错误率达标。
- 章节 06 打开 ASan/UBSan 跑完全部测试与 60 秒 fuzz 无崩溃。
- 学习者按讲义从 00 走到 06 能独立复现每一步。

## 节奏

| 阶段 | 内容 | 估算 |
| --- | --- | --- |
| M0 | 骨架、CI、脚本、模板 | 2～3 天 |
| M1.1 | 第 00、01 章 | 2～3 天 |
| M1.2 | 第 02、03 章 | 3～4 天 |
| M1.3 | 第 04、05 章 | 3～4 天 |
| M1.4 | 第 06 章、演练与发布 | 2～3 天 |

## 风险与取舍

- **基线只在 Windows 可编译**：第 00 章不进入 CMake 构建，避免污染 CI；第 01 章起三平台可构建。
- **并发选型**：第 04 章先给线程池（易读、跨平台），事件循环作为进阶阅读与可选分支，避免一章塞两套模型。
- **fuzz 只对解析入口开放**：避免把整个服务器塞进 fuzz 目标导致运行时间不可控。
