# 第 00 章 · 基线：先看见问题

> 本章代码：`stages/00-baseline/`（原项目，UTF-8 转码，逻辑未改）
> 复现脚本：`tests/repro_p0.py`
> 预计用时：2～3 小时（含复现）

## 0.1 本章目标

读完并动手之后，你应该能做到：

1. 说清一个 HTTP 请求从 `accept` 到 `send` 在本项目中经过哪些函数。
2. 用一条命令复现至少 5 个缺陷，并指出对应的代码行。
3. 区分「崩溃」「死循环」「安全漏洞」「协议错误」四类问题，并给出修复优先级。
4. 理解为什么「能打开网页」不等于「服务器是对的」。

本章**不修代码**。先学会看见问题，再谈怎么修。

## 0.2 原项目速览

| 文件 | 行数 | 职责 |
| --- | --- | --- |
| `src/main.c` | 82 | 启动、`listen`/`accept` 循环、收请求、关连接 |
| `src/init.c` | 49 | `WSAStartup`、建 socket、`bind` 到 `127.0.0.1:80` |
| `src/handle.c` | 39 | 用 `sscanf` 取请求行，判方法，判 URI |
| `src/method.c` | 77 | 方法判断与 501 响应 |
| `src/send.c` | 108 | 200 响应与文件分块发送 |
| `src/file.c` | 115 | 404 响应、扩展名到 `Content-Type` 的映射、文件存在性检查 |
| `src/other.c` | 59 | `send` 封装、`ctime` 取时间、URI 检查、POST 值打印 |
| `src/webserver.h` | 38 | 常量（IP、端口、缓冲区）与函数声明 |

请求路径：

```
accept → recv 一次 → sscanf 请求行 → Judge_Method → Judge_URI(stat)
      → Send_File（定 Content-Type、发响应头、分块发文件）→ closesocket
```

整个服务器是**单线程阻塞**的：同一时刻只服务一个连接，收完一个请求就关连接。

## 0.3 编译与运行

基线只支持 Windows（WinSock、`itoa`、`system("pause")`）。

**方式一：MSVC（推荐，与原项目一致）**

在「x64 Native Tools Command Prompt for VS」中，进入 `stages/00-baseline`：

```bat
cl /W3 /I src /Fe:webserver.exe src\main.c src\init.c src\handle.c src\method.c ^
   src\send.c src\file.c src\other.c ws2_32.lib
```

**方式二：MinGW-w64**

```bat
gcc -std=c11 -I src src\main.c src\init.c src\handle.c src\method.c ^
    src\send.c src\file.c src\other.c -lws2_32 -o webserver.exe
```

> `itoa` 是 MSVC 扩展，MinGW 下若报错，把 `send.c` 中的 `itoa` 换成 `snprintf(Length, sizeof Length, "%ld", (long)file_stat.st_size)` 再编译。这本身就是第 01 章要解决的问题之一。

**运行**

- 端口写死为 80（`webserver.h:8`），Windows 上需要**管理员权限**。不想用 80，先改成 8080 再编译。
- 必须在 `stages/00-baseline` 目录下启动，否则 `test-file/...` 的路径对不上。
- 浏览器访问 `http://127.0.0.1:8080/test-file/html/hearts.html`。

原 README 写的 `127.0.0.1\gif\xxx.gif` 与仓库实际目录不符：素材在 `test-file/` 下，且浏览器会把反斜杠规范成斜杠。这是第 00 章的第一个「文档也是缺陷」的例子。

## 0.4 事故档案（7 个 P0）

复现脚本统一入口：

```bash
python tests/repro_p0.py --host 127.0.0.1 --port 8080 --case <name>
```

可用 `--case`：`traversal`、`recv-oob`、`disconnect`、`noext`、`post`、`slowloris`。

> 建议顺序：先跑不致命的 `traversal`、`post`、`recv-oob`，再跑 `slowloris`，最后跑会崩服务的 `noext` 与会卡死的 `disconnect`。服务崩了重新启动即可。

### P0-1 路径穿越（安全）

**现象**：请求里的 `..` 被原样拼进 `fopen`，可以读到工作目录之外的文件。

**复现**（工作目录为 `stages/00-baseline`）：

```bash
curl --path-as-is "http://127.0.0.1:8080/../../README.md"
```

`../../README.md` 从 `stages/00-baseline` 往上两级，正好是 `projects/01-webserver/README.md`——一个本不该被服务出去的文件。

**根因**：`file.c:26` 直接 `fopen(URI, "rb")`；`other.c:49-59` 只检查文件是否存在，没有把路径限制在根目录内，也没有规范化。`curl` 默认会先把 `..` 归一化掉，所以必须加 `--path-as-is`；攻击者当然不会帮你归一化。

**后果**：读取进程有权限读到的任意文件。以管理员运行（80 端口的要求）时，影响放大。

**修复方向**：第 03 章。根目录约束 + 规范化后前缀校验 + URL 解码后再次校验。

### P0-2 recv 返回值未检查，越界写（崩溃 / 未定义行为）

**现象**：`recv` 返回 `-1` 或缓冲区满时，下标越界。

**复现**：

```bash
python tests/repro_p0.py --case recv-oob
```

脚本一次写入 1024 字节的请求行，`recv` 返回 1024，随后 `revbuf[1024] = 0` 写到数组之外。可能不立即崩溃——这正是未定义行为可怕的地方。

**根因**：`main.c:64-65`

```c
rval = recv(MessageSock, revbuf, BUF_SIZE, 0);
revbuf[rval] = 0x00;
```

`rval == -1`（连接异常）时写 `revbuf[-1]`；`rval == 1024` 时写 `revbuf[1024]`。正确做法是先判 `rval <= 0`，并用 `rval < BUF_SIZE` 保证留出终止符位置。

**修复方向**：第 01 章（安全读写封装）与第 06 章（ASan 回归）。

### P0-3 发送失败后不退出，单线程被长时间占用（拒绝服务）

**现象**：客户端下载中途断开后，服务器不会立刻结束这个请求，而是继续读完剩余文件，每次发送失败都打印一行错误。文件越大，占用越久；若某次 `send()` 返回 0，才会真正死循环。

**复现**：

```bash
python -c "open('big.bin','wb').write(b'A'*(64*1024*1024))"
python tests/repro_p0.py --case disconnect      # 下到一半 RST 断开
curl -m 3 "http://127.0.0.1:8080/test-file/txt/Genesis.txt"
```

观察：控制台出现大量 `Sending file information error!`；后续请求会排队，文件越大越明显。8 MB 文件下这个过程很快，不足以让人察觉，所以复现脚本用 64 MB。

**根因**：`send.c:84-86` 与 `92-96`，发送失败时 `continue`，但 `file_stat.st_size` 没有递减：

```c
if (Send_Ifon(Socket, sendbuf, send_length) == ERROR) {
    printf("Sending file information error!\n");
    continue;          // st_size 没变，下一轮继续读文件、继续失败
}
```

循环之所以最终能结束，是因为文件读完后 `fread` 返回 0，`Send_Ifon(..., 0)` 返回 OK，随后才递减 `st_size`。也就是说：**失败不退出，而是空转到文件末尾**。真正无限循环发生在 `other.c:10-19`：

```c
rval = send(Socket, sendbuf + sendtotal, bufleft, 0);
if (rval < 0) break;   // rval == 0 时 sendtotal/bufleft 都不变，循环不前进
```

**后果**：大文件断连时单线程被空转占用，其他请求排队；日志刷屏；`send()` 返回 0 时进程卡死。

**修复方向**：第 05 章（错误退出与资源处理）+ 第 04 章（每连接独立，坏连接不影响其他连接）。

### P0-4 无扩展名文件触发空指针（崩溃）

**现象**：请求一个存在但没有扩展名的文件，服务器崩溃。

**复现**：仓库里已放了空文件 `stages/00-baseline/noext`。

```bash
curl "http://127.0.0.1:8080/noext"
```

**根因**：`file.c:64-72`

```c
if ((suffix = strrchr(URI, '.')) != NULL)
    suffix = suffix + 1;

if (strcmp(suffix, "html") == 0)   // suffix 为 NULL 时崩溃
```

**后果**：远程崩溃；单线程下服务器直接退出，需人工重启。

**修复方向**：第 02 章（MIME 解析与错误响应）。

### P0-5 POST 判定用指针比较（逻辑错误 / 未定义行为）

**现象**：POST 请求有时被当成 GET 处理，请求体从未被解析。

**复现**：

```bash
curl -X POST -d "a=1" "http://127.0.0.1:8080/test-file/txt/Genesis.txt"
```

观察服务器控制台是否打印 `Post Value: a=1`。这段行为取决于编译器是否合并字符串常量。

**根因**：`handle.c:21`

```c
else if (Judge_Method(Method, Socket) == "POST")
```

比较的是两个字符串字面量的**地址**，不是内容；`Judge_Method` 还被调用了两次，而它在方法不支持时会**发送 501 响应**（`method.c:72`），判方法变成了有副作用的动作。

**修复方向**：第 02 章（解析与判定分离，返回枚举而非字符串）。

### P0-6 单线程阻塞：一个慢连接拖垮所有人（拒绝服务）

**现象**：一个连上但不发完整请求的连接，会让后续请求全部排队。

**复现**：

```bash
python tests/repro_p0.py --case slowloris   # 占住连接 15 秒
curl -m 3 "http://127.0.0.1:8080/test-file/txt/Genesis.txt"   # 超时
```

**根因**：`main.c:39-76` 在 `accept` 后同步 `recv`，没有超时；单线程模型下，`recv` 阻塞期间不会回到 `accept`。

**修复方向**：第 04 章（连接超时 + 线程池或事件循环）。

### P0-7 请求只读一次、无长度上限（协议 / 健壮性）

**现象**：超过 1024 字节的请求被截断；请求体不读；带 Cookie 的浏览器请求可能丢字段。

**根因**：`main.c:64` 只调用一次 `recv`，`BUF_SIZE` 为 1024；没有按 `\r\n\r\n` 找头部结束，也不读 `Content-Length` 指定的正文。

**修复方向**：第 02 章（缓冲区与解析器）。

## 0.5 还有一批协议问题（P1）

| # | 位置 | 问题 | 标准做法 |
| --- | --- | --- | --- |
| 1 | `send.c:45`、`method.c:30`、`file.c:29` | 响应头写 `Data:` | `Date:` |
| 2 | `other.c:27-35` | 用 `ctime()` 当日期，带换行、本地时区 | IMF-fixdate，GMT，如 `Sun, 01 Nov 2026 06:01:00 GMT` |
| 3 | `send.c:63` | `Content-Length` 后发 `\n`，与其他头的 `\r\n` 混用 | 所有头部行以 `\r\n` 结尾，头部与正文之间空行 `\r\n` |
| 4 | 全部响应 | 没有 `Connection: close` | HTTP/1.1 发完即关时应显式声明 |
| 5 | `file.c:78` | `image/jpg` | `image/jpeg` |
| 6 | `method.c:59-75` | 只支持 GET/POST，HEAD 返回 501 | HEAD 与 GET 同头无体；已知但禁止的方法用 405 + `Allow` |
| 7 | `handle.c` | 不解析 URL 编码与查询串，`/` 不映射 `index.html` | 解码后再做路径校验；`/` → 配置的默认文件 |
| 8 | `send.c:30` | `itoa(file_stat.st_size, ...)` 用 `int` 接收 64 位大小 | `snprintf` + `long long` |
| 9 | `file.c:10`、`method.c:11` | `Content-Length` 硬编码 42 / 41 | 由正文长度计算 |
| 10 | `file.c:26` 后 | 文件不存在时先返回 404，但类型未知时直接断开、无响应 | 统一错误响应（400/404/405/415/500） |

## 0.6 自测

不看讲义，回答：

1. 一个请求从 `accept` 到文件发完，经过哪几个函数？
2. 客户端发来 1024 字节请求时，哪一行写了数组之外的内存？
3. 为什么下载断开会卡住**整个**服务器，而不是只影响这一个连接？
4. `Judge_Method(...) == "POST"` 为什么有时「看起来能用」？为什么仍然必须改？
5. 七个 P0 里，哪一个属于安全问题而不是稳定性问题？
6. 原项目 README 里的访问路径为什么打不开文件？

## 0.7 练习

1. **穿越层数**：用 `curl --path-as-is` 读到 `projects/01-webserver/README.md`，再试着读仓库根目录的 `README.md`，需要几层 `../`？
2. **三秒打挂**：写一个不超过 20 行的 Python 脚本，让服务器在 3 秒内不再响应新请求。分别用「慢连接」和「断连下载」各实现一次。
3. **找第八个问题**：讲义之外，至少再找 2 个问题（提示：`listen` 在循环里、`accept` 的长度参数、`Send_Ifon` 的返回值语义、`system("pause")`、源码编码）。
4. **分类**：给 7 个 P0 标注「崩溃 / 死循环 / 安全 / 协议」，并按你理解的修复成本排序，说明理由。

<details>
<summary>练习 3 参考</summary>

- `main.c:42` 把 `listen` 放在 `accept` 循环里，重复调用；监听应只做一次。
- `main.c:52` 用 `sizeof(struct sockaddr)` 作为 `accept` 的长度参数，应传 `sizeof(ClientAddr)`。
- `other.c:21` `Length = sendtotal;` 赋值后从未使用，接口语义混乱。
- `main.c:46`、`init.c:28/43` 用 `system("pause")` 与 `exit(1)` 处理错误，服务器进程不应被交互式输入卡住。
- 源码为 GBK，GitHub 上中文注释乱码；`webserver.h:5` 的 `_CRT_SECURE_NO_WARNNINGS` 拼写错误，实际未生效。
- 没有 `Content-Type` 的 `charset`，UTF-8 的 HTML 在中文 Windows 浏览器上可能按 GBK 解码。

</details>

## 0.8 延伸阅读

- RFC 9112（HTTP/1.1 报文语法）、RFC 9110（语义）：`Date`、`Connection`、状态码的权威定义。
- Beej's Guide to Network Programming：socket 生命周期与阻塞语义。
- 《深入理解计算机系统》第 11 章：客户端-服务器编程、`rio` 健壮读写。
- OWASP：Path Traversal、Unvalidated Redirects 的防御清单。

---

下一章：01 · 可构建与跨平台（计划中）。在进入下一章前，请确认你能独立复现本章至少 5 个 P0。
