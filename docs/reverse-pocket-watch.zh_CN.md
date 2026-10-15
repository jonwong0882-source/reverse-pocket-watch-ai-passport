<p align="right"><strong>简体中文</strong> · <a href="reverse-pocket-watch.md">English</a></p>

# 逆行怀表

这是适配 AI Passport 240 × 320 屏幕的离线应用。独立怀表界面采用黄铜表壳、
象牙色表盘、原创矢量罗马数字（四点采用传统 IIII）、深色时分针和红色连续逆行秒针。
下方独立区域显示怀表的公历日期与 24 小时时间，右上角显示电量，读取不可用时显示 `--`。
启动和操作路径均不进入原有演示菜单。

计时公式为“起始时间戳 − 单调时钟经过的微秒数”。现实经过一小时，怀表回到一小时前。
时针、分针、秒针均同步逆行；每 40 毫秒更新，目标每秒 25 帧，不通过累减回调次数计时，
因此掉帧不会累计走时误差。日期支持逆向跨日、跨月、跨年、闰日和 1970 年以前的时间。
公历下限为公元 0001 年，到达下限后停止回退，避免溢出。

## 起始时间与操作

设备没有独立的电池供电实时时钟。本离线版本每次开机立即倒走，但初始日期明确标记
**UNSET（未校时）**：编译时间按 UTC+8 显示，仅作为设置参考，不能当作已经校准的现实时间。
每次开机后需设置当下日期时间。无需 Wi-Fi、云服务、账户凭据，也不写入 Flash。
设置不跨断电保存，因为保存的时间戳无法计算断电期间经过的时间。

- 长按确认键打开起始时间设置，上下键调整当前字段。
- 短按确认键依次选择年、月、日、时、分、秒；最后一次确认以所选时间开始新的倒行时间线。
- 设置中长按确认键取消。编辑期间原有时间线持续倒走。
- 表盘中上下键调节亮度，档位为 10/20/35/50/70/100%。

界面使用英文标签及数字日期；罗马数字直接由原创矢量笔画绘制，不依赖外部字体，
也不假定默认字体具有中文字形。

## 验证与交付

使用 ESP-IDF 5.5.3 执行 `./tools/validate.sh`。新增主机测试覆盖精确 1:1 回退、亚秒运动、
公历闰年和世纪年、年份下限及日期编辑。沿用默认 NVS/PHY/factory 分区。
验证通过的完整镜像为 `build/FoloToy-AI-Passport-full.bin`；对应 ELF、MAP 和清单位于
`build/firmware/` 下以完整镜像 SHA-256 命名的目录。完整镜像从地址 0 写入可能重置 NVS。
构建成功不表示已获授权刷机。

已具备 managed components 时，可运行实际 LVGL 主机渲染及按键流程测试：

```sh
cmake -S tests/watch_preview -B build/watch-preview
cmake --build build/watch-preview -j 8
cd build/watch-preview
./watch_preview
```

程序直接编译设备使用的 `main/watch_main.c`，采用 40 KiB LVGL 内存池，输出未校时、表盘、
设置页面及回退后的 PPM 图像，并验证设置、取消、开始及一秒和一小时的倒行。
这属于主机渲染，不能证明真实屏幕效果、设备帧率、电量读数或实体按键行为；这些仍需授权刷机后验证。

## 已验证构建

Build：PASS。Host tests：PASS，包括实际 LVGL 渲染及连续 1,000 帧。Device tests：NOT RUN。Unverified：实体屏幕效果、帧率、电量和按键。交付时未检测到 USB modem 串口。

Full image SHA-256: `da8685180921606e0c27761ddce9e5e5c046dbd1bba9fda14b6b1da01bab0042`

Matching ELF SHA-256: `f1fea5ca2c17f43663b396938116af50cef3b747cf514e0295c7dfb7442e9e7e`

Archive: `build/firmware/da8685180921606e0c27761ddce9e5e5c046dbd1bba9fda14b6b1da01bab0042`
