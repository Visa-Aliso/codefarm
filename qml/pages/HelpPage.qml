import QtQuick
import QtQuick.Controls
import CodeFarm

Rectangle {
    id: root
    width: parent ? parent.width : 0
    height: parent ? parent.height : 0
    color: Theme.bgMain

    // Marker so the global "?" button can hide itself on this page
    property bool isHelpPage: true

    // Back button (matches LevelSelect style)
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 12
        anchors.leftMargin: 16
        z: 100
        width: 32; height: 32; radius: 6
        color: backMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
        Text { anchors.centerIn: parent; text: "←"; color: "white"; font.pixelSize: 16; font.weight: Font.Bold }
        MouseArea { id: backMa; anchors.fill: parent; hoverEnabled: true; onClicked: navigator.pop() }
    }

    // Top bar (keeps geometry for Flickable anchoring + centered title)
    Item {
        id: topBar
        width: parent.width
        height: 56
        z: 10

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
                title: "1. 游戏概述"
                content: "Code Farm 是一款编程教育游戏。你将通过编写 Python 代码控制一架农业无人机，在农场上完成耕地、种植、浇水、施肥、除虫和收获等任务。\n\n游戏共 20 个关卡，分为 7 个教学阶段，逐步引入从变量赋值到生成器表达式的完整 Python 基础语法。\n\n每关只能使用当前已解锁的函数、语法和内置函数，未解锁的会报错。通关后自动解锁下一关。"
            }

            // Section 2: Editor & Running
            HelpSection {
                title: "2. 编辑器与运行"
                content: "游戏界面中的深色浮动窗口就是代码编辑器。\n\n标题栏按钮：\n  ▶ — 运行代码（空闲时显示）\n  ⏸ — 暂停执行（运行时显示）\n  ■ — 停止并重置\n  ↺ — 载入本关教程代码\n  ─ — 最小化/还原\n\n状态栏（底部）显示最近一条消息。将鼠标悬停在状态栏上可弹出完整控制台历史窗口，查看所有 print 输出和 API 调用记录。\n\n编辑器支持 Python 语法高亮：关键字蓝色、API 函数名绿色（未解锁的显示灰色删除线）、字符串金色、注释灰色斜体。"
            }

            // Section 3: Crop Guide
            HelpSection {
                title: "3. 作物图鉴"
                content: "小麦 wheat — 6 tick 成熟，水分阈值 0.0（最宽容）。解锁: 关卡1\n\n胡萝卜 carrot — 14 tick，水分阈值 0.4，需浇两次水。过涝敏感：水>0.85 损失进度。解锁: 关卡5\n\n番茄 tomato — 12 tick，水分阈值 0.25，初始水分少需早补水。解锁: 关卡7\n\n玉米 corn — 20 tick，水分阈值 0.2。旱灾敏感：水=0 损失进度，需持续供水。解锁: 关卡9\n\n向日葵 sunflower — 35 tick，不可收割！成熟后持续为周围8格提供 +0.15/tick 阳光加成。过涝且缺水都致命。解锁: 关卡12\n\n水分机制：每 tick 蒸发 0.01，浇水 +0.35（最高1.0）。水≥阈值全速生长，水>0但<阈值半速，水=0 停止生长。\n\n施肥效果：水≥0.6 时 1.8 倍，否则 1.5 倍，持续 12 tick。"
            }

            // Section 4: Terrain & Mechanics
            HelpSection {
                title: "4. 地形与机制"
                content: "岩石（灰色）— 不可进入、不可犁地/种植/浇水/喷药。无人机移动到岩石会失败。规划路径时需绕开。\n\n虫害区（紫色标记）— 虫子生成概率翻倍。需要更频繁的 spray() 巡逻。\n\n虫害机制：每 tick 有概率随机出现在有作物的格子。虫害加速水分蒸发3倍，并回退生长进度。用 spray() 清除并获得12 tick免疫。\n\n3 星维度判定：\n  ★1 — 完成所有必需收获目标（通关条件）\n  ★2 — 通关时 tick 数 ≤ 效率阈值\n  ★3 — tick 数 ≤ 更严格阈值，且代码中实际使用了当关教学的新特性\n  巩固关（无新特性）★3 退化为纯 tick 门槛\n  maxTimeSec 为硬失败时限（墙上时间），与星级解耦"
            }

            // Section 5: Action Functions
            HelpSection {
                title: "5. 动作函数手册"
                content: "每个动作函数消耗 1 tick，返回 True/False。\n\ntill() — 犁地。空地→已犁。解锁: 关卡1\nplant(作物) — 种植。已犁→已种。解锁: 关卡1\nwater() — 浇水 +0.35。解锁: 关卡1\nwait() — 等待 1 tick（不操作）。解锁: 关卡1\nharvest() — 收割成熟作物（无虫害）。解锁: 关卡1\nmove(方向) — 移动一格。方向: \"up\"/\"down\"/\"left\"/\"right\"。解锁: 关卡2\nfertilize() — 施肥加速。解锁: 关卡7\nspray() — 除虫+12tick免疫。解锁: 关卡8"
            }

            // Section 6: Query Functions
            HelpSection {
                title: "6. 查询函数手册"
                content: "查询函数不消耗 tick，立即返回数据。\n\nget_pos() — 返回元组 (x, y)。x=列, y=行。解锁: 关卡2\nget_map_size() — 返回元组 (w, h)。解锁: 关卡3\nget_current() — 返回字典: state/crop/water/hasBug/progress/fertilized。解锁: 关卡4\ndebug() — 打印当前格信息到控制台，同时返回字典。解锁: 关卡6\nget_tick() — 返回当前 tick 数（整数）。解锁: 关卡10\nget_goals() — 返回目标列表，每项含 description/current/target/completed。解锁: 关卡14"
            }

            // Section 7: Syntax Unlock Roadmap
            HelpSection {
                title: "7. Python 语法解锁路线"
                content: "关卡1: 顺序调用、注释\n关卡2: 变量赋值 assign, if 条件\n关卡3: for + range 循环\n关卡4: while 循环\n关卡5: def 函数定义、参数\n关卡6: return 返回值, elif/else, not 取反\n关卡7: and / or 布尔组合\n关卡8: break / continue 循环控制\n关卡10: 元组解包 a, b = ...\n关卡11: 列表字面量 [a, b, c], 切片\n关卡12: 增强赋值 += -=, min/max/sum\n关卡13: 字典字面量 {k: v}, items/keys/values\n关卡14: 列表推导式 [x for x in xs if cond]\n关卡15: enumerate, zip, sorted\n关卡16: all/any, round/abs, 嵌套函数\n关卡17: lambda, 默认参数, 闭包\n关卡18: 字典推导式 {k: v for ...}\n关卡19: 生成器表达式 (x for x in xs), 集合推导式 {x for x in xs}\n关卡20: 全语法综合\n\n注意：import 语句被禁用。每关未解锁的语法会在运行前 AST 检查时报错。"
            }

            // Section 8: Builtins Quick Reference
            HelpSection {
                title: "8. 内置函数速查"
                content: "原子层（永远可用）：\n  print(x) — 输出到控制台（鼠标悬停状态栏查看完整历史）\n  int(x), float(x), str(x), bool(x) — 类型转换\n\n教学层（逐关解锁）：\n  range(n) — 生成 0..n-1 序列。解锁: 关卡3\n  tuple() — 元组构造器。解锁: 关卡10\n  len(x) — 长度。解锁: 关卡11\n  enumerate(xs) — 带下标遍历。解锁: 关卡11\n  min(xs), max(xs) — 最值。解锁: 关卡12\n  sum(xs) — 求和。解锁: 关卡12\n  dict(), list() — 构造器。解锁: 关卡13\n  sorted(xs) — 排序。解锁: 关卡15\n  zip(a, b) — 配对。解锁: 关卡15\n  all(xs), any(xs) — 全真/一真。解锁: 关卡16\n  round(x, n), abs(x) — 四舍五入/绝对值。解锁: 关卡16\n\n未解锁的内置函数调用时会抛出 \"当前关卡不允许使用 X 函数\" 错误。"
            }

            // Section 9: Common Errors
            HelpSection {
                title: "9. 常见错误与调试"
                content: "缩进错误 (IndentationError) — Python 用缩进表示代码块，请用4个空格，不要混用 Tab。\n\n未解锁语法 — \"当前关卡不允许使用 for 循环\" 等。查看帮助第7节确认该关解锁了哪些语法。\n\n未解锁函数 — \"当前关卡不允许调用 spray()\" 等。查看关卡提示窗口确认可用函数。\n\n未解锁内置函数 — \"当前关卡不允许使用 len 函数\" 等。查看帮助第8节确认解锁顺序。\n\n作物未成熟 — harvest() 返回 False。用 get_current()[\"state\"] 检查是否为 \"mature\"，或用 debug() 查看进度。\n\n过涝 — 胡萝卜/向日葵水>0.85 损失进度。减少浇水次数。\n\n旱灾 — 玉米/向日葵水=0 损失进度。增加巡逻补水频率。\n\n移动失败 — 可能是岩石阻挡或地图边界。用 get_pos() 确认位置。"
            }

            // Section 10: Stars & Progress
            HelpSection {
                title: "10. 星级与进度"
                content: "★1 通关 — 完成所有必需收获目标，解锁下一关。\n★2 效率 — 通关时 tick 数 ≤ 阈值。tick 是确定性计数器（每次动作+1），只取决于代码算法，不受电脑速度或手速影响。\n★3 学以致用 — tick 数 ≤ 更严格阈值，且代码中实际使用了当关教学的新语法或新函数。这确保你真的学会了新概念，而不只是用旧知识暴力通关。\n\n巩固关（如关卡9）无新特性，★3 退化为纯 tick 门槛。\n\nmaxTimeSec 为硬失败时限——墙上时间耗尽直接失败，与星级无关。\n\n进度自动保存到 ~/.codefarm/save.json。可在设置中重置全部进度。\n\n每关用户脚本也自动保存，切回该关时自动恢复。"
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
