# Code Farm：智能果园 Qt/C++ 版

项目已从浏览器原型重构为 C++17 + Qt6 Widgets 桌面游戏。

## 功能

- Qt Widgets 前端，包含开始页、20 关关卡路线、任务简报和游戏主界面
- 自绘农场地图、无人机、成长进度、水分、施肥和 Bug 状态
- 逐关解锁、星级评分和 QSettings 本地保存
- Python 风格命令编辑器，支持语法高亮、运行、暂停、单步执行
- 20 个关卡，包含小麦、胡萝卜、番茄、玉米、向日葵和浆果等目标组合

## 构建运行

```bash
cmake -S . -B build
cmake --build build
./build/codefarm
```

## 支持命令

编辑器支持一行一个命令：

```python
move("right")
till()
plant("wheat")
water()
fertilize()
debug()
harvest()
get_current()
get_goals()
```

当前解释器按行解析函数调用，尚未接入完整 CPython 沙箱。后续如需完整 Python 语法，可在现有命令层之上接入 `pybind11` 或嵌入式 Python 白名单执行器。

## 设计资料

- `codefarm_levelmode_prompt.md`：原始完整规格
- `codefarm_game_design.md`：整理后的游戏设计文档
- `src/main.cpp`：Qt/C++ 实现
