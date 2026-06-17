# FinalAssignment

嵌入式期末作业：基于 MQTT 的 UE5 双人弹球游戏与 STM32 控制器联动。

## 项目简介

本项目将 STM32 嵌入式控制器、MQTT 通信和 Unreal Engine 5 游戏端结合，实现通过实体按键/控制器操作 UE5 双人 Pong 游戏的交互效果。

## 视频展示

[嵌入式期末作业视频展示](https://www.bilibili.com/video/BV18ULD6iEjP?vd_source=e03c2edd09090734ede0815cdc202303)

## 目录结构

- `01_UE_Project/`：UE5 工程源码
- `02_UE_Package/`：Windows 打包版本
- `03_STM32_Project/`：STM32 控制端工程

## 说明

仓库已启用 Git LFS，用于管理 UE 资源、打包文件和其他大文件。克隆仓库后如需完整拉取大文件，请先安装 Git LFS，然后执行：

```bash
git lfs pull
```
