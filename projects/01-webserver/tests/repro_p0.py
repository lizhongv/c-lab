"""复现 WebServer1.0 的 P0 缺陷。

用法（先启动基线服务器，默认 127.0.0.1:8080）：

    python repro_p0.py --case traversal
    python repro_p0.py --case recv-oob
    python repro_p0.py --case disconnect     # 需在本目录或服务器工作目录生成 big.bin
    python repro_p0.py --case noext          # 会让服务器崩溃，最后再跑
    python repro_p0.py --case post
    python repro_p0.py --case slowloris
    python repro_p0.py --case all            # 跳过 noext 与 disconnect

脚本只做「发送请求 + 观察现象」，不做断言：基线是教材，不是被测系统。
判定标准写在每个用例的说明里，与 docs/00-baseline.md 对应。
"""

import argparse
import os
import socket
import struct
import sys
import time


HOST = "127.0.0.1"
PORT = 8080


def connect(timeout: float = 5.0) -> socket.socket:
    sock = socket.create_connection((HOST, PORT), timeout=timeout)
    sock.settimeout(timeout)
    return sock


def send_and_read(payload: bytes, timeout: float = 5.0, limit: int = 65536) -> bytes:
    """发一个请求，读到对端关闭或读满 limit。"""
    with connect(timeout) as sock:
        sock.sendall(payload)
        sock.shutdown(socket.SHUT_WR)
        chunks = []
        total = 0
        while total < limit:
            try:
                data = sock.recv(min(4096, limit - total))
            except socket.timeout:
                break
            if not data:
                break
            chunks.append(data)
            total += len(data)
        return b"".join(chunks)


def status_line(response: bytes) -> str:
    first = response.split(b"\r\n", 1)[0]
    return first.decode("latin-1", "replace") or "(无响应)"


def case_traversal() -> None:
    """P0-1：GET /../../README.md 读到工作目录之外的文件。"""
    payload = b"GET /../../README.md HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
    response = send_and_read(payload)
    print("状态行：", status_line(response))
    print("正文前 80 字节：", response.split(b"\r\n\r\n", 1)[-1][:80])
    if response.startswith(b"HTTP/1.1 200"):
        print("判定：复现成功——请求路径里的 .. 被原样用于 fopen，读到了上级目录的文件。")
    else:
        print("判定：未复现。确认服务器工作目录是 stages/00-baseline，且上级存在 README.md。")


def case_recv_oob() -> None:
    """P0-2：一次发送恰好 1024 字节，recv 返回值被用作下标。"""
    prefix = b"GET /"
    suffix = b" HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
    payload = prefix + b"a" * (1024 - len(prefix) - len(suffix)) + suffix
    assert len(payload) == 1024, len(payload)
    print("发送字节数：", len(payload))
    try:
        response = send_and_read(payload, timeout=3)
        print("状态行：", status_line(response))
    except OSError as error:
        print("连接异常：", error)
    print("判定：revbuf[1024]=0 已写到数组之外；不一定崩溃，但属于未定义行为。")
    print("      第 01 章用 ASan 复现，第 06 章加入回归。")


def case_disconnect() -> None:
    """P0-3：下载中途 RST 断开，服务器空转到文件末尾；send 返回 0 时才是死循环。"""
    big = os.path.abspath("big.bin")
    if not os.path.exists(big):
        with open(big, "wb") as f:
            f.write(b"A" * (64 * 1024 * 1024))
        print("已生成", big, "（64 MB；8 MB 太小，空转过程短到察觉不到）")
    sock = connect()
    sock.sendall(b"GET /big.bin HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n")
    sock.recv(4096)
    # SO_LINGER(on, 0) 让 close 发 RST 而不是正常四次挥手，服务端 send 会立刻失败
    linger = struct.pack("HH", 1, 0)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_LINGER, linger)
    sock.close()
    print("已发送 RST 断开。")
    time.sleep(0.5)
    start = time.time()
    try:
        response = send_and_read(b"GET /test-file/txt/Genesis.txt HTTP/1.1\r\n\r\n", timeout=3)
        cost = time.time() - start
        print(f"后续请求：{status_line(response)}（{cost:.2f}s）")
        print("判定：若明显变慢或超时，说明单线程在空转读文件；文件越大越明显。")
        print("      send 返回 0 时才会真正死循环（代码走读，见 other.c:10-19）。")
    except socket.timeout:
        print("后续请求超时——服务器仍被这个断开的下载占用。")
    except OSError as error:
        print("后续请求失败：", error)


def case_noext() -> None:
    """P0-4：请求存在但无扩展名的文件，strrchr 返回 NULL。"""
    payload = b"GET /noext HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n"
    try:
        response = send_and_read(payload, timeout=3)
        print("状态行：", status_line(response))
        print("判定：未崩溃。确认 noext 文件在服务器工作目录，且请求成功走到了 Send_File。")
    except OSError as error:
        print("连接异常：", error)
        print("判定：若连接被重置且服务器已退出，即为复现成功（strcmp(NULL) 崩溃）。")


def case_post() -> None:
    """P0-5：Judge_Method(...) == \"POST\" 比较指针。"""
    payload = (
        b"POST /test-file/txt/Genesis.txt HTTP/1.1\r\n"
        b"Host: 127.0.0.1\r\nContent-Length: 7\r\n\r\na=1&b=2"
    )
    response = send_and_read(payload)
    print("状态行：", status_line(response))
    print("判定：看服务器控制台是否打印 Post Value。打印与否取决于编译器是否合并字符串常量，")
    print("      两种结果都说明分支判断不可靠；正确做法是 strcmp 或返回枚举。")


def case_slowloris() -> None:
    """P0-6：占住一个连接不发完整请求，后续请求被阻塞。"""
    idle = connect(timeout=20)
    idle.sendall(b"GET / HTTP/1.1\r\n")
    print("已占住一个连接（只发送了半行请求），保持 15 秒。")
    time.sleep(1)
    start = time.time()
    try:
        response = send_and_read(b"GET /test-file/txt/Genesis.txt HTTP/1.1\r\n\r\n", timeout=3)
        cost = time.time() - start
        print(f"后续请求：{status_line(response)}（{cost:.2f}s）")
        print("判定：若在 3 秒内成功，说明服务端已并发处理；基线应为超时。")
    except socket.timeout:
        print("后续请求 3 秒超时——复现成功：单线程阻塞在第一个连接的 recv 上。")
    finally:
        idle.close()
        print("已释放占住的连接。")


CASES = {
    "traversal": case_traversal,
    "recv-oob": case_recv_oob,
    "disconnect": case_disconnect,
    "noext": case_noext,
    "post": case_post,
    "slowloris": case_slowloris,
}


def main() -> int:
    global HOST, PORT
    parser = argparse.ArgumentParser(description="复现 WebServer1.0 的 P0 缺陷")
    parser.add_argument("--host", default=HOST)
    parser.add_argument("--port", type=int, default=PORT)
    parser.add_argument("--case", default="all", choices=[*CASES, "all"])
    args = parser.parse_args()
    HOST, PORT = args.host, args.port

    order = ["traversal", "post", "recv-oob", "slowloris"] if args.case == "all" else [args.case]
    for name in order:
        print(f"\n=== {name} ===")
        try:
            CASES[name]()
        except ConnectionRefusedError:
            print("连接被拒绝：服务器未启动，或端口不对。", file=sys.stderr)
            return 1
        except OSError as error:
            print("连接异常：", error)
    if args.case == "all":
        print("\nnoext 与 disconnect 会崩溃或卡死服务，请单独执行。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
