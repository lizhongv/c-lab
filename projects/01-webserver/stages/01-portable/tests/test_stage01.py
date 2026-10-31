"""第 01 章的集成测试：启动服务器，用原始 socket 发请求，验证可构建后的基本行为。

用法：python test_stage01.py <webserver 可执行文件>
覆盖：
  1. GET 存在的文件 → 200 + 正确正文
  2. GET 不存在的文件 → 404
  3. 一次发 1024 字节请求 → 服务器不崩溃，之后仍能正常响应
  4. 未知方法 → 501
"""

import os
import socket
import subprocess
import sys
import tempfile
import time


def free_port() -> int:
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def wait_ready(port: int, timeout: float = 10.0) -> None:
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            with socket.create_connection(("127.0.0.1", port), timeout=0.5):
                return
        except OSError:
            time.sleep(0.05)
    raise RuntimeError(f"服务器未在 {timeout} 秒内就绪")


def request(port: int, payload: bytes, limit: int = 65536, timeout: float = 5.0) -> bytes:
    with socket.create_connection(("127.0.0.1", port), timeout=timeout) as sock:
        sock.sendall(payload)
        sock.shutdown(socket.SHUT_WR)
        chunks = []
        total = 0
        while total < limit:
            try:
                data = sock.recv(min(4096, limit - total))
            except socket.timeout:
                break
            except ConnectionResetError:
                # 服务端在未读完请求时就 close，Windows 会以 RST 结束；这本身是被测行为之一。
                break
            if not data:
                break
            chunks.append(data)
            total += len(data)
        return b"".join(chunks)


def body_of(response: bytes) -> bytes:
    """第 01 章仍保留 Content-Length 后使用 LF 的缺陷，因此两种分隔都要兼容。"""
    for separator in (b"\r\n\r\n", b"\n\r\n"):
        if separator in response:
            return response.split(separator, 1)[1]
    return b""


def check(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def main() -> int:
    if len(sys.argv) != 2:
        print("用法：python test_stage01.py <webserver 可执行文件>", file=sys.stderr)
        return 2
    executable = os.path.abspath(sys.argv[1])
    port = free_port()

    with tempfile.TemporaryDirectory(prefix="clab-stage01-") as workdir:
        with open(os.path.join(workdir, "hello.txt"), "w", encoding="utf-8", newline="\n") as f:
            f.write("hello c-lab\n")
        with open(os.path.join(workdir, "page.html"), "w", encoding="utf-8", newline="\n") as f:
            f.write("<h1>c-lab</h1>\n")

        process = subprocess.Popen(
            [executable, "--port", str(port)],
            cwd=workdir,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
        )
        try:
            wait_ready(port)

            response = request(port, b"GET /hello.txt HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n")
            check(response.startswith(b"HTTP/1.1 200 OK"), f"GET hello.txt 状态行错误：{response[:40]!r}")
            check(body_of(response) == b"hello c-lab\n", f"GET hello.txt 正文错误：{body_of(response)!r}")

            response = request(port, b"GET /missing.txt HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n")
            check(response.startswith(b"HTTP/1.1 404 Not Found"), f"GET 不存在文件应 404：{response[:40]!r}")

            # 1024 字节请求：第 00 章会越界写，第 01 章应安全处理并保持存活
            payload = b"GET /" + b"a" * (1024 - len(b"GET /") - len(b" HTTP/1.1\r\n\r\n")) + b" HTTP/1.1\r\n\r\n"
            check(len(payload) == 1024, "构造的请求长度不是 1024")
            request(port, payload, timeout=3.0, limit=4096)
            response = request(port, b"GET /page.html HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n")
            check(response.startswith(b"HTTP/1.1 200 OK"), "1024 字节请求之后服务器不再正常响应")

            response = request(port, b"PUT /hello.txt HTTP/1.1\r\nHost: 127.0.0.1\r\n\r\n")
            check(response.startswith(b"HTTP/1.1 501"), f"未知方法应返回 501：{response[:40]!r}")
        finally:
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
            output = process.stdout.read() if process.stdout else ""
            if process.returncode not in (0, -15, 1, 143):
                print(output, file=sys.stderr)

    print("stage 01 integration: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
