# WaterSensorController

初始化稳定版本：水位与温度传感器控制器

## 📦 项目简介

本项目基于 8051 单片机，集成了电容式水位传感器与 DS18B20 温度传感器，支持通过 Modbus 协议进行数据通信。适用于太阳能热水器控制系统或其他液体监测场景。

## 🔧 功能特性

- 🌡️ 温度采集：支持 DS18B20 数字温度传感器
- 💧 水位检测：电容式探头 + 自定义映射函数
- 🔁 Modbus 通信：支持标准 Modbus RTU 协议
- 🧮 CRC 校验：内置 CRC16 校验模块
- 🧠 EEPROM 存储：支持参数持久化
- ⏱️ 精准延时：基于定时器的微秒级延时模块

## 📁 项目结构

├── src/                # 源代码 (.c)
├── inc/                # 头文件 (.h)
├── .vscode/            # VS Code 配置
├── .eide/              # EIDE 工程配置
├── .gitignore          # 忽略编译生成文件
├── .clang-format       # 代码格式规范
├── ModbusSlave_8G1K08A.code-workspace


## 🛠️ 编译环境

- **开发工具链**：SDCC 或 Keil C51
- **编辑器**：Visual Studio Code + EIDE 插件
- **目标平台**：8051 系列单片机（如 STC89C52RC）

## 🚀 快速开始

1. 克隆仓库：

   ```bash
   git clone https://github.com/dxwxws/WaterSensorController.git

📡 Modbus 通信说明
波特率：115200

数据位：8

停止位：1

校验：无

支持功能码：03（读保持寄存器）

📚 文档与校准
水位映射函数已在 cap.c 中定义

温度采集流程详见 18B20.c

CRC 校验逻辑在 crc16.c 中实现

EEPROM 参数存储在 eeprom.c 中管理

🧪 测试建议
使用串口调试助手或 Modbus 工具验证通信

使用标准电容或实测探头进行水位校准

对比理论值与实测值，调整映射表

📄 许可证
本项目采用 MIT 许可证，欢迎学习、修改。
