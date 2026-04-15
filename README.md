# 🧩 Genshin-LinkLink-SDL-v1 (原神连连看)

![C](https://img.shields.io/badge/Language-C-blue.svg)
![SDL2](https://img.shields.io/badge/Library-SDL2-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![Status](https://img.shields.io/badge/Status-Archived-purple.svg)

基于纯 **C 语言** 与 **SDL2** 图形库开发的原神主题“连连看”游戏。
> 这是我在 **2022年8月（初三暑假）** 编写的练手项目，现作为个人编程历程的存档开源展示。虽然代码全写在了一个 `main.c` 里，结构十分青涩，但它见证了一个少年最初探索图形化界面和算法的时光

---

## ✨ 核心功能与特性

- 🎮 **经典连连看算法**：完整实现了0折（直线）、1折（L型）、2折（U/Z型）的路径连通性判定。
- 🗺️ **边缘消除支持**：通过在 10x10 地图外围增加“辅助透明层”，完美支持外围方块的绕边消除。
- 🎵 **沉浸式视听体验**：
  - 使用 `SDL_image` 渲染原神角色头像与蒙德城背景。
  - 使用 `SDL_mixer` 加入了点击、消除、胜利音效，并支持多首《原神》原声音乐（HOYO-MiX）的自动轮播。
- ⏱️ **数据持久化**：内置计时器系统，并在通关后自动结算，记录并保存**本地最短通关时间**（基于本地文件读写）。
- 🚩 **可视化路径**：消除成功时，会在屏幕上实时绘制出消除的折线路径，视觉反馈拉满。

---

## 🕹️ 实机演示

### 游戏截图
<div align="center">
  <img width="400" alt="游戏界面1" src="https://github.com/user-attachments/assets/6c220584-84e3-4dea-a444-3dd301eb6462" />
  <img width="400" alt="游戏界面2" src="https://github.com/user-attachments/assets/9dafeb85-d512-4a23-a8b7-3bd4316953d9" />
</div>

### 游戏录屏
因当时录制原因，导致点击位置与实际显示位置不符，实机可正常游玩

https://github.com/user-attachments/assets/b3775450-a6a4-4dae-984a-4f4ac510eddf

---

## 🚀 游玩指南

无需配置开发环境，直接下载即可游玩：

1. 在仓库右侧点击 **Releases**，下载最新版本的压缩包。
2. 将压缩包解压到本地文件夹。
3. 双击运行 `.exe` 文件即可开始游戏。
4. **⚠️ 注意事项**：由于游戏包含本地记录功能，请确保 `assets/Data` 文件存在（如果没有，请在 `assets` 文件夹下手动新建一个名为 `Data` 的无后缀空文件），否则游戏会提示无法找到记录文件。
5. 游戏内按 `Esc` 键可随时退出。

---

## 💻 核心技术细节 (For Developers)

**图形与媒体库：**
* [SDL2](https://libsdl.org/) (核心窗口与渲染) - SDL2-2.32.10-VC
* [SDL2_image](https://github.com/libsdl-org/SDL_image) (PNG/JPG 图片加载) - SDL2_image-2.8.8-VC
* [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer) (音频播放, FLAC/WAV 支持) - SDL2_mixer-2.8.1-VC

**连连看算法简析：**
核心判定逻辑封装在 `clear()` 函数中，依次降级判断：
1. `straight()`: 检查同行/同列两点之间是否有障碍。
2. `turn_once()`: 寻找两个方块所在行列的交点，判断转角处是否为空，且两条线段是否 `straight`。
3. `turn_twice()`: 遍历地图（包括辅助边界区）寻找空节点作为首个拐点，再结合 `turn_once()` 与 `straight()` 判定。

---

## 🕰️ 历史归档证明

本项目的主体代码完成于 2022 年初三暑假。附上早期文件夹的创建日期记录（后因上传 GitHub 导致部分修改日期变更）：

<img width="500" alt="日期记录" src="https://github.com/user-attachments/assets/d27cf063-a033-44b5-97cb-f1c918fcd1b4" />

---
*If you like this small project, feel free to give it a ⭐ Star!*
