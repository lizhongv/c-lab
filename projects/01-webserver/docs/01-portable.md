# 第 01 章 · 可构建：CMake 与跨平台 socket 抽象

> 本章代码：`stages/01-portable/`
> 对应提交：`feat(webserver): 第 01 章 跨平台构建与平台抽象`
> 预计用时：3～4 小时

## 1.1 本章目标

1. 让同一份 C 代码在 Linux、macOS、Windows 上都能编译、运行、通过测试。
2. 把 WinSock 与 POSIX socket 的差异收进一层薄封装（`platform.h`）。
3. 用 CMake 管构建，警告即错误，测试接入 CTest。
4. 修掉第 00 章里**属于「移植」范畴**的缺陷：越界写、`itoa`、`ERROR` 宏冲突、`system("pause")`、POST 指针比较。

本章**不修**行为层面的缺陷（路径穿越、无扩展名崩溃、单线程阻塞等），它们留给 02～05 章。每一章只解决一类问题，是这套课程的基本节奏。

## 1.2 为什么不能直接编译

把第 00 章的源码拿到 Linux 上，编译器会在第一行就拒绝：

| 依赖 | 出现在 | 平台 |
| --- | --- | --- |
| `#include <WinSock2.h>` | `webserver.h` | 仅 Windows |
| `SOCKET` / `INVALID_SOCKET` | 全部 socket 调用 | 仅 Windows |
| `WSAStartup` / `WSACleanup` | `init.c`/`main.c` | 仅 Windows |
| `closesocket` | `main.c` | 仅 Windows |
| `itoa` | `send.c` | MSVC 扩展 |
| `system("pause")` | 多处 | Windows 控制台 |
| `_CRT_SECURE_NO_WARNNINGS` | `webserver.h` | 拼写错误，实际无效 |

还有一类更隐蔽的差异：`recv`/`send` 在 POSIX 返回 `ssize_t`，在 Windows 返回 `int`；端口与地址要经过 `htons`/`inet_pton`；对端关闭后继续 `send` 在 POSIX 会触发 `SIGPIPE` 直接杀进程。

## 1.3 平台抽象层

`platform.h` 只暴露 12 个函数，两种平台各实现一份：

| 关注点 | Windows（`platform_win32.c`） | POSIX（`platform_posix.c`） |
| --- | --- | --- |
| 初始化 | `WSAStartup(2.2)` | 忽略 `SIGPIPE` |
| 套接字类型 | `SOCKET`（`UINT_PTR`） | `int`（文件描述符） |
| 无效值 | `INVALID_SOCKET` | `-1` |
| 关闭 | `closesocket` | `close` |
| 接收/发送 | `int` | `ssize_t`，`EINTR` 重试 |
| 地址转换 | `inet_pton` / `inet_ntop` | 同左（POSIX 2001） |
| 地址复用 | `SO_REUSEADDR` | `SO_REUSEADDR` |

对外统一返回 `clab_socket_t` 与 `long` 字节数，调用方不再关心平台。

> **一个真实的坑**：`wingdi.h` 把 `ERROR` 定义成 `0`，第 00 章的 `#define ERROR 0` 在包含 `windows.h` 后必然冲突。本章把状态码改名为 `CLAB_OK` / `CLAB_ERR`——宏名要带项目前缀，这是跨平台代码的基本纪律。

## 1.4 本章的改动清单

| # | 问题（第 00 章） | 本章改法 | 位置 |
| --- | --- | --- | --- |
| 1 | 只能在 Windows 编译 | 平台抽象 + CMake 条件源文件 | `platform*.c`、`CMakeLists.txt` |
| 2 | `itoa` 非标准 | `snprintf("%lld")` | `send.c` |
| 3 | `ERROR` 宏与 Windows 头冲突 | 改名 `CLAB_ERR` | 全部源文件 |
| 4 | `recv` 返回值当数组下标 | 先判 `got <= 0`，缓冲区留 1 字节 | `main.c` |
| 5 | `Judge_Method(...) == "POST"` 比较指针 | 返回枚举，`strcmp` 判断 | `handle.c` |
| 6 | 判方法有副作用、被调用两次 | 解析与响应分离，501 只在分发处发 | `handle.c`、`method.c` |
| 7 | `%s` 无宽度限制 | `%15s %1023s %15s` | `handle.c` |
| 8 | 端口写死 80，需要管理员 | `--port N`，支持 `0` 由系统分配 | `main.c` |
| 9 | `system("pause")`、裸 `exit(1)` | 打印错误并返回非零码 | `main.c` |
| 10 | 请求目标以 `/` 开头，`fopen("/x")` 找不到工作目录文件 | 去掉一个前导斜杠 | `handle.c` |
| 11 | 源码 GBK | 全仓库 UTF-8 | — |

> 第 10 条值得单独说明：第 00 章的 README 让用户访问 `127.0.0.1\gif\x.gif`，浏览器会把它规范成带前导斜杠的 `/gif/x.gif`，而原实现把 `/gif/x.gif` 直接交给 `fopen`——在 Windows 上等于当前盘符根目录。这是「文档能跑、代码跑不了」的典型案例。

## 1.5 构建与运行

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

只构建这一章：

```bash
cmake --build build --target webserver01
./build/projects/01-webserver/stages/01-portable/webserver01 --port 8080
```

`--port 0` 时程序会打印实际端口，测试脚本依赖这一行：

```
listening on 127.0.0.1:54321
```

## 1.6 自动化验收

`tests/test_stage01.py` 用原始 socket 发请求，覆盖：

1. `GET /hello.txt` → `200 OK`，正文逐字节一致。
2. `GET /missing.txt` → `404 Not Found`。
3. 一次发送**恰好 1024 字节**的请求 → 服务器不崩溃，之后仍能响应（第 00 章的越界写）。
4. `PUT` → `501 Not Implemented`。

运行：

```bash
python3 projects/01-webserver/stages/01-portable/tests/test_stage01.py \
    build/projects/01-webserver/stages/01-portable/webserver01
```

CTest 会自动找到 Python 并注册这条集成测试。

## 1.7 本章故意没修的问题

| 问题 | 现象 | 留到 |
| --- | --- | --- |
| 路径穿越 | `GET /../../README.md` 仍能读到上级文件 | 第 03 章 |
| 无扩展名崩溃 | `GET /noext` 仍会 `strcmp(NULL)` | 第 02 章 |
| 单线程阻塞 | 一个慢连接仍会拖住所有人 | 第 04 章 |
| 只 `recv` 一次 | 请求体与超长头部仍被截断 | 第 02 章 |
| 发送失败空转 | 断连后仍会读完剩余文件 | 第 05 章 |
| `Data:`、LF、`ctime` 日期 | 响应头仍不合规 | 第 02 章 |
| `Content-Length` 硬编码 | 404/501 的 42、41 仍写死 | 第 02 章 |

讲义末尾的练习会让你亲手验证：这些问题在 01 章依旧存在，只是换了编译方式。

## 1.8 练习

1. **平台差异**：把 `platform_socket_recv` 的返回值从 `long` 改成 `ssize_t`，在 Windows 上会发生什么？为什么统一成 `long` 更简单？
2. **EINTR**：POSIX 实现里 `recv` 对 `EINTR` 做了重试，`send` 也做了。举一个会产生 `EINTR` 的场景（提示：信号处理函数）。
3. **SO_REUSEADDR**：去掉 `SO_REUSEADDR`，快速重启服务器会看到什么错误？为什么测试里需要它？
4. **前导斜杠**：如果请求行是 `GET /../../etc/passwd HTTP/1.1`，本章的「去掉一个斜杠」会得到什么路径？为什么它仍然是穿越？
5. **回归**：用 `--port 0` 启动，手工发一个 `GET /noext`，确认第 00 章的崩溃仍在。这证明本章只做了「移植」，没有「修 bug」。

<details>
<summary>练习 1 参考</summary>

Windows 的 `recv` 返回 `int`，POSIX 返回 `ssize_t`（64 位下是 `long`）。若统一成 `ssize_t`，Windows 上没有这个类型，需要额外 `typedef`，且与 Winsock 的 `int` 语义不一致；统一成 `long` 在两种平台都能容纳返回值，转换只发生在封装层。

</details>

<details>
<summary>练习 4 参考</summary>

去掉了第一个 `/`，得到 `../../etc/passwd`，仍然是相对当前工作目录向上穿越。前导斜杠与路径穿越是两件事：前者是 HTTP 请求目标的格式，后者需要规范化 + 根目录前缀校验，第 03 章处理。

</details>

## 1.9 延伸阅读

- Beej's Guide to Network Programming：`getaddrinfo`、`inet_pton`、`EINTR` 与部分发送。
- POSIX `signal(7)`：`SIGPIPE` 的默认行为与 `MSG_NOSIGNAL`。
- Microsoft Learn：Winsock 的 `WSAStartup` 引用计数与 `closesocket` 语义。
- CMake 官方教程：`target_sources`、生成器表达式与 `add_test`。

---

下一章：02 · 协议正确（计划中）。进入前请确认：Linux 与 Windows 都能编译，`ctest` 全部通过，并能说出「为什么 `ERROR` 这个宏名会冲突」。
