# 基线：WebServer1.0（原样）

> 这是第 00 章的教材，**故意保留全部缺陷**。
> 原仓库已归档删除；源码原样保存在本目录。
> 转码说明：原项目为 GBK，本目录统一转 UTF-8，逻辑未改。

## 目录

```
00-baseline/
├── src/            8 个源文件（590 行）
├── test-file/      html / txt / xml / rtf 素材（图片未纳入，见下）
├── noext           空文件，用于复现「无扩展名崩溃」
└── README.md
```

原项目的 `test-file/gif/rain.gif`、`jpg/HotAirBalloon.jpg`、`png/paint.png` 共约 1.4 MB，未纳入本仓库。需要时自己生成：

```bash
python -c "open('test-file/png/paint.png','wb').write(b'\x89PNG\r\n\x1a\n' + b'0'*300000)"
```

## 编译

**MSVC**（在 `x64 Native Tools Command Prompt` 中，当前目录为本目录）：

```bat
cl /W3 /I src /Fe:webserver.exe src\main.c src\init.c src\handle.c src\method.c ^
   src\send.c src\file.c src\other.c ws2_32.lib
```

**MinGW-w64**：

```bat
gcc -std=c11 -I src src\main.c src\init.c src\handle.c src\method.c ^
    src\send.c src\file.c src\other.c -lws2_32 -o webserver.exe
```

`itoa` 为 MSVC 扩展；MinGW 下若链接失败，见第 00 章 0.3 的替换方法。

## 运行

- 端口默认 80（`src/webserver.h` 的 `SERVER_PORT`），需要管理员权限；建议改成 8080 再编译。
- **必须在当前目录运行 `webserver.exe`**，否则 `test-file/...` 找不到。
- 访问 `http://127.0.0.1:8080/test-file/html/hearts.html`。
- 控制台会打印请求内容；按 Ctrl+C 结束。

## 已知问题

7 个 P0 缺陷与 10 个协议问题的完整分析、复现命令和修复章节，见 [`../docs/00-baseline.md`](../docs/00-baseline.md)。

复现脚本：`../tests/repro_p0.py`。

## 说明

本目录**不进入**顶层 CMake 构建：它依赖 WinSock 与 MSVC 扩展，且我们刻意保留缺陷。第 01 章会给出跨平台版本。
