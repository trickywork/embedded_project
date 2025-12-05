# 帕金森症状检测系统

基于STM32和Mbed框架的嵌入式系统，用于检测帕金森病相关症状：
- **震颤 (Tremor)**: 3-5Hz频率范围的节律性振荡
- **运动障碍 (Dyskinesia)**: 5-7Hz频率范围的舞蹈样运动
- **冻结步态 (Freezing of Gait, FOG)**: 行走后突然的身体冻结

## 项目结构

```
embedded/
├── platformio.ini          # PlatformIO配置文件
├── src/
│   ├── main.cpp            # 主程序
│   ├── SensorManager.h/cpp # 传感器管理（支持模拟模式）
│   ├── SymptomDetector.h/cpp # 症状检测算法
│   ├── FFTProcessor.h/cpp    # FFT频率分析
│   └── BLEManager.h/cpp      # BLE通信管理
├── test/
│   └── test_simulator.cpp    # 电脑端测试程序
└── README.md
```

## 功能特性

1. **传感器数据采集**: 52Hz采样率，3秒数据窗口（156个样本）
2. **FFT频率分析**: 检测特定频率范围内的能量
3. **症状检测算法**:
   - 震颤检测：3-5Hz频率分析
   - 运动障碍检测：5-7Hz频率分析
   - 冻结步态检测：步态分析 + 运动突然停止检测
4. **BLE通信**: 三个BLE特征值传输检测结果
5. **模拟模式**: 支持电脑端测试，无需硬件

## 编译和运行

### 在电脑上测试（模拟模式）

```bash
# 编译测试程序
pio run -e native

# 运行测试
pio run -e native -t exec
```

### 在STM32开发板上运行

```bash
# 编译
pio run -e disco_l475vg_iot01a

# 上传到开发板
pio run -e disco_l475vg_iot01a -t upload

# 监控串口输出
pio device monitor
```

## 硬件要求

- STM32L475VG IoT Discovery开发板
- 内置LSM6DSL加速度计/陀螺仪传感器
- 内置BLE模块

## 算法说明

### 震颤检测
- 使用FFT分析加速度数据
- 检测3-5Hz频率范围内的能量
- 阈值：强度 > 0.3

### 运动障碍检测
- 使用FFT分析加速度数据
- 检测5-7Hz频率范围内的能量
- 阈值：强度 > 0.3

### 冻结步态检测
1. 步态分析：检测步数和步频
2. 运动方差分析：计算加速度和角速度的方差
3. 冻结判断：之前有步态活动 + 当前运动方差很小

## 待完成的工作

1. **硬件集成**: 实现STM32 L475VG IoT Discovery板的传感器读取
2. **BLE实现**: 完成实际的BLE服务和特征值配置
3. **参数调优**: 根据实际测试调整检测阈值
4. **用户界面**: 添加LED指示和按钮控制

## 测试

运行测试程序验证检测算法：

```bash
cd test
pio run -e native -t exec
```

测试程序会生成模拟的传感器数据，验证三种症状的检测功能。

