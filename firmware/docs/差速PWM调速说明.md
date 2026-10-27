# 手动小车差速 PWM 调速说明

本文记录当前已经验证并合入 `manual_car_control` 的小车手动调速方案。

## 已验证硬件链路

- STM32 输出为 3.3V PWM。
- 绿色 YUEXIN 万能信号电平转换模块可把 3.3V PWM 转为 5V PWM。
- 示波器已经确认模块输出 5V PWM。
- 电机驱动器可以识别该 5V PWM 调速信号。

## 调速 PWM 输出

| 功能 | STM32 引脚 | 定时器通道 | 说明 |
| --- | --- | --- | --- |
| 左轮调速 PWM | PD12 | TIM4_CH1 | 接左轮调速模块 PWM 输入 |
| 右轮调速 PWM | PD13 | TIM4_CH2 | 接右轮调速模块 PWM 输入 |

PWM 频率固定为 `1000Hz`。

占空比规则：

| 摇杆状态 | PWM 占空比 |
| --- | --- |
| 中位死区内 | 0%，方向全停 |
| 刚离开死区 | 10% |
| 摇杆推到底 | 90% |

## 遥控输入

| 功能 | 引脚 | 定时器 | 说明 |
| --- | --- | --- | --- |
| 转向 CH1 | PA0 | TIM2_CH1 | 左右转向，1000us 到 2000us，约 1500us 中位 |
| 油门 CH2 | PC6 | TIM8_CH1 | 前进后退，1000us 到 2000us，约 1500us 中位 |

程序启动后会自动标定 CH1/CH2 中位约 1.5 秒，中位附近 `±100us` 为死区。

## 方向输出

| 功能 | 当前代码引脚 | 说明 |
| --- | --- | --- |
| 左电机前进控制 | PA5 | 低电平有效 |
| 左电机后退控制 | PB1 | 低电平有效 |
| 右电机前进控制 | PB12 | 低电平有效 |
| 右电机后退控制 | PA6 | 低电平有效 |

停止时四个方向脚全部拉高，左右 PWM 占空比归零。

## 手动摇杆控制规则

| 摇杆位置 | 预期动作 |
| --- | --- |
| 上推 | 直线前进，推得越多越快 |
| 下拉 | 直线后退，拉得越多越快 |
| 左上角 | 向左前方弧线行驶 |
| 右上角 | 向右前方弧线行驶 |
| 左下角 | 向左后方弧线倒车 |
| 右下角 | 向右后方弧线倒车 |
| 左推到底 | 原地逆时针自转 |
| 右推到底 | 原地顺时针自转 |

当前混控算法：

```text
throttle = PC6 映射值
steering = PA0 映射值

如果 throttle 不为 0：
    turn_scaled = throttle * steering / 1000
    left_value = throttle + turn_scaled
    right_value = throttle - turn_scaled

如果 throttle 为 0：
    left_value = steering
    right_value = -steering
```

这样前进弧线和倒车弧线会随油门方向自动翻转，避免左下角、右下角与前进转弯逻辑相反。

## 独立测试环境

`platformio.ini` 中保留独立测试环境：

```ini
[env:dual_motor_pwm_1khz_30_test]
```

该环境只编译 `src/main_dual_motor_pwm_1khz_30_test.c`，不运行移苗主流程。

当前测试输出：

- PD12：1000Hz，30% 占空比。
- PD13：1000Hz，90% 占空比。

烧写命令：

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e dual_motor_pwm_1khz_30_test -t upload
```

主程序烧写命令：

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e manual_car_control -t upload
```

## 测试注意事项

- 测小车时先把轮子架空。
- `PC7` 保持手动模式，避免进入 12 棵苗自动流程。
- `PA8` 不能处于空闲低位，否则主循环会全停；切到中位或高位后，小车遥控才会更新。
- 绿色电平转换模块输入侧接 STM32 的 PWM、5V、GND；输出侧用 `OUT+ / OUT-` 接驱动器调速输入。
