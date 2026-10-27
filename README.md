# 踏沙四履种植者 · Tasha

小型履带式沙地全自动种苗移栽机器人 —— 微信小程序 + 树莓派网关 + STM32F407 固件

**Author**: yuanjuna

> 面向海南沙地小块果园的种苗移栽机器人：STM32 负责底盘与种植机构执行，树莓派做通信中转，
> 微信小程序提供远程管控，三层解耦。

## 系统架构

```
微信小程序  <-- WebSocket / Wi-Fi -->  树莓派 4B  <-- USART2 -->  STM32F407
  用户交互层                            感知决策层                  底层执行层
```

| 层级 | 主要组成 | 职责 | 代码位置 |
| --- | --- | --- | --- |
| 用户交互层 | 微信小程序、视频流、状态面板 | 参数配置、自动/手动切换、远程监控、人工接管 | `tasha-mini-program/` |
| 感知决策层 | 树莓派 4B、激光雷达、摄像头、超声波、IMU | 行向识别、任务状态机、路径决策、通信中转 | `raspberry_pi_gateway/` |
| 底层执行层 | STM32F407ZGT6、履带底盘、种植头、水泵 | 闭环运动控制、种植机构时序、急停与故障降级 | `firmware/` |

固件是独立可编译的 PlatformIO 工程，构建、引脚表、蛇形种植流程与诊断环境见
[`firmware/README.md`](firmware/README.md)。

> `stm32_patch/` 提供小程序遥测接入所需的补丁与协议说明。**该补丁尚未合入
> `firmware/src/main.c`** —— 固件目前跑完整的底盘与种植流程，但不向树莓派上报状态。

## 目录结构

```
.
├── tasha-mini-program/          微信小程序
│   ├── app.js / app.json / app.wxss
│   ├── pages/
│   │   ├── overview/            总览大屏：核心能力、当前状态、种植进度
│   │   ├── control/             远程控制：前后左右、无级调速、自动/手动
│   │   └── sensor/              传感与定位：超声波、IMU、GPS、雷达、电池
│   └── images/tab/              TabBar 图标
├── raspberry_pi_gateway/        树莓派网关（Python）
│   ├── tasha_gateway.py         aiohttp WebSocket 服务 + 串口桥接
│   ├── gateway_config.json      端口、串口、波特率、模拟开关、上报频率
│   ├── requirements.txt         aiohttp / pyserial / pynmea2
│   └── install.sh               一键建虚拟环境并装依赖
├── firmware/                    STM32F407 固件（PlatformIO）
│   ├── src/                     主程序与应用层：底盘、步进、执行机构、流程、IMU、LCD
│   ├── src/diag/                20 个外设单点诊断程序
│   ├── include/                 引脚定义、HAL 配置、LCD 驱动头
│   ├── platformio.ini           构建配置与全部环境
│   └── docs/                    操作与参数说明
└── stm32_patch/                 小程序遥测接入补丁（待合入 main.c）
    ├── mini_program_stm32_patch.c   状态上报与指令解析示例代码
    └── MINIPROGRAM_PROTOCOL.md      通信协议说明（含字段表与最小改法）
```

## 功能特性

**小程序（三 Tab）**

- **总览大屏** —— 核心能力、设备状态、种植进度、快捷入口，面向平板展示优化
- **远程控制** —— 前进/后退/左转/右转、无级调速、停车与急停、自动模式启停与计数清零
- **传感与定位** —— 超声波距离、IMU 三轴姿态、GPS 定位、雷达左/前/右测距、电池电压、通信健康度

**树莓派网关**

- 对小程序提供 WebSocket 服务（默认 `ws://<pi-ip>:8765/ws`），广播遥测、接收控制命令
- 对 STM32 走串口：接收 CSV 遥测帧，下发 `@CMD` 指令
- 串口未接入时自动进入模拟模式，方便先联调小程序界面
- 可选读取 GPS NMEA 数据（`pynmea2`）

**布局自适应**

启动时读取设备宽度：平板启用 `pad-mode`（信息密度更高），手机启用 `phone-mode`（单列卡片）。
横竖屏或窗口尺寸变化时自动重新判断。

## 通信协议

**STM32 → 树莓派（CSV 遥测，每 200 ms 一帧）**

```
$TS,tick,mode,stage,plant,total,ultra_mm,roll100,pitch100,yaw100,L,F,R,volt100,signal
```

示例：

```
$TS,123456,auto,water,3,12,385,10,-25,9025,253,52,88,2400,96
```

| 字段 | 含义 |
| --- | --- |
| `tick` | `HAL_GetTick()` |
| `mode` | `manual` / `auto` |
| `stage` | `idle` / `drill` / `seed` / `forward` / `cover` / `water` / `row_change` / `complete` / `error` |
| `plant` / `total` | 已栽植数 / 总数（浇水完成才计入） |
| `ultra_mm` | 超声波距离，mm |
| `roll100` / `pitch100` / `yaw100` | IMU 姿态角 ×100 |
| `L` / `F` / `R` | 雷达左/前/右距离，cm |
| `volt100` | 电池电压 ×100 |
| `signal` | 通信健康度 0~100 |

**树莓派 → STM32（指令）**

| 指令 | 作用 |
| --- | --- |
| `@CMD,MOVE,F,60` | 前进，速度 60% |
| `@CMD,MOVE,B,60` | 后退 |
| `@CMD,MOVE,L,60` / `@CMD,MOVE,R,60` | 左转 / 右转 |
| `@CMD,STOP` | 停车 |
| `@CMD,AUTO,START` / `PAUSE` / `RESUME` | 自动作业启停 |
| `@CMD,COUNT,RESET` | 苗数清零 |
| `@CMD,ESTOP` | 急停 |

完整字段与 STM32 侧最小改法见 `stm32_patch/MINIPROGRAM_PROTOCOL.md`。

## 快速开始

**1. 树莓派网关**

```bash
cd raspberry_pi_gateway
bash install.sh
source .venv/bin/activate && python tasha_gateway.py
```

默认监听 `0.0.0.0:8765`，串口 `/dev/ttyUSB0` @115200，GPS `/dev/ttyAMA0` @9600。
参数在同目录 `gateway_config.json` 中调整：

| 配置项 | 默认值 | 说明 |
| --- | --- | --- |
| `port` | 8765 | WebSocket 服务端口 |
| `stm32_serial` / `stm32_baudrate` | `/dev/ttyUSB0` / 115200 | STM32 串口 |
| `gps_serial` / `gps_baudrate` | `/dev/ttyAMA0` / 9600 | GPS 串口 |
| `simulate_when_serial_missing` | true | 无串口时自动模拟数据 |
| `row_end_threshold_cm` | 45 | 行末检测阈值 |
| `telemetry_hz` | 5 | 遥测上报频率 |

**2. 微信小程序**

用微信开发者工具打开 `tasha-mini-program` 目录。

> 仓库中的 `project.config.json` 里 `appid` 已替换为占位值 `touristappid`，
> 导入后需填入你自己的小程序 AppID 才能真机预览（本地模拟器可直接运行）。

在「远程控制」页填写树莓派地址，示例 `192.168.4.1:8765`；未接真实设备时网关会回模拟数据。

## 注意事项

- 小程序不直连 STM32，必须经树莓派网关中转
- 自动流程中人工接管优先级高于自动指令；`ESTOP` 为最高优先级的全局停止
- 断网时 STM32 侧应转入本地安全策略，不应依赖网络持续存在

## License

[MIT](LICENSE)
