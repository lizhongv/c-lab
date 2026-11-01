# 更新日志

格式参照 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)。

## [未发布]

### 新增

- 仓库骨架：CMake（C11、警告即错误、Sanitizer 开关）、`common/` 日志库与测试、三平台 CI。
- `projects/01-webserver` 第 00 章：基线代码、7 个 P0 缺陷的讲义与复现脚本。
- `projects/01-webserver` 第 01 章：平台抽象层（POSIX / WinSock）、CMake 接入、可配置端口、第 01 章讲义与集成测试。
- 路线图：M0 工程骨架、M1 WebServer 七个阶段的验收标准。
- 文档：README 增加徽章、页内导航与学习路线图（`docs/images/roadmap.png`）。

### 计划

- 第 01 章：CMake + 跨平台 socket 抽象。
- 第 02～06 章：HTTP 协议、安全、并发、健壮性、测试与 fuzz。
- 横向项目：hash table、mini shell、JSON parser、thread pool。

## [0.1.0] - 2026-11-01

首个骨架版本。
