# STM32 与树莓派通信协议说明

本包的小程序不是直接连 STM32，而是：

```text
微信小程序  <--WebSocket/IP-->  树莓派  <--USART2/USB-TTL-->  STM32F407
```

STM32 工程中需要提供以下变量或功能：

- `ultrasonic_distance_mm`：超声波距离，单位 mm。
- `imu_roll_x100 / imu_pitch_x100 / imu_yaw_x100`：IMU 姿态角，单位为角度 ×100。
- `radar_left_distance_cm / radar_front_distance_cm / radar_right_distance_cm`：USART1 雷达距离，单位 cm。
- `manual_snake_resume_seed`：下一颗苗索引，1~12；0 表示完成。
- `manual_snake_resume_phase`：当前自动流程阶段。
- `PlantOneSeedling(seed_index)`：单颗苗动作流程。
- `ManualCar_AllStop()`：安全停止所有执行机构。

## 1. STM32 上报格式

树莓派网关支持两种上报格式，推荐使用轻量 CSV：

```text
$TS,tick,mode,stage,plant,total,ultra_mm,roll100,pitch100,yaw100,L,F,R,volt100,signal\n
```

示例：

```text
$TS,123456,auto,water,3,12,385,10,-25,9025,253,52,88,2400,96
```

字段含义：

| 字段 | 含义 |
| --- | --- |
| tick | `HAL_GetTick()` |
| mode | `manual` / `auto` |
| stage | `idle` / `drill` / `seed` / `forward` / `cover` / `water` / `row_change` / `complete` / `error` |
| plant | 已完成苗数。注意：浇水完成才 +1 |
| total | 总苗数，当前为 12 |
| ultra_mm | 超声波距离，单位 mm |
| roll100/pitch100/yaw100 | IMU 角度 ×100 |
| L/F/R | 雷达左/前/右距离，单位 cm；没有雷达可填 0 |
| volt100 | 电池电压 ×100；没有电压采集可先填 2400 |
| signal | 通信信号/健康度，0~100；没有可先填 100 |

## 2. 树莓派下发指令格式

树莓派向 STM32 发送：

```text
@CMD,MOVE,F,60\n       # 前进，速度 60%
@CMD,MOVE,B,60\n       # 后退，速度 60%
@CMD,MOVE,L,60\n       # 左转，速度 60%
@CMD,MOVE,R,60\n       # 右转，速度 60%
@CMD,STOP\n            # 停车
@CMD,AUTO,START\n      # 启动自动种植
@CMD,AUTO,PAUSE\n      # 暂停/请求中止当前自动动作
@CMD,AUTO,RESUME\n     # 继续
@CMD,COUNT,RESET\n     # 苗数清零
@CMD,ESTOP\n           # 急停
```

## 3. 与 STM32 工程的集成

STM32 侧的 `manual_car_control` 环境已包含大部分底层动作，但要做到“小程序实时显示钻孔/送苗/前进/覆土/浇水”和“浇水完成苗数 +1”，必须让 STM32 在关键阶段上报状态。

最小改法：

1. 在 `src/main.c` 的 `CAR_GPIO_MANUAL_TEST_ONLY` 分支里增加 `mini_program_stm32_patch.c` 中的状态变量和函数。
2. 在 `PlantOneSeedling()` 每个阶段前调用 `Mini_SetStage("drill")` 等。
3. 在 `ManualCar_WaterAuto()` 返回后，也就是浇水完成后，调用 `Mini_SetPlantCount(seed_index)`。
4. 在主循环里周期调用 `Mini_TelemetryTask()`，例如每 200ms 上报一次。
5. 在 `HAL_UART_RxCpltCallback()` 的 USART2 分支里调用 `Mini_Uart2PushByte(uart2_rx_byte)`，解析树莓派下发指令。

示例代码见 `mini_program_stm32_patch.c`。它不是独立编译单元，需按上表位置并入现有 `main.c`。
