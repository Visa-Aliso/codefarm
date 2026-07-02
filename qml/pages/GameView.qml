import QtQuick
import QtQuick.Controls
import CodeFarm

Rectangle {
    id: root
    width: parent ? parent.width : 0
    height: parent ? parent.height : 0
    color: Theme.bgMain

    property var level: appVm.activeLevel || {}

    function toggleHint() {
        hintWindow.visible = !hintWindow.visible
    }

    function getHintNewContent() {
        return level.id ? appVm.levelNewContent(level.id) : ({})
    }

    property bool isQuitting: false

    // ---- Animated background clouds (large, staggered, always visible) ----
    Repeater {
        model: appVm.bgAnimations ? [
            { w: 260, h: 90,  y: 15,  speed: 90000,  stagger: 0    },
            { w: 200, h: 70,  y: 50,  speed: 110000, stagger: 0.15 },
            { w: 300, h: 105, y: 90,  speed: 75000,  stagger: 0.3  },
            { w: 180, h: 65,  y: 130, speed: 120000, stagger: 0.45 },
            { w: 240, h: 85,  y: 35,  speed: 85000,  stagger: 0.6  },
            { w: 220, h: 78,  y: 70,  speed: 95000,  stagger: 0.75 },
            { w: 280, h: 98,  y: 110, speed: 80000,  stagger: 0.88 },
            { w: 170, h: 60,  y: 150, speed: 130000, stagger: 0.05 },
            { w: 250, h: 88,  y: 0,   speed: 100000, stagger: 0.35 },
            { w: 190, h: 68,  y: 140, speed: 115000, stagger: 0.55 },
            { w: 230, h: 82,  y: 55,  speed: 105000, stagger: 0.9  },
            { w: 160, h: 56,  y: 120, speed: 125000, stagger: 0.2  }
        ] : []
        delegate: Item {
            y: modelData.y
            width: modelData.w
            height: modelData.h
            opacity: 0.1
            z: 0

            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    ctx.fillStyle = "#FFFFFF"
                    var rw = modelData.w, rh = modelData.h
                    // Main body
                    ctx.beginPath()
                    ctx.ellipse(rw*0.05, rh*0.4, rw*0.45, rh*0.45, 0, 0, Math.PI*2)
                    ctx.fill()
                    // Top bump
                    ctx.beginPath()
                    ctx.ellipse(rw*0.2, rh*0.15, rw*0.3, rh*0.5, 0, 0, Math.PI*2)
                    ctx.fill()
                    // Side bump
                    ctx.beginPath()
                    ctx.ellipse(rw*0.45, rh*0.25, rw*0.28, rh*0.4, 0, 0, Math.PI*2)
                    ctx.fill()
                    // Extra small bump
                    ctx.beginPath()
                    ctx.ellipse(rw*0.65, rh*0.35, rw*0.2, rh*0.35, 0, 0, Math.PI*2)
                    ctx.fill()
                }
                Component.onCompleted: requestPaint()
            }

            // First loop: stagger position → right edge (partial)
            // Subsequent loops: left edge → right edge (full)
            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation {
                    from: -(modelData.w + 40) + modelData.stagger * (root.width + modelData.w + 80)
                    to: root.width > 0 ? root.width + 40 : 1400
                    duration: (1 - modelData.stagger) * modelData.speed
                }
                NumberAnimation {
                    from: -(modelData.w + 40)
                    to: root.width > 0 ? root.width + 40 : 1400
                    duration: modelData.speed
                }
            }
        }
    }

    // Top-left: back button (green square style)
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 12
        anchors.leftMargin: 16
        z: 100
        width: 32; height: 32; radius: 6
        color: backBtnMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
        Text { anchors.centerIn: parent; text: "←"; color: "white"; font.pixelSize: 16; font.weight: Font.Bold }
        MouseArea { id: backBtnMa; anchors.fill: parent; hoverEnabled: true
            onClicked: { root.isQuitting = true; appVm.giveUp(); navigator.pop() }
        }
    }

    // Top-center: elapsed timer
    Text {
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        z: 100
        text: {
            var sec = appVm.timeElapsed
            var m = Math.floor(sec / 60)
            var s = sec % 60
            return (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
        }
        color: Theme.textLight
        font.family: "Fredoka One"
        font.pixelSize: 28
        style: Text.Raised
        styleColor: "#30000000"
    }



    // Isometric farm map (center, with proper offset for top bar)
    IsometricMap {
        id: farmMapView
        anchors.fill: parent
        anchors.topMargin: 50
        gridWidth: farmMap.gridWidth
        gridHeight: farmMap.gridHeight
        mapModel: farmMap
        droneX: appVm.droneX
        droneY: appVm.droneY
    }

    // Goal overlay (bottom-left, draggable)
    Item {
        id: goalContainer
        x: 16
        y: root.height - goalPanel.height - 16
        width: goalPanel.width
        height: goalPanel.height
        z: 80

        GoalOverlay {
            id: goalPanel
            goals: appVm.activeGoals
        }

        MouseArea {
            anchors.fill: parent
            property real sx: 0
            property real sy: 0
            onPressed: function(mouse) { sx = mouse.x; sy = mouse.y }
            onPositionChanged: function(mouse) {
                goalContainer.x += mouse.x - sx
                goalContainer.y += mouse.y - sy
            }
        }
    }

    // Floating code editor
    FloatingEditor {
        id: codeEditor
        x: parent.width - width - 24
        y: 60
        width: Math.min(460, parent.width * 0.35)
        height: (parent.height - 100) / 2
        z: 90
        text: appVm.scriptText
        executingLine: appVm.executingLine
        state: appVm.state
        statusText: appVm.consoleLine
        onCodeEdited: function(newText) { appVm.saveScript(newText) }
        onRunClicked: appVm.runOrPause()
        onPauseClicked: appVm.runOrPause()
        onStopClicked: appVm.resetLevel()
        onStepClicked: appVm.stepOnce()
        onLoadTutorialClicked: appVm.resetScriptToTutorial()
        onResetClicked: {
            codeEditor.x = root.width - codeEditor.width - 24
            codeEditor.y = 60
            codeEditor.width = Math.min(460, root.width * 0.35)
            codeEditor.height = (root.height - 100) / 2
        }
        onCloseRequested: codeEditor.visible = false
    }

    // Hint/tutorial window (draggable, hidden by default unless autoShowHint is on)
    Item {
        id: hintWindow
        x: 56
        y: 50
        width: 280
        height: Math.min(hintCol.height + 42, root.height - 80)
        z: 95
        visible: appVm.autoShowHint

        Rectangle {
            anchors.fill: parent
            radius: 8
            color: Theme.editorBg
            clip: true

            // Title bar (draggable)
            Rectangle {
                id: hintTitleBar
                width: parent.width; height: 34; radius: 8
                color: Theme.editorTitleBar
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 8; color: parent.color }

                MouseArea {
                    anchors.fill: parent
                    property real sx: 0
                    property real sy: 0
                    onPressed: function(mouse) { sx = mouse.x; sy = mouse.y }
                    onPositionChanged: function(mouse) {
                        hintWindow.x += mouse.x - sx
                        hintWindow.y += mouse.y - sy
                    }
                }

                Text {
                    anchors.left: parent.left; anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    text: "关卡提示"; color: Theme.textDim; font.family: Theme.fontCode; font.pixelSize: 12
                }

                Rectangle {
                    anchors.right: parent.right; anchors.rightMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    width: 26; height: 26; radius: 5
                    color: hintMinMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
                    Text { anchors.centerIn: parent; text: "─"; color: "white"; font.pixelSize: 14; font.weight: Font.Bold }
                    MouseArea { id: hintMinMa; anchors.fill: parent; hoverEnabled: true
                        onClicked: hintWindow.visible = false
                    }
                }
            }

            // Scrollable content
            Flickable {
                anchors.top: hintTitleBar.bottom
                anchors.left: parent.left; anchors.right: parent.right
                anchors.bottom: parent.bottom
                contentHeight: hintCol.height + 16
                clip: true
                flickableDirection: Flickable.VerticalFlick

                Column {
                    id: hintCol
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.margins: 10
                    topPadding: 8
                    spacing: 8

                    // Level description
                    Text {
                        text: level.description || ""
                        color: Theme.textLight
                        font.family: Theme.fontUI; font.pixelSize: 12
                        width: parent.width; wrapMode: Text.WordWrap
                    }

                    // Learning objectives
                    Column {
                        width: parent.width; spacing: 4
                        visible: level.id && getLearningObjectives(level.id).length > 0
                        Text { text: "学习目标"; color: Theme.btnGreen; font.family: Theme.fontUI; font.pixelSize: 12; font.weight: Font.Bold }
                        Repeater {
                            model: getLearningObjectives(level.id)
                            Text {
                                text: "• " + modelData
                                color: Theme.textDim; font.family: Theme.fontUI; font.pixelSize: 11
                                width: parent.width; wrapMode: Text.WordWrap
                            }
                        }
                    }

                    // Goals section
                    Column {
                        width: parent.width; spacing: 4
                        visible: level.goals && level.goals.length > 0
                        Text { text: "通关目标"; color: Theme.btnGreen; font.family: Theme.fontUI; font.pixelSize: 12; font.weight: Font.Bold }
                        Repeater {
                            model: level.goals || []
                            Rectangle {
                                width: parent.width; height: goalHintRow.height + 10
                                radius: 4; color: Qt.rgba(0,0,0,0.2)
                                Row {
                                    id: goalHintRow
                                    anchors.left: parent.left; anchors.right: parent.right
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.margins: 6
                                    spacing: 6
                                    Text {
                                        text: {
                                            var tier = modelData.starTier || 1
                                            if (tier === 1) return "★"
                                            if (tier === 2) return "★★"
                                            return "★★★"
                                        }
                                        color: Theme.starGold; font.pixelSize: 10
                                    }
                                    Text {
                                        text: modelData.description || ""
                                        color: Theme.textDim; font.family: Theme.fontUI; font.pixelSize: 11
                                        width: parent.width - 50; elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }

                    // New functions — styled card
                    Column {
                        width: parent.width; spacing: 6
                        visible: getHintNewContent().newFunctions && getHintNewContent().newFunctions.length > 0
                        Text { text: "本关新函数"; color: Theme.btnGreen; font.family: Theme.fontUI; font.pixelSize: 12; font.weight: Font.Bold }
                        Repeater {
                            model: getHintNewContent().newFunctions || []
                            Rectangle {
                                width: parent.width; height: nfCol.height + 12
                                radius: 6; color: Qt.rgba(0,0,0,0.25)
                                border.width: 1; border.color: Qt.rgba(1,1,1,0.06)
                                Column {
                                    id: nfCol
                                    anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 2
                                    Text {
                                        text: {
                                            var docs = getFunctionDocs()
                                            for (var i = 0; i < docs.length; i++) {
                                                if (docs[i].funcName === modelData) return docs[i].name
                                            }
                                            return modelData + "()"
                                        }
                                        color: Theme.statusRunning; font.family: Theme.fontCode; font.pixelSize: 12; font.weight: Font.Bold
                                    }
                                    Text {
                                        text: {
                                            var docs = getFunctionDocs()
                                            for (var i = 0; i < docs.length; i++) {
                                                if (docs[i].funcName === modelData) return docs[i].desc
                                            }
                                            return ""
                                        }
                                        color: Theme.textDim; font.family: Theme.fontUI; font.pixelSize: 10
                                        width: parent.width; wrapMode: Text.WordWrap
                                    }
                                }
                            }
                        }
                    }

                    // New syntax — styled cards with explanations
                    Column {
                        width: parent.width; spacing: 6
                        visible: getHintNewContent().newSyntax && getHintNewContent().newSyntax.length > 0
                        Text { text: "本关新语法"; color: Theme.btnGreen; font.family: Theme.fontUI; font.pixelSize: 12; font.weight: Font.Bold }
                        Repeater {
                            model: getHintNewContent().newSyntax || []
                            Rectangle {
                                width: parent.width; height: synCardCol.height + 12
                                radius: 6; color: Qt.rgba(0,0,0,0.25)
                                border.width: 1; border.color: Qt.rgba(1,1,1,0.06)
                                Column {
                                    id: synCardCol
                                    anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                    spacing: 2
                                    Text {
                                        text: modelData
                                        color: Theme.statusRunning; font.family: Theme.fontCode; font.pixelSize: 12; font.weight: Font.Bold
                                    }
                                    Text {
                                        text: {
                                            var docs = getSyntaxDocs()
                                            if (docs[modelData]) return docs[modelData]
                                            return ""
                                        }
                                        color: Theme.textDim; font.family: Theme.fontUI; font.pixelSize: 10
                                        width: parent.width; wrapMode: Text.WordWrap
                                    }
                                }
                            }
                        }
                    }

                    // New crops — styled card
                    Column {
                        width: parent.width; spacing: 6
                        visible: getHintNewContent().newCrops && getHintNewContent().newCrops.length > 0
                        Text { text: "本关新作物"; color: Theme.btnGreen; font.family: Theme.fontUI; font.pixelSize: 12; font.weight: Font.Bold }
                        Rectangle {
                            width: parent.width; height: cropCol.height + 16
                            radius: 6; color: Qt.rgba(0,0,0,0.25)
                            border.width: 1; border.color: Qt.rgba(1,1,1,0.06)
                            Column {
                                id: cropCol
                                anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 8
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 4
                                Repeater {
                                    model: getHintNewContent().newCrops || []
                                    Text {
                                        text: modelData
                                        color: Theme.statusRunning; font.family: Theme.fontCode; font.pixelSize: 12; font.weight: Font.Bold
                                    }
                                }
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.borderDim; visible: getHintNewContent().newFunctions && getHintNewContent().newFunctions.length > 0 || getHintNewContent().newSyntax && getHintNewContent().newSyntax.length > 0 || getHintNewContent().newCrops && getHintNewContent().newCrops.length > 0 }

                    Item { width: 1; height: 8 }
                }
            }
        }
    }

    function getLearningObjectives(lvlId) {
        var obj = {
            1: ["理解程序从上到下顺序执行","学会使用 # 写注释（机器不执行的说明文字）"],
            2: ["学会用 = 把值存到变量里","理解 if 条件判断：满足条件才执行"],
            3: ["学会 for 循环：for i in range(n) 重复 n 次","理解嵌套循环：循环里面套循环"],
            4: ["学会 while 循环：条件为真就一直重复","学会 get_current() 查询当前格状态","理解用字典键取值: dict['key']"],
            5: ["学会 def 定义函数，封装重复流程","理解函数参数：def plant_row(crop) 可传入不同作物"],
            6: ["学会 return 让函数返回计算结果","学会 elif/else 处理多分支判断","学会 not 取反逻辑"],
            7: ["学会 and/or 组合多个条件","学会 fertilize() 施肥加速生长"],
            8: ["学会 break 跳出循环、continue 跳过本次","学会 spray() 除虫"],
            9: ["综合运用所学：大规模种植 + 巡逻管理"],
            10: ["学会元组解包: a, b = get_pos()","学会 get_tick() 计时"],
            11: ["学会列表字面量 [a, b, c] 和切片 [1:3]","学会 len() 求长度、enumerate() 带下标遍历"],
            12: ["学会增强赋值 += -= 累加计数","学会 min/max/sum 统计函数","了解向日葵：不可收割但加速邻居"],
            13: ["学会字典字面量 {键: 值}","学会 items()/keys()/values() 遍历字典"],
            14: ["学会列表推导式 [x for x in xs if cond]","学会 get_goals() 查询目标进度"],
            15: ["学会 zip() 并行遍历两个列表","学会 sorted() 排序"],
            16: ["学会 all() 全真才真、any() 一真即真","学会嵌套函数（函数里面定义函数）"],
            17: ["学会 lambda 写匿名小函数","理解闭包：内层函数记住外层变量"],
            18: ["学会字典推导式 {k: v for ...}","综合运用所有技巧"],
            19: ["学会生成器表达式 (x for x in xs) 惰性求值","学会集合推导式 {x for x in xs} 去重"],
            20: ["终极综合：全部 API + 全部语法 + 全部机制","尝试用最优雅的方式写出高效解法"]
        }
        return obj[lvlId] || []
    }

    function getSyntaxDocs() {
        return {
            "for": "重复执行一段代码。for i in range(3): 会循环 3 次，i 依次取 0,1,2。",
            "while": "条件为真时一直重复。while x > 0: move('left') 直到 x 回到 0 才停止。",
            "if": "条件判断，满足条件才执行后面的缩进代码块。",
            "else": "与 if 配合，条件不满足时执行的代码块。也可与循环配合（循环正常结束后执行）。",
            "elif": "else + if 的缩写，用于多分支判断。if...elif...else 依次检查。",
            "def": "定义函数，把重复流程封装成可复用的代码块。def 函数名(参数): ...",
            "return": "让函数返回计算结果。return True 或 return 表达式。执行到 return 函数就结束。",
            "and": "逻辑与：两边都为 True 结果才是 True。c['state'] == 'mature' and c['crop'] != 'sunflower'",
            "or": "逻辑或：至少一边为 True 结果就是 True。crop == 'carrot' or crop == 'tomato'",
            "not": "逻辑非：True 变 False，False 变 True。not is_mature() 取反。",
            "break": "跳出循环。常用于提前结束 for/while 循环。",
            "continue": "跳过本次循环剩余代码，直接进入下一次迭代。",
            "assign": "变量赋值。用 = 把值存到变量里：crop = 'wheat' 然后 plant(crop)。",
            "augassign": "增强赋值。harvested += 1 等价于 harvested = harvested + 1。还有 -= *= /=。",
            "tuple": "元组：不可修改的序列。(x, y) 或直接写成 x, y。常用于多变量赋值：a, b = get_pos()。",
            "list": "列表：可修改的序列。[1, 2, 3] 或 list(range(5))。支持切片、append。",
            "dict": "字典：键值对映射。{'wheat': 6, 'carrot': 14}。用 [键] 取值，items() 遍历。",
            "listcomp": "列表推导式：一行生成列表。[c for c in crops if c != 'sunflower'] 筛选非向日葵。",
            "dictcomp": "字典推导式：一行生成字典。{c: 0 for c in crops} 初始化计数器。",
            "setcomp": "集合推导式：一行去重。{c['crop'] for c in cells if c['crop']}。",
            "genexp": "生成器表达式：惰性求值。sum(1 for c in cells if c['hasBug']) 统计虫害数。",
            "lambda": "匿名函数：lambda x: x * 2 一个小函数。常用于 sorted(xs, key=lambda g: g['target'])。"
        }
    }

    function getFunctionDocs() {
        var docs = {
            "move": {funcName: "move", name: "move(方向)", desc: "移动无人机一格。方向: \"up\"/\"down\"/\"left\"/\"right\"。不能移出地图，不能移动到岩石格子。"},
            "harvest": {funcName: "harvest", name: "harvest()", desc: "收割当前格已成熟的作物（进度=1.0且无虫害）。向日葵不可收割，它会持续提供阳光加成。"},
            "till": {funcName: "till", name: "till()", desc: "将当前空地开垦为可种植状态。当前格必须为空地，岩石格子无法犁地。"},
            "plant": {funcName: "plant", name: "plant(作物)", desc: "在已犁地上种植。作物: \"wheat\"/\"carrot\"/\"tomato\"/\"corn\"/\"sunflower\"。向日葵不可收割但提供阳光加成。"},
            "water": {funcName: "water", name: "water()", desc: "浇水+0.35水分（最高1.0）。可多次叠加。注意：胡萝卜和向日葵水>0.85会过涝受伤！"},
            "fertilize": {funcName: "fertilize", name: "fertilize()", desc: "施肥加速生长。水分≥0.6时1.8倍，否则1.5倍，持续12 tick。"},
            "spray": {funcName: "spray", name: "spray()", desc: "清除当前格虫害，并获得12 tick虫害免疫。"},
            "wait": {funcName: "wait", name: "wait()", desc: "等待一个tick（约0.5秒），不做任何操作。"},
            "get_pos": {funcName: "get_pos", name: "get_pos()", desc: "返回坐标元组 (x, y)。x=列，y=行。"},
            "get_current": {funcName: "get_current", name: "get_current()", desc: "返回格子字典: state, crop, water, hasBug, progress, fertilized。state含\"rock\"。"},
            "get_map_size": {funcName: "get_map_size", name: "get_map_size()", desc: "返回地图尺寸元组 (w, h)。"},
            "get_tick": {funcName: "get_tick", name: "get_tick()", desc: "返回当前已执行的tick数（整数）。可用于计时。"},
            "get_goals": {funcName: "get_goals", name: "get_goals()", desc: "返回目标列表，每项含: description, current, target, completed, starTier。"},
            "debug": {funcName: "debug", name: "debug()", desc: "在控制台打印当前格详细信息，同时返回格子信息字典。"},
            "range": {funcName: "range", name: "range(n)", desc: "生成 0..n-1 的整数序列，常与 for 配合。解锁: 关卡3"},
            "tuple": {funcName: "tuple", name: "tuple()", desc: "元组构造器。也可用 a, b = ... 解包。解锁: 关卡10"},
            "len": {funcName: "len", name: "len(x)", desc: "返回列表/字典/字符串的长度。解锁: 关卡11"},
            "enumerate": {funcName: "enumerate", name: "enumerate(xs)", desc: "返回 (下标, 元素) 对，常用于 for 循环。解锁: 关卡11"},
            "min": {funcName: "min", name: "min(xs)", desc: "返回最小值。解锁: 关卡12"},
            "max": {funcName: "max", name: "max(xs)", desc: "返回最大值。解锁: 关卡12"},
            "sum": {funcName: "sum", name: "sum(xs)", desc: "返回总和。解锁: 关卡12"},
            "dict": {funcName: "dict", name: "dict()", desc: "字典构造器。也可用 {k: v} 字面量。解锁: 关卡13"},
            "list": {funcName: "list", name: "list(x)", desc: "列表构造器，可将可迭代对象转为列表。解锁: 关卡13"},
            "sorted": {funcName: "sorted", name: "sorted(xs)", desc: "返回排好序的新列表，可用 key=lambda 指定排序键。解锁: 关卡15"},
            "zip": {funcName: "zip", name: "zip(a, b)", desc: "将多个序列配对，返回 (a[i], b[i]) 元组。解锁: 关卡15"},
            "all": {funcName: "all", name: "all(xs)", desc: "全真才返回 True。解锁: 关卡16"},
            "any": {funcName: "any", name: "any(xs)", desc: "一真即返回 True。解锁: 关卡16"},
            "round": {funcName: "round", name: "round(x, n)", desc: "四舍五入到 n 位小数。解锁: 关卡16"},
            "abs": {funcName: "abs", name: "abs(x)", desc: "返回绝对值。解锁: 关卡16"}
        }
        var result = []
        var apis = appVm.allowedApis
        for (var i = 0; i < apis.length; i++) {
            if (docs[apis[i]]) result.push(docs[apis[i]])
        }
        // Also include allowed builtins from the active level
        var lvl = appVm.activeLevel
        if (lvl && lvl.allowedBuiltins) {
            for (var j = 0; j < lvl.allowedBuiltins.length; j++) {
                var b = lvl.allowedBuiltins[j]
                if (docs[b] && result.indexOf(docs[b]) < 0) result.push(docs[b])
            }
        }
        return result
    }

    // Result overlay
    ResultScreen {
        id: resultOverlay
        anchors.fill: parent
        z: 200
        visible: false
    }

    // Fail overlay
    FailScreen {
        id: failOverlay
        anchors.fill: parent
        z: 200
        visible: false
    }

    // Confirm dialog
    ConfirmDialog {
        id: confirmDialog
        z: 300
        onConfirmed: {
            appVm.giveUp()
            navigator.pop()
        }
    }

    Connections {
        target: appVm
        function onLevelCleared(stars) {
            if (root.isQuitting) return
            resultOverlay.stars = stars
            resultOverlay.visible = true
            audioManager.playSfx("clear")
        }
        function onLevelFailed(reason) {
            if (root.isQuitting) return
            failOverlay.reason = reason
            failOverlay.visible = true
            audioManager.playSfx("fail")
        }
    }
}
