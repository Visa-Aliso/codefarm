import QtQuick
import QtQuick.Controls
import CodeFarm

Rectangle {
    id: root
    width: parent ? parent.width : 0
    height: parent ? parent.height : 0
    color: Theme.bgMain

    // Top bar
    Rectangle {
        id: topBar
        width: parent.width
        height: 48
        color: Qt.rgba(0,0,0,0.2)
        z: 10

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "←  返回"
            color: backMa.containsMouse ? Theme.textLight : Theme.textDim
            font.family: Theme.fontUI
            font.pixelSize: 14
            MouseArea { id: backMa; anchors.fill: parent; anchors.margins: -8; hoverEnabled: true; onClicked: navigator.pop() }
        }

        Text {
            anchors.centerIn: parent
            text: "游戏帮助"
            color: Theme.textLight
            font.family: Theme.fontUI
            font.pixelSize: 18
            font.weight: Font.Bold
        }
    }

    // Scrollable content
    Flickable {
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width - 80, 700)
        contentHeight: helpContent.height + 60
        clip: true
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: helpContent
            width: parent.width
            topPadding: 30
            spacing: 24

            // Section 1: Game Overview
            HelpSection {
                title: "游戏简介"
                content: "Code Farm 是一款编程教育游戏。你将通过编写 Python 代码控制一架农业无人机，在农场上完成耕地、种植、浇水、施肥、除虫和收获等任务。\n\n游戏共 20 个关卡，分为 8 个教学阶段，逐步引入编程概念和农场机制。\n\n星级规则：\n  ★   收获所有指定作物（必须全部完成才能通关）\n  ★★  在规定时间内完成\n  ★★★ 在更短时间内完成\n\n提示：收获数量通常等于或超过田地大小，你可能需要收割后重新种植！"
            }

            // Section 2: How to Play
            HelpSection {
                title: "如何游玩"
                content: "1. 从关卡选择界面选择一个已解锁的关卡\n2. 在代码编辑器中编写 Python 脚本\n3. 点击绿色按钮（▶）运行代码\n4. 观察无人机在农场上执行你的指令\n5. 完成所有 ★ 目标即可通关\n\n基本耕种流程：\n  till() → plant(\"wheat\") → water() → wait... → harvest()\n\n重要提示：\n  • ★ 目标要求收获指定数量的每种作物，必须全部完成\n  • 收获总数通常 ≥ 田地格子数，你可能需要收割后重新种植！\n  • 用 get_current() 检查格子状态，决定是种植还是收割\n  • 代码每个 tick 执行一个动作，用循环简化重复操作\n\n作物特性速览：\n  小麦 — 最基础，只需浇水\n  胡萝卜 — 需多浇水，但水>0.85会过涝受伤\n  番茄 — 初始水分少，需更早补浇\n  玉米 — 生长慢，缺水会损失进度\n  向日葵 — 不可收割，为周围8格提供阳光加成\n\n注意：灰色岩石格子不可移动、种植或浇水。"
            }

            // Section 3: Code Editor
            HelpSection {
                title: "代码编辑器"
                content: "游戏界面中的深色浮动窗口就是代码编辑器。\n\n标题栏左侧按钮：\n  ▶ — 运行代码（空闲时显示）\n  ⏸ — 暂停执行（运行时显示）\n  ■ — 停止并重置关卡（运行时显示）\n\n标题栏右侧按钮：\n  ─ — 最小化/还原编辑器\n\n状态栏（底部）显示当前状态：\n  Idle（空闲）、Running（运行中）、Paused（暂停）、Error（错误）\n\n编辑器支持 Python 语法高亮，当前执行行会以绿色标记。\n游戏界面右上角 ? 按钮可打开关卡提示窗口，包含参考答案。"
            }

            // Section 4: Teaching Roadmap
            HelpSection {
                title: "教学路线图"
                content: "阶段 1（关1-2）：入门耕种\n  1×1 → 2×2，学习 till→plant→water→harvest\n  二维移动、for 循环和 if 条件\n\n阶段 2（关3-5）：基础循环\n  3×3 → 4×4，岩石障碍物绕行\n  for 循环批量耕种、def 函数封装\n  get_current() 查询、变量和条件判断\n  胡萝卜过涝机制\n\n阶段 3（关6-8）：进阶管理\n  4×4，施肥加速 + 番茄二次浇水\n  fertilize()、多作物管理\n  while 循环、嵌套循环优化\n\n阶段 4（关9-11）：虫害管理\n  4×4 → 5×5，虫害区概念\n  spray() 喷药除虫、玉米旱灾机制\n  持续巡逻模式\n\n阶段 5（关12-14）：高级逻辑\n  5×5 → 6×6，布尔运算 (and/or/not)\n  break / continue 优化巡逻\n  return、get_goals()、路径规划\n\n阶段 6（关15-17）：向日葵\n  6×6，向日葵阳光加成\n  不可收割作物、过涝致命\n  棋盘格布局优化、效率挑战\n\n阶段 7（关18-19）：综合应用\n  7×7，大规模多作物管理\n  极限效率限时挑战\n\n阶段 8（关20）：终极挑战\n  8×8 终极农场，全机制综合"
            }

            // Section 5: Basic Commands
            HelpSection {
                title: "基础指令（动作）"
                content: "till() — 犁地\n  将当前空地开垦为可种植状态。\n  如果当前格不是空地或是岩石，操作失败。\n  解锁: 关卡 1\n\nplant(作物) — 种植\n  在已犁好的地上种下作物。\n  可选作物: \"wheat\" / \"carrot\" / \"tomato\" / \"corn\" / \"sunflower\"\n  向日葵不可收割，但会持续为周围8格提供阳光加成。\n  如果当前格未犁地或已有作物，操作失败。\n  解锁: 关卡 1\n\nwater() — 浇水\n  给当前格浇水，水分 +0.35（最高 1.0）。\n  作物需要水分才能生长，可多次浇水叠加。\n  注意：胡萝卜和向日葵浇水过多（>0.85）会过涝受伤！\n  如果当前格没有作物或是岩石，操作失败。\n  解锁: 关卡 1\n\nwait() — 等待\n  等待一个 tick（约 0.5 秒），不做任何操作。\n  用于等待作物成熟。\n  解锁: 关卡 1\n\nharvest() — 收割\n  收割当前格已成熟的作物。\n  作物进度达到 100% 后变为 Mature 状态才可收割。\n  向日葵不可收割（它会持续提供阳光加成）。\n  如果作物未成熟、有虫害或不可收割，操作失败。\n  解锁: 关卡 1\n\nmove(方向) — 移动\n  移动无人机一格。\n  方向: \"up\"(上) / \"down\"(下) / \"left\"(左) / \"right\"(右)\n  不能移出地图边界，不能移动到岩石格子。\n  解锁: 关卡 2\n\nfertilize() — 施肥\n  给当前格施肥，加速作物生长。\n  水分 ≥ 0.6 时加速 1.8 倍，否则加速 1.5 倍。\n  效果持续 12 tick，之后失效。\n  解锁: 关卡 6\n\nspray() — 喷药\n  清除当前格的虫害，并获得 12 tick 的虫害免疫。\n  如果当前格没有虫害，仍然消耗一个 tick 但无实际效果。\n  解锁: 关卡 9"
            }

            // Section: Crop Info
            HelpSection {
                title: "作物信息"
                content: "小麦 (wheat)\n  成熟时间: 约 3 秒 (6 tick)\n  水分阈值: 0.0（最宽容，只需浇水即可）\n  特点: 最快成熟的作物，适合入门练习\n  解锁: 关卡 1\n\n胡萝卜 (carrot)\n  成熟时间: 约 7 秒 (14 tick)\n  水分阈值: 0.4（需要较多浇水，建议 water() 两次）\n  特点: 过涝惩罚！水分 > 0.85 时每 tick 损失 0.03 进度\n  解锁: 关卡 4\n\n番茄 (tomato)\n  成熟时间: 约 6 秒 (12 tick)\n  水分阈值: 0.25\n  特点: 初始水分只有 0.4（非 0.5），需要更早补浇\n  解锁: 关卡 7\n\n玉米 (corn)\n  成熟时间: 约 10 秒 (20 tick)\n  水分阈值: 0.2\n  特点: 旱灾惩罚！水分 = 0 时每 tick 损失 0.02 进度\n        生长慢且容易生虫，需要持续供水和巡逻\n  解锁: 关卡 10\n\n向日葵 (sunflower)\n  成熟时间: 约 17.5 秒 (35 tick)\n  水分阈值: 0.3\n  特点: 不可收割！成熟后持续为周围 8 格提供阳光加成\n        (+0.15 生长速度/tick)\n        过涝致命：水分 > 0.85 时每 tick 损失 0.05 进度\n        缺水也致命：水分 = 0 时每 tick 损失 0.05 进度\n  解锁: 关卡 15\n\n水分机制:\n  每 tick 水分自然蒸发 0.01\n  浇水一次 +0.35，最高 1.0\n  水分 ≥ 阈值: 全速生长\n  水分 > 0 但 < 阈值: 半速生长\n  水分 = 0: 停止生长（敏感作物还会损失进度）\n\n施肥效果:\n  水分 ≥ 0.6: 生长加速 1.8 倍\n  水分 < 0.6: 生长加速 1.5 倍\n  持续 12 tick 后失效\n\n虫害机制:\n  每 tick 有概率随机出现在有作物的格子\n  虫害区（紫色格子）虫子生成概率翻倍\n  虫害会使水分蒸发加速 3 倍，并回退生长\n  用 spray() 清除虫害并获得 12 tick 免疫\n\n岩石障碍:\n  灰色岩石格子不可移动、种植、浇水或喷药\n  需要规划路径绕开岩石"
            }

            // Section: Available Syntax
            HelpSection {
                title: "可用语法"
                content: "你可以在代码编辑器中使用以下 Python 语法：\n\n从关卡 1 起可用：\n  表达式和函数调用: harvest(), move(\"right\")\n  for 循环: for i in range(5):\n\n从关卡 2 起新增：\n  if 条件: if i < 3:\n\n从关卡 3 起新增：\n  变量赋值: pos = get_pos()\n  while 循环: while get_pos()[0] > 0:\n\n从关卡 5 起新增：\n  def 函数定义: def plant_row():\n\n从关卡 12 起新增：\n  布尔运算: and, or, not\n  else 分支: if ... else ...\n\n从关卡 13 起新增：\n  break: 提前退出循环\n  continue: 跳过本次迭代\n\n从关卡 14 起新增：\n  return: 从函数返回值\n\n注意：import 语句被禁用，只可使用游戏提供的内置函数。"
            }

            // Section 6: Query Commands
            HelpSection {
                title: "查询指令（免费）"
                content: "get_pos() — 获取坐标\n  返回元组 (x, y)，表示无人机当前位置。\n  x 为列号（从左到右），y 为行号（从上到下）。\n  用法: pos = get_pos(); x = pos[0]\n  解锁: 关卡 2\n\nget_map_size() — 获取地图尺寸\n  返回元组 (w, h)，w 为宽度，h 为高度。\n  用法: size = get_map_size(); w = size[0]\n  解锁: 关卡 2\n\nget_current() — 获取当前格信息\n  返回字典，包含以下字段:\n    state: \"empty\" / \"tilled\" / \"planted\" / \"mature\" / \"rock\"\n    crop: \"wheat\" / \"carrot\" / \"tomato\" / \"corn\" / \"sunflower\" / \"\"\n    water: 当前水分值 (0.0 ~ 1.0)\n    hasBug: True / False\n    progress: 生长进度 (0.0 ~ 1.0)\n    fertilized: True / False\n  用法: cell = get_current(); if cell[\"hasBug\"]: spray()\n  解锁: 关卡 3\n\ndebug() — 调试信息\n  在控制台打印当前格的详细信息，同时返回格子信息字典。\n  用于调试脚本逻辑。\n  解锁: 关卡 9\n\nget_tick() — 获取 tick 数\n  返回当前已执行的 tick 数（整数）。\n  解锁: 关卡 13\n\nget_goals() — 获取目标列表\n  返回关卡目标列表，每个目标包含:\n    description: 目标描述\n    current: 当前进度\n    target: 目标值\n    completed: 是否完成\n    starTier: 星级 (1/2/3)\n  解锁: 关卡 14"
            }

            // Section 7: Programming Concepts
            HelpSection {
                title: "编程概念"
                content: "for 循环（关卡 1 起）：\n  for i in range(5):\n      harvest()\n      move(\"right\")\n  重复执行固定次数\n\nif 条件（关卡 2 起）：\n  if cell[\"crop\"] == \"wheat\":\n      harvest()\n  根据条件选择不同操作\n\n变量和 while 循环（关卡 3 起）：\n  pos = get_pos()\n  cell = get_current()\n  while get_pos()[0] > 0:\n      move(\"left\")\n  用变量保存数据，while 重复直到条件不满足\n\ndef 函数（关卡 5 起）：\n  def plant_row(crop):\n      for i in range(5):\n          till()\n          plant(crop)\n  封装可复用的逻辑\n\n布尔逻辑和 else（关卡 12 起）：\n  and（与）、or（或）、not（非）\n  if ... else ... 分支\n  组合复杂条件判断\n\nbreak / continue（关卡 13 起）：\n  break — 提前退出循环\n  continue — 跳过本次迭代\n\nreturn（关卡 14 起）：\n  从函数返回值"
            }

            // Section 8: Game Mechanics
            HelpSection {
                title: "星级评价"
                content: "★ 完成所有指定作物的收获目标（必须全部完成才能通关）\n  每种作物都有独立的数量要求\n  例如：收获小麦×4、胡萝卜×4\n  收获总数通常等于或超过田地可用格子数\n\n★★ 在规定时间内完成（宽松时限）\n  给玩家留有余地的时间目标\n\n★★★ 在更短时间内完成（严格时限）\n  需要高效代码才能达成\n\n时间计算：\n  从进入关卡开始计时，包括思考、打字、代码运行的全部时间。\n  暂停期间时间继续计算。\n  详细时间要求请查看每关的目标面板。\n\n提示：如果收获总数 > 田地格子数，你需要：\n  1. 种满所有格子\n  2. 等待成熟并收割\n  3. 在收割后的空地上再次种植\n  4. 重复直到达到目标数量"
            }

            Item { width: 1; height: 30 }
        }
    }

    // Inline component for help sections
    component HelpSection: Column {
        property string title: ""
        property string content: ""
        width: parent.width
        spacing: 8

        Text {
            text: title
            color: Theme.btnGreen
            font.family: Theme.fontUI
            font.pixelSize: 16
            font.weight: Font.Bold
        }
        Rectangle {
            width: parent.width; height: contentText.height + 20
            radius: 8; color: Qt.rgba(0,0,0,0.3)
            Text {
                id: contentText
                anchors.left: parent.left; anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 14
                text: content
                color: Theme.textDim
                font.family: Theme.fontCode
                font.pixelSize: 13
                lineHeight: 1.5
                wrapMode: Text.WordWrap
            }
        }
    }
}
