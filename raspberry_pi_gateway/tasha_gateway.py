#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
踏沙四履种植者 - 树莓派网关
--------------------------------
作用：
1. 小程序通过 ws://树莓派IP:8765/ws 连接树莓派。
2. 树莓派通过串口连接 STM32，接收状态/传感器数据，转发小程序控制命令。
3. 可选读取 GPS NMEA 串口。
4. 串口未接入时自动模拟数据，便于先调小程序界面。

推荐硬件链路：
微信小程序 <Wi-Fi/IP> 树莓派 <USB-TTL/串口> STM32F407
"""

from __future__ import annotations

import asyncio
import json
import math
import os
import random
import signal
import time
from dataclasses import dataclass, asdict, field
from pathlib import Path
from threading import Lock, Thread
from typing import Any, Dict, Optional, Set

from aiohttp import web, WSMsgType

try:
    import serial
except Exception:  # pragma: no cover
    serial = None

try:
    import pynmea2
except Exception:  # pragma: no cover
    pynmea2 = None

CONFIG_PATH = Path(__file__).with_name("gateway_config.json")

STAGE_TEXT = {
    "idle": "待机",
    "drill": "钻孔",
    "seed": "送苗",
    "forward": "前进落苗",
    "cover": "覆土",
    "water": "浇水",
    "row_change": "换行",
    "complete": "完成",
    "error": "异常",
}

DIRECTION_TO_STM32 = {
    "forward": "F",
    "backward": "B",
    "left": "L",
    "right": "R",
}


def load_config() -> Dict[str, Any]:
    default = {
        "host": "0.0.0.0",
        "port": 8765,
        "stm32_serial": "/dev/ttyUSB0",
        "stm32_baudrate": 115200,
        "gps_serial": "/dev/ttyAMA0",
        "gps_baudrate": 9600,
        "simulate_when_serial_missing": True,
        "row_end_threshold_cm": 45,
        "telemetry_hz": 5,
    }
    if CONFIG_PATH.exists():
        try:
            default.update(json.loads(CONFIG_PATH.read_text(encoding="utf-8")))
        except Exception as exc:
            print(f"[WARN] 读取配置失败，使用默认配置：{exc}")
    return default


@dataclass
class ImuData:
    roll: float = 0.0
    pitch: float = 0.0
    yaw: float = 0.0
    valid: bool = False


@dataclass
class GpsData:
    lat: float = 0.0
    lng: float = 0.0
    speed: float = 0.0
    fix: bool = False
    sats: int = 0


@dataclass
class RadarData:
    left: int = 0
    front: int = 0
    right: int = 0


@dataclass
class BatteryData:
    voltage: float = 24.0
    percent: int = 80


@dataclass
class RobotState:
    online: bool = False
    mode: str = "manual"
    stage: str = "idle"
    stageText: str = "待机"
    stageDesc: str = "等待连接或等待启动"
    plant_count: int = 0
    total_count: int = 12
    row: int = 1
    point: int = 1
    progress: int = 0
    row_end: bool = False
    ultrasonic_cm: float = 0.0
    imu: ImuData = field(default_factory=ImuData)
    gps: GpsData = field(default_factory=GpsData)
    radar: RadarData = field(default_factory=RadarData)
    battery: BatteryData = field(default_factory=BatteryData)
    signal: int = 100
    last_update: float = field(default_factory=time.time)

    def normalize(self, row_end_threshold_cm: float) -> None:
        self.plant_count = max(0, min(int(self.plant_count), int(self.total_count)))
        self.total_count = max(1, int(self.total_count))
        self.progress = round((self.plant_count / self.total_count) * 100)
        self.row = max(1, math.ceil(max(1, self.plant_count + 1) / 4))
        self.point = ((max(1, self.plant_count + 1) - 1) % 4) + 1
        self.stageText = STAGE_TEXT.get(self.stage, self.stageText or "待机")
        self.row_end = bool(self.ultrasonic_cm > 0 and self.ultrasonic_cm <= row_end_threshold_cm)
        self.online = (time.time() - self.last_update) < 3.0

    def to_dict(self, row_end_threshold_cm: float) -> Dict[str, Any]:
        self.normalize(row_end_threshold_cm)
        data = asdict(self)
        data["last_update"] = round(self.last_update, 3)
        return data


class Gateway:
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.state = RobotState()
        self.lock = Lock()
        self.clients: Set[web.WebSocketResponse] = set()
        self.stm32: Optional[Any] = None
        self.running = True
        self.simulate = bool(config.get("simulate_when_serial_missing", True))
        self.last_serial_line = ""

    def start_background_threads(self) -> None:
        Thread(target=self._stm32_reader_loop, name="stm32-reader", daemon=True).start()
        Thread(target=self._gps_reader_loop, name="gps-reader", daemon=True).start()
        if self.simulate:
            Thread(target=self._simulate_loop, name="simulator", daemon=True).start()

    def _open_serial(self, port: str, baudrate: int, timeout: float = 0.1) -> Optional[Any]:
        if serial is None:
            print("[WARN] pyserial 未安装，串口不可用")
            return None
        try:
            ser = serial.Serial(port, baudrate=baudrate, timeout=timeout)
            print(f"[OK] 已打开串口 {port} @ {baudrate}")
            return ser
        except Exception as exc:
            print(f"[WARN] 无法打开串口 {port}: {exc}")
            return None

    def _stm32_reader_loop(self) -> None:
        port = self.config.get("stm32_serial", "/dev/ttyUSB0")
        baud = int(self.config.get("stm32_baudrate", 115200))
        self.stm32 = self._open_serial(port, baud)
        if self.stm32 is None:
            return

        while self.running:
            try:
                raw = self.stm32.readline()
                if not raw:
                    continue
                line = raw.decode("utf-8", errors="ignore").strip()
                if not line:
                    continue
                self.last_serial_line = line
                self._parse_stm32_line(line)
            except Exception as exc:
                print(f"[WARN] STM32 串口读取异常：{exc}")
                time.sleep(0.5)

    def _gps_reader_loop(self) -> None:
        port = self.config.get("gps_serial", "/dev/ttyAMA0")
        baud = int(self.config.get("gps_baudrate", 9600))
        gps_ser = self._open_serial(port, baud)
        if gps_ser is None or pynmea2 is None:
            return

        while self.running:
            try:
                line = gps_ser.readline().decode("ascii", errors="ignore").strip()
                if not line.startswith("$"):
                    continue
                msg = pynmea2.parse(line)
                with self.lock:
                    if hasattr(msg, "latitude") and hasattr(msg, "longitude"):
                        lat = float(msg.latitude or 0)
                        lng = float(msg.longitude or 0)
                        if lat and lng:
                            self.state.gps.lat = round(lat, 7)
                            self.state.gps.lng = round(lng, 7)
                            self.state.gps.fix = True
                    if hasattr(msg, "spd_over_grnd") and msg.spd_over_grnd is not None:
                        self.state.gps.speed = round(float(msg.spd_over_grnd) * 1.852, 2)
                    if hasattr(msg, "num_sats") and msg.num_sats:
                        self.state.gps.sats = int(msg.num_sats)
            except Exception:
                continue

    def _parse_stm32_line(self, line: str) -> None:
        """支持两种 STM32 上报格式：
        1) JSON 行：{"type":"telemetry", "stage":"drill", ...}
        2) CSV 行：$TS,tick,mode,stage,plant,total,ultra_mm,roll100,pitch100,yaw100,L,F,R,volt100,signal
        """
        try:
            if line.startswith("{"):
                msg = json.loads(line)
                data = msg.get("data", msg)
                self._merge_state(data)
                return

            if line.startswith("$TS,"):
                p = line.split(",")
                if len(p) < 15:
                    return
                data = {
                    "mode": p[2],
                    "stage": p[3],
                    "plant_count": int(p[4]),
                    "total_count": int(p[5]),
                    "ultrasonic_cm": round(int(p[6]) / 10.0, 1),
                    "imu": {
                        "roll": round(int(p[7]) / 100.0, 2),
                        "pitch": round(int(p[8]) / 100.0, 2),
                        "yaw": round(int(p[9]) / 100.0, 2),
                        "valid": True,
                    },
                    "radar": {
                        "left": int(p[10]),
                        "front": int(p[11]),
                        "right": int(p[12]),
                    },
                    "battery": {
                        "voltage": round(int(p[13]) / 100.0, 2),
                        "percent": self._voltage_to_percent(int(p[13]) / 100.0),
                    },
                    "signal": int(p[14]),
                }
                self._merge_state(data)
        except Exception as exc:
            print(f"[WARN] 解析 STM32 数据失败：{line} / {exc}")

    def _merge_state(self, data: Dict[str, Any]) -> None:
        with self.lock:
            for key in ["mode", "stage", "stageText", "stageDesc"]:
                if key in data:
                    setattr(self.state, key, str(data[key]))
            for key in ["plant_count", "total_count", "row", "point", "signal"]:
                if key in data:
                    setattr(self.state, key, int(float(data[key])))
            if "ultrasonic_cm" in data:
                self.state.ultrasonic_cm = round(float(data["ultrasonic_cm"]), 1)
            if "ultrasonic_mm" in data:
                self.state.ultrasonic_cm = round(float(data["ultrasonic_mm"]) / 10.0, 1)
            if "imu" in data and isinstance(data["imu"], dict):
                for k in ["roll", "pitch", "yaw"]:
                    if k in data["imu"]:
                        setattr(self.state.imu, k, round(float(data["imu"][k]), 2))
                if "valid" in data["imu"]:
                    self.state.imu.valid = bool(data["imu"]["valid"])
            if "gps" in data and isinstance(data["gps"], dict):
                for k in ["lat", "lng", "speed"]:
                    if k in data["gps"]:
                        setattr(self.state.gps, k, round(float(data["gps"][k]), 7 if k in ["lat", "lng"] else 2))
                for k in ["fix"]:
                    if k in data["gps"]:
                        setattr(self.state.gps, k, bool(data["gps"][k]))
                if "sats" in data["gps"]:
                    self.state.gps.sats = int(data["gps"]["sats"])
            if "radar" in data and isinstance(data["radar"], dict):
                for k in ["left", "front", "right"]:
                    if k in data["radar"]:
                        setattr(self.state.radar, k, int(float(data["radar"][k])))
            if "battery" in data and isinstance(data["battery"], dict):
                if "voltage" in data["battery"]:
                    self.state.battery.voltage = round(float(data["battery"]["voltage"]), 2)
                    self.state.battery.percent = self._voltage_to_percent(self.state.battery.voltage)
                if "percent" in data["battery"]:
                    self.state.battery.percent = max(0, min(100, int(data["battery"]["percent"])))
            self.state.last_update = time.time()
            self.state.online = True

    @staticmethod
    def _voltage_to_percent(voltage: float) -> int:
        # 6S 锂电粗略估算：满电 25.2V，低电 21.0V。根据你实际电池可在这里改。
        return max(0, min(100, round((voltage - 21.0) / (25.2 - 21.0) * 100)))

    def _simulate_loop(self) -> None:
        stages = ["drill", "forward", "seed", "cover", "water"]
        t0 = time.time()
        while self.running:
            if self.stm32 is not None:
                time.sleep(1.0)
                continue
            elapsed = time.time() - t0
            stage = stages[int(elapsed / 4) % len(stages)]
            plant = min(12, int(elapsed / 20))
            with self.lock:
                self.state.mode = "manual" if plant == 0 else "auto"
                self.state.stage = "complete" if plant >= 12 else stage
                self.state.plant_count = plant
                self.state.ultrasonic_cm = round(80 - (elapsed % 25) * 1.7, 1)
                if self.state.ultrasonic_cm < 25:
                    self.state.ultrasonic_cm = 95
                self.state.imu.roll = round(random.uniform(-2.5, 2.5), 2)
                self.state.imu.pitch = round(random.uniform(-3.0, 3.0), 2)
                self.state.imu.yaw = round((elapsed * 4.2) % 360, 2)
                self.state.imu.valid = True
                self.state.gps.lat = 19.997451
                self.state.gps.lng = 110.349228
                self.state.gps.fix = True
                self.state.gps.sats = 10
                self.state.gps.speed = round(random.uniform(0, 0.7), 2)
                self.state.radar.left = int(230 + random.uniform(-20, 20))
                self.state.radar.front = int(90 + random.uniform(-15, 15))
                self.state.radar.right = int(210 + random.uniform(-20, 20))
                self.state.battery.voltage = round(24.2 - plant * 0.03, 2)
                self.state.battery.percent = self._voltage_to_percent(self.state.battery.voltage)
                self.state.signal = random.randint(86, 100)
                self.state.last_update = time.time()
            time.sleep(0.2)

    def write_stm32_cmd(self, cmd: str, **payload: Any) -> None:
        line = self._build_stm32_command(cmd, **payload)
        if self.stm32 is None:
            print(f"[SIM-CMD] {line.strip()}")
            # 模拟阶段变化，便于演示
            if cmd == "estop":
                self._merge_state({"stage": "error", "stageDesc": "已触发小程序急停"})
            elif cmd == "start_auto":
                self._merge_state({"mode": "auto", "stage": "drill", "plant_count": 0})
            elif cmd == "reset_count":
                self._merge_state({"plant_count": 0, "stage": "idle"})
            return
        try:
            self.stm32.write(line.encode("ascii"))
            self.stm32.flush()
            print(f"[TX->STM32] {line.strip()}")
        except Exception as exc:
            print(f"[WARN] 发送 STM32 指令失败：{exc}")

    @staticmethod
    def _build_stm32_command(cmd: str, **payload: Any) -> str:
        if cmd == "move":
            direction = DIRECTION_TO_STM32.get(str(payload.get("dir", "stop")), "S")
            speed = int(payload.get("speed", 60))
            speed = max(10, min(90, speed))
            return f"@CMD,MOVE,{direction},{speed}\n"
        if cmd == "stop":
            return "@CMD,STOP\n"
        if cmd == "start_auto":
            return "@CMD,AUTO,START\n"
        if cmd == "pause":
            return "@CMD,AUTO,PAUSE\n"
        if cmd == "resume":
            return "@CMD,AUTO,RESUME\n"
        if cmd == "reset_count":
            return "@CMD,COUNT,RESET\n"
        if cmd == "estop":
            return "@CMD,ESTOP\n"
        return f"@CMD,{cmd.upper()}\n"


async def ws_handler(request: web.Request) -> web.WebSocketResponse:
    gw: Gateway = request.app["gw"]
    ws = web.WebSocketResponse(heartbeat=10)
    await ws.prepare(request)
    gw.clients.add(ws)
    await ws.send_json({"type": "ack", "message": "connected"})

    try:
        async for msg in ws:
            if msg.type == WSMsgType.TEXT:
                try:
                    data = json.loads(msg.data)
                except Exception:
                    await ws.send_json({"type": "error", "message": "bad json"})
                    continue
                if data.get("type") == "ping":
                    await ws.send_json({"type": "pong", "ts": time.time()})
                elif data.get("type") == "cmd":
                    cmd = str(data.get("cmd", ""))
                    gw.write_stm32_cmd(cmd, **data)
                    await ws.send_json({"type": "ack", "cmd": cmd, "message": "sent"})
                elif data.get("type") == "hello":
                    await ws.send_json({"type": "ack", "message": "hello"})
            elif msg.type == WSMsgType.ERROR:
                print(f"[WS] error: {ws.exception()}")
    finally:
        gw.clients.discard(ws)
    return ws


async def status_handler(request: web.Request) -> web.Response:
    gw: Gateway = request.app["gw"]
    with gw.lock:
        data = gw.state.to_dict(float(gw.config.get("row_end_threshold_cm", 45)))
    return web.json_response({"type": "telemetry", "data": data, "last_serial_line": gw.last_serial_line})


async def cmd_handler(request: web.Request) -> web.Response:
    gw: Gateway = request.app["gw"]
    data = await request.json()
    cmd = str(data.get("cmd", ""))
    gw.write_stm32_cmd(cmd, **data)
    return web.json_response({"ok": True, "cmd": cmd})


async def broadcaster(app: web.Application) -> None:
    gw: Gateway = app["gw"]
    hz = max(1, int(gw.config.get("telemetry_hz", 5)))
    interval = 1.0 / hz
    while gw.running:
        with gw.lock:
            data = gw.state.to_dict(float(gw.config.get("row_end_threshold_cm", 45)))
        dead = []
        for ws in list(gw.clients):
            try:
                await ws.send_json({"type": "telemetry", "data": data})
            except Exception:
                dead.append(ws)
        for ws in dead:
            gw.clients.discard(ws)
        await asyncio.sleep(interval)


async def on_startup(app: web.Application) -> None:
    app["broadcast_task"] = asyncio.create_task(broadcaster(app))


async def on_cleanup(app: web.Application) -> None:
    gw: Gateway = app["gw"]
    gw.running = False
    task = app.get("broadcast_task")
    if task:
        task.cancel()


def create_app() -> web.Application:
    config = load_config()
    gw = Gateway(config)
    gw.start_background_threads()

    app = web.Application()
    app["gw"] = gw
    app.router.add_get("/ws", ws_handler)
    app.router.add_get("/api/status", status_handler)
    app.router.add_post("/api/cmd", cmd_handler)
    app.on_startup.append(on_startup)
    app.on_cleanup.append(on_cleanup)
    return app


if __name__ == "__main__":
    cfg = load_config()
    print("踏沙树莓派网关启动")
    print(f"WebSocket: ws://<树莓派IP>:{cfg['port']}/ws")
    print(f"HTTP状态: http://<树莓派IP>:{cfg['port']}/api/status")
    web.run_app(create_app(), host=cfg.get("host", "0.0.0.0"), port=int(cfg.get("port", 8765)))
