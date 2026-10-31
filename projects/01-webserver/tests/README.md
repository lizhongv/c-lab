# 复现脚本

`repro_p0.py` 只做「发送请求 + 观察现象」，不做断言——第 00 章的目标是看见问题，不是通过测试。

## 前置

1. 在 `../stages/00-baseline` 编译并启动基线服务器（见该目录 README）。
2. 确认端口（讲义示例为 8080；原项目默认 80）。

## 用法

```bash
python repro_p0.py --port 8080 --case traversal
python repro_p0.py --port 8080 --case post
python repro_p0.py --port 8080 --case recv-oob
python repro_p0.py --port 8080 --case slowloris
python repro_p0.py --port 8080 --case noext       # 会让服务器崩溃，最后跑
python repro_p0.py --port 8080 --case disconnect  # 会卡死服务，最后跑
```

`--case all` 只运行不会崩溃的 4 个用例。

## 注意

- `disconnect` 会在**当前目录**生成 8 MB 的 `big.bin`；请在服务器工作目录（`stages/00-baseline`）下运行，否则服务器找不到该文件。
- `noext` 依赖 `stages/00-baseline/noext` 空文件存在。
- 用例顺序：先 `traversal`/`post`/`recv-oob`，再 `slowloris`，最后 `noext`/`disconnect`；崩溃后重启服务器即可。

## 每个用例对应的讲义

见 [`../docs/00-baseline.md`](../docs/00-baseline.md) 的「事故档案」。
