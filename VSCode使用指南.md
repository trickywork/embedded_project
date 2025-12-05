# VSCode PlatformIO 使用指南

## 快速开始

### 1. 安装PlatformIO扩展

如果还没有安装：
1. 打开VSCode
2. 按 `Ctrl+Shift+X` (Windows/Linux) 或 `Cmd+Shift+X` (Mac) 打开扩展市场
3. 搜索 "PlatformIO IDE"
4. 点击安装

### 2. 打开项目

1. 在VSCode中打开项目文件夹：`/Users/junliu/Downloads/embedded`
2. VSCode会自动识别PlatformIO项目

## 电脑端测试（无需硬件）

### 步骤1：选择测试环境

1. 点击VSCode底部状态栏的 **环境选择器**（显示当前环境，如 `disco_l475vg_iot01a`）
2. 选择 `native` 环境

### 步骤2：构建测试程序

**方法1：使用按钮**
- 点击VSCode底部状态栏的 **✓ Build** 按钮
- 或按快捷键：`Ctrl+Alt+B` (Windows/Linux) 或 `Cmd+Alt+B` (Mac)

**方法2：使用命令面板**
- 按 `Ctrl+Shift+P` (Windows/Linux) 或 `Cmd+Shift+P` (Mac)
- 输入 "PlatformIO: Build"
- 选择并执行

### 步骤3：运行测试

**方法1：使用终端**
1. 在VSCode中打开终端（`` Ctrl+` `` 或 `Cmd+` `）
2. 运行：
   ```bash
   pio run -e native -t exec
   ```

**方法2：使用PlatformIO侧边栏**
1. 点击左侧PlatformIO图标（蚂蚁图标）
2. 展开 `PROJECT TASKS` → `native`
3. 点击 `Test` → `Run tests`

## STM32硬件构建（连接开发板后）

### 步骤1：选择硬件环境

1. 点击VSCode底部状态栏的环境选择器
2. 选择 `disco_l475vg_iot01a` 环境

### 步骤2：连接开发板

1. 用USB线连接STM32开发板到电脑
2. 等待系统识别设备

### 步骤3：构建和上传

1. **构建**：点击底部状态栏的 **✓ Build** 按钮
2. **上传**：点击底部状态栏的 **→ Upload** 按钮
3. **监控**：点击底部状态栏的 **🔌 Serial Monitor** 按钮查看输出

## 常用快捷键

| 操作 | Windows/Linux | Mac |
|------|--------------|-----|
| 构建 | `Ctrl+Alt+B` | `Cmd+Alt+B` |
| 上传 | `Ctrl+Alt+U` | `Cmd+Alt+U` |
| 串口监控 | `Ctrl+Alt+S` | `Cmd+Alt+S` |
| 命令面板 | `Ctrl+Shift+P` | `Cmd+Shift+P` |

## 环境说明

### `native` 环境
- **用途**：电脑端测试，无需硬件
- **平台**：native (本地编译)
- **源文件**：使用 `test/main_test.cpp` 作为主程序
- **特点**：快速测试算法，验证检测逻辑

### `disco_l475vg_iot01a` 环境
- **用途**：STM32硬件运行
- **平台**：ststm32
- **开发板**：STM32L475VG IoT Discovery
- **框架**：Mbed OS
- **源文件**：使用 `src/main.cpp` 作为主程序

## 常见问题

### Q: 构建失败怎么办？
A: 
1. 检查是否选择了正确的环境
2. 查看底部终端输出的错误信息
3. 确保PlatformIO扩展已完全加载（等待右下角加载完成）

### Q: 找不到环境选择器？
A: 
1. 确保已安装PlatformIO扩展
2. 确保在项目根目录（有`platformio.ini`的目录）
3. 重启VSCode

### Q: native环境构建失败？
A: 
1. 确保系统已安装C++编译器（GCC/G++）
2. Mac/Linux通常已自带
3. Windows需要安装MinGW或使用WSL

### Q: 如何查看构建输出？
A: 
- 构建时会在VSCode底部终端显示详细输出
- 可以点击终端标签查看完整日志

## 项目结构说明

```
embedded/
├── platformio.ini       # PlatformIO配置文件（定义了两个环境）
├── src/                 # 源代码（用于硬件）
│   ├── main.cpp        # 主程序
│   └── ...             # 其他源文件
└── test/               # 测试代码（用于电脑测试）
    └── main_test.cpp   # 测试主程序
```

## 推荐工作流程

1. **开发阶段**：使用 `native` 环境在电脑上测试算法
2. **验证阶段**：切换到 `disco_l475vg_iot01a` 环境，连接硬件测试
3. **调试阶段**：使用串口监控查看实时输出

