# QTCopyHistory

Qt 6 剪贴板历史。当前桌面集成针对 Ubuntu GNOME 50（包括 Wayland）。

## 构建与运行

```sh
cmake -S . -B build
cmake --build build -j4
ctest --test-dir build --output-on-failure
./build/QTCopyHistory
```

## 安装 GNOME 桌面桥接

Wayland 下全局快捷键、跨应用恢复焦点/模拟粘贴和绝对窗口定位由配套 GNOME Shell 扩展完成。仅运行 Qt 程序不会启用这些功能。

```sh
gnome-extensions pack gnome-extension --force --out-dir build
gnome-extensions install --force build/qtcopyhistory@local.shell-extension.zip
```

首次安装后注销并重新登录，再执行：

```sh
gnome-extensions enable qtcopyhistory@local
```

启动新版 `build/QTCopyHistory`，在编辑器输入框按 Win+V，使用方向键选择，Enter 或单击粘贴回唤起前的输入位置。Escape 关闭，Delete 删除记录。弹窗在鼠标指针旁显示，空间不足时翻转并限制在当前显示器工作区内（不包含任务栏）。这里的定位是鼠标指针，不是跨应用文本插入光标。

扩展启用时会释放 GNOME 原来的 Super+V 通知快捷键，禁用时恢复。没有恢复目标窗口或用户仍按住修饰键时，不会向其他窗口盲目发送粘贴。自动粘贴使用 Ctrl+V，目标应用必须支持该快捷键；终端等使用 Ctrl+Shift+V 的应用不在此范围内。

托盘和 `--show` 可打开列表并复制记录；自动粘贴目标仅由 Win+V 捕获。新版 AppImage 位于 `dist/QTCopyHistory-x86_64.AppImage`；项目根目录的同名文件是旧版打包种子。

## 手动验收

- 在两个不同编辑器分别用 Win+V 唤起，单击或 Enter 后文本应进入原来的输入位置。
- 在屏幕四角、多显示器和不同缩放下检查弹窗完整显示。
- 按住 Win 后选择记录，松开修饰键后才应粘贴。
- 删除全部记录后，方向键/Enter 不崩溃，Escape 不粘贴。
- 禁用扩展后，原 GNOME 通知快捷键恢复。

## 新版 AppImage

```sh
chmod +x dist/QTCopyHistory-x86_64.AppImage
./dist/QTCopyHistory-x86_64.AppImage
```

没有 FUSE 时可使用 `./dist/QTCopyHistory-x86_64.AppImage --appimage-extract-and-run`。
程序包含 Qt 6.11.0 和 Wayland 插件；GNOME 扩展仍需按上述步骤单独安装。
当前构建为 Linux x86_64，包内 ELF 符号要求 glibc 至少 2.34，仍依赖系统图形库；尚未在其他发行版验收，不能仅凭 glibc 版本保证兼容。

重新打包（需要 CMake、C++ 编译器、Qt SDK、mksquashfs 和原始 AppImage）：

```sh
QT_PREFIX=/path/to/Qt/6.11.0/gcc_64 ./scripts/build-appimage.sh
```

脚本复用原包运行时和 Qt 库，检查 Qt 库与 SDK 完全一致后替换 Release 程序；更换 Qt 版本时需要重新部署依赖。输出校验和位于 `dist/SHA256SUMS`。
