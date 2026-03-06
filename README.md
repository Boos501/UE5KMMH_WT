# UE5KMMH_WT

## UE快门问题 / UE Shutter Issue

本文档记录了虚幻引擎5（UE5）中常见的快门（Shutter）配置问题及其解决方案。  
This document covers common shutter configuration issues in Unreal Engine 5 (UE5) and their solutions.

---

### 问题描述 / Problem Description

在 UE5 中使用 **电影摄像机（Cine Camera Actor）** 或 **电影渲染队列（Movie Render Queue, MRQ）** 时，
快门角度（Shutter Angle）或快门速度设置不正确会导致以下问题：

When using **Cine Camera Actor** or **Movie Render Queue (MRQ)** in UE5,
incorrect shutter angle or shutter speed settings may cause:

- 运动模糊过强或过弱 / Motion blur too strong or too weak
- 画面曝光不准确 / Incorrect exposure
- 渲染帧与实时预览不一致 / Rendered frames inconsistent with real-time preview
- 高速运动物体出现拖影或锯齿 / Ghosting or aliasing on fast-moving objects

---

### 根本原因 / Root Cause

UE5 中的快门角度遵循电影行业的 **180度快门法则（180° Shutter Rule）**：

The shutter angle in UE5 follows the cinematic industry's **180° Shutter Rule**:

| 帧率 / Frame Rate | 推荐快门角度 / Recommended Shutter Angle | 等效快门速度 / Equivalent Shutter Speed |
|---|---|---|
| 24 fps | 180° | 1/48 s |
| 25 fps | 180° | 1/50 s |
| 30 fps | 180° | 1/60 s |
| 60 fps | 180° | 1/120 s |

若快门角度设置为 **0°** 或异常值，运动模糊将完全消失或产生错误效果。

If the shutter angle is set to **0°** or an abnormal value, motion blur will disappear entirely or produce incorrect results.

---

### 解决方案 / Solution

#### 1. 电影摄像机设置 / Cine Camera Actor Settings

在 UE5 编辑器中选中 **Cine Camera Actor**，在 **Details** 面板中：

Select **Cine Camera Actor** in the UE5 editor and in the **Details** panel:

1. 找到 **Current Camera Settings > Shutter Speed (1/s)**  
   Locate **Current Camera Settings > Shutter Speed (1/s)**
2. 根据目标帧率设置对应值（例如24fps → 48）  
   Set the value according to the target frame rate (e.g., 24fps → 48)
3. 确认 **Post Process > Motion Blur > Amount** 不为0  
   Ensure **Post Process > Motion Blur > Amount** is not 0

#### 2. 电影渲染队列设置 / Movie Render Queue (MRQ) Settings

在 **Movie Render Queue** 的输出设置中：

In the **Movie Render Queue** output settings:

1. 打开 **Anti-aliasing** 设置  
   Open **Anti-aliasing** settings
2. 将 **Shutter Timing** 设置为 `Frame Center`（推荐）  
   Set **Shutter Timing** to `Frame Center` (recommended)
3. 将 **Override Shutter Angle** 设置为 `180.0`（对应180度快门法则）  
   Set **Override Shutter Angle** to `180.0` (following the 180° shutter rule)
4. 根据需要调整 **Temporal Sample Count** 以平衡质量和性能  
   Adjust **Temporal Sample Count** as needed to balance quality and performance

#### 3. 项目设置 / Project Settings

确认以下项目设置正确 / Verify the following project settings are correct:

1. 打开 **Edit > Project Settings > Engine > Rendering**  
   Open **Edit > Project Settings > Engine > Rendering**
2. 确保 **Motion Blur** 已启用  
   Ensure **Motion Blur** is enabled
3. 检查 **Default Post Processing Settings** 中快门相关参数  
   Check shutter-related parameters in **Default Post Processing Settings**

---

### Blueprint / C++ 代码示例 / Code Example

通过蓝图或 C++ 动态设置快门速度 / Dynamically set shutter speed via Blueprint or C++:

**Blueprint（蓝图）:**
```
Get Player Camera Manager
→ Set Camera Post Process Blend Weight
→ Modify Post Process Settings: Shutter Speed
```

**C++:**
```cpp
// 获取 Cine Camera Component 并设置快门速度
// Get Cine Camera Component and set shutter speed
UCineCameraComponent* CineCamera = Cast<UCineCameraComponent>(CameraActor->GetCameraComponent());
if (CineCamera)
{
    // 设置快门速度为 1/48（对应24fps的180度快门）
    // Set shutter speed to 1/48 (180° shutter for 24fps)
    CineCamera->CurrentCameraSettings.ShutterSpeed = 48.0f;
}
```

---

### 参考资料 / References

- [UE5 官方文档 - Cine Camera Actor](https://dev.epicgames.com/documentation/en-us/unreal-engine/cinematic-cameras-in-unreal-engine)
- [UE5 官方文档 - Movie Render Queue](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-cinematics-in-unreal-engine)
- [快门角度详解 / Shutter Angle](https://en.wikipedia.org/wiki/Shutter_angle)