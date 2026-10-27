#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt
printf '\n安装完成。启动命令：\n  . .venv/bin/activate && python tasha_gateway.py\n'
