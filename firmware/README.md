# 踏沙四履种植者 · 移苗装置固件

STM32F407ZGT6 全自动移栽机器人底层固件 —— 履带底盘运动控制、遥控接管、蛇形路线自动种植流程。

**Author**: yuanjuna

## 硬件

| 项目 | 规格 |
| --- | --- |
| 主控 | STM32F407ZGT6（`black_f407zg` 开发板） |
| 构建 | PlatformIO + `stm32cube` HAL |
| 烧写/调试 | CMSIS-DAP |
| 显示 | ALIENTEK 4.3 寸屏（`LCD_FORCE_ID=0x9806`） |
| 实时系统 | FreeRTOS（`freertos_sources.py` 从 CubeF4 包引入，主流程编译时未启用） |

## 引脚分配

### 小车运动

| 功能 | 引脚 | 说明 |
| --- | --- | --- |
| 左驱 FWD | PA5 | 左轮正转 |
| 左驱 REV | PB1 | 左轮反转 |
| 右驱 FWD | PB12 | 右轮正转 |
| 右驱 REV | PA6 | 右轮反转 |
| 左轮调速 PWM | PD12 | `TIM4_CH1`，经 5V 电平转换 |
| 右轮调速 PWM | PD13 | `TIM4_CH2`，经 5V 电平转换 |

### 遥控与模式

| 功能 | 引脚 | 说明 |
| --- | --- | --- |
| 模式开关 | PC7 | `TIM8_CH2` 输入捕获，约 1000 µs 手动 / 约 2000 µs 自动 |
| 遥控转向 | PA0 | `TIM2_CH1` |
| 遥控油门 | PC6 | `TIM8_CH1` |
| 手动电钻 | PB14 | `TIM12_CH1`，仅手动模式有效 |
| 手动覆土 / 自动速度档位 | PE6 | `EXTI6`，仅手动模式有效 |
| 手动送苗 / 水泵测试 | PF11 | EXTI 输入捕获，仅手动模式有效 |

### 种植执行机构

| 功能 | 引脚 | 说明 |
| --- | --- | --- |
| 移动电钻 STEP | PC11 | 步进脉冲，TIM3 中断输出 |
| 移动电钻 DIR | PC12 | 步进方向 |
| 电钻继电器 | PC10 | 钻孔流程按时序开关 |
| 水泵继电器 | PE5 | 每颗苗浇水 3 秒 |
| 送苗脉冲 | PB0 | 125 个低脉冲为一步 |
| 覆土 IN1 / IN2 | PG2 / PG3 | 自动流程只用这一组 |
| 太阳能板 IN1 / IN2 | PG4 / PG5 | 自动种植流程保持停止 |
| IMU SCL / SDA | PB10 / PB11 | 仅读取显示，不参与自动换行 |
| 指示灯 | PF9 / PF10 | 状态指示 |

## 自动种植流程

PC7 处于自动档（≥1700 µs）时启动一次，共 **3 行 × 4 颗 = 12 颗苗**，蛇形路线：

1. 第 1 行种第 1–4 颗
2. 第 4 颗后：前进 2 s → 右转 4 s → 前进 2 s → 右转 5 s → 前进 2 s
3. 第 2 行种第 5–8 颗，方向与第 1 行相反
4. 第 8 颗后：前进 2 s → 左转 4 s → 前进 2 s → 左转 5 s → 前进 2 s
5. 第 3 行种第 9–12 颗
6. 第 12 颗后调用 `ManualCar_AllStop()` 全部停止

单颗苗流程 `PlantOneSeedling(seed_index)`：

1. 移动电钻下移 8 s（第 5 s 起电钻开启）→ 停 2 s → 上移 8 s（第 4 s 起电钻关闭）
2. 小车前进 0.5 s
3. PB0 送苗走一步
4. 覆土 PG2/PG3：伸出 10 s → 停 1 s → 缩回 10 s
5. 水泵 PE5 开启 3 s
6. 第 4、8、12 颗不执行普通前进；其余颗前进 1 s 到下一株位

自动流程运行中若 PC7 切回手动范围，立即 `ManualCar_AllStop()` 退出。完整跑完后若 PC7 仍保持自动档不会重复启动，需先切回手动再切自动。

## 构建与烧写

```bash
pio run -e manual_car_control                 # 编译
pio run -e manual_car_control -t upload       # 编译并烧写
```

主程序环境是 `manual_car_control`（`platformio.ini` 默认环境）。它会初始化 LCD 并进入完整手动/自动流程分支
（`CAR_GPIO_MANUAL_TEST_ONLY`）。

## 外设诊断环境

`src/diag/` 下 20 个 `main_*_test.c` 是硬件调试时逐个外设验证用的独立程序，每个对应
`platformio.ini` 里一个环境，编译时只包含自身，不参与主流程：

| 类别 | 环境示例 |
| --- | --- |
| 舵机 / MG996R | `servo_13_test`、`servo_basic_test`、`mg996r_pwm_test`、`servo_pa1_pwm_test` |
| 步进电机 28BYJ-48 | `stepper_28byj48_test`、`stepper_28byj48_altpins_test` |
| PWM 波形 | `pwm_wave_soft_test`、`pwm_wave_tim2_test`、`pa1_1khz_pwm_test`、`dual_motor_pwm_1khz_30_test` |
| 整车 / 单机构 | `car_test_only`、`car_turn_test_only`、`drill_test_only`、`cover_test_only`、`conveyor_test_only` |
| 传感器 / 显示 | `imu_test_only`、`monitor_only`、`all_features` |

`一键烧写电转测试.bat` 和 `一键烧写移苗装置测试.bat` 是对应诊断环境的免命令行入口。

## 目录结构

```
.
├── platformio.ini          构建配置与全部环境
├── freertos_sources.py     引入 CubeF4 包内的 FreeRTOS 源文件
├── include/
│   ├── app.h               全部引脚定义与对外声明
│   ├── BSP/LCD/            LCD 驱动头与寄存器初始化（lcd.c 以 #include 方式内联 lcd_ex.c）
│   └── SYSTEM/             delay / sys / usart
├── src/
│   ├── main.c              主入口与各编译分支
│   ├── app_car.c           底盘运动与差速
│   ├── app_stepper.c       步进电机
│   ├── app_actuator.c      电钻、水泵、覆土等执行机构
│   ├── app_workflow.c      自动种植流程
│   ├── app_uart.c          串口与雷达
│   ├── app_imu.c           IMU 读取
│   ├── app_ultrasonic.c    超声波
│   ├── app_lcd_ui.c        LCD 界面
│   ├── app_board.c / app_service.c / app_state.c / app_rtos.c
│   ├── BSP/LCD/lcd.c
│   └── diag/               20 个外设单点诊断程序
└── docs/                   操作与参数说明
```

## 安全提示

- 烧写或上电 `manual_car_control` 前，把 PC7 模式开关置于**手动档**，避免上电即进入自动流程
- 做转向测试时尽量架空轮子或断开非底盘执行机构电源
- `mobile_test_only` 会同时操作 PC10，如果 PC10 接了真实继电器，不能当成"只动步进电机"的安全测试
- 自动流程结束后由 `ManualCar_AllStop()` 统一关断全部输出

## License

[MIT](LICENSE)。`src/BSP/LCD` 与 `include/BSP/LCD` 下的 LCD 驱动参考正点原子（ALIENTEK）公开例程。
