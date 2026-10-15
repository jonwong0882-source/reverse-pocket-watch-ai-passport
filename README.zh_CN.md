<p align="right"><strong>简体中文</strong> · <a href="README.md">English</a></p>

# 逆行怀表 · Retrograde Pocket Watch

一只向历史走去的古典怀表。现实经过一小时，怀表就回到一小时前；时针、分针和红色秒针一起逆行，下方日期也跟着跨日、跨月、跨年回退。

黄铜表壳、象牙色表盘与罗马刻度组成独立的怀表界面。秒针连续逆向滑动，适合把设备放在桌边，静静看时间退回。

## 下载与使用

在 [Releases](https://github.com/jonwong0882-source/reverse-pocket-watch-ai-passport/releases) 下载完整固件。刷入完整镜像可能清除已有设置，且会替换当前应用；不需要 Wi-Fi 或账户。

1. 开机后怀表立即倒走。初始显示 **UNSET**：编译日期只是参考，并非已校准的当前时间。
2. 长按确认键，进入起始时间设置；上下键调整数值，短按确认键依次选择年、月、日、时、分、秒。
3. 最后一次确认后，从所选日期时间开始等速倒走。设置时可长按确认键取消，原有倒行不会中断。
4. 表盘页面上下键可调节亮度，档位为 10/20/35/50/70/100%。

每次断电重启后都需重新校时；本离线版本无法获知断电期间经过的现实时间。界面使用英文和数字。

## 预览与验证

Release 封面由实际应用代码在主机上渲染，展示日期和电量为示例数据，**不是实机照片或截图**。

- Build：PASS，完整镜像和匹配调试文件已校验。
- Host tests：PASS，包括等速回退、闰年/世纪年、年份下限、按键设置流程、实际界面渲染及连续 1,000 帧。

## 构建

硬件为 FoloToy AI Passport，ESP-IDF 版本为 5.5.3。在已激活的开发环境执行：

```sh
./tools/validate.sh
```

完整镜像为 `build/FoloToy-AI-Passport-full.bin`（起始地址 `0x0`）。源码与操作详见 [开发说明](docs/reverse-pocket-watch.zh_CN.md)，主机渲染测试命令也在该说明中。

基于 [FoloToy AI Passport](https://gitee.com/FoloToy/ai-passport)，沿用原项目 MIT 许可，见 [LICENSE](LICENSE)。
