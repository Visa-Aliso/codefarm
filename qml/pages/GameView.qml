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

                    // New syntax — styled card
                    Column {
                        width: parent.width; spacing: 6
                        visible: getHintNewContent().newSyntax && getHintNewContent().newSyntax.length > 0
                        Text { text: "本关新语法"; color: Theme.btnGreen; font.family: Theme.fontUI; font.pixelSize: 12; font.weight: Font.Bold }
                        Rectangle {
                            width: parent.width; height: syntaxCol.height + 16
                            radius: 6; color: Qt.rgba(0,0,0,0.25)
                            border.width: 1; border.color: Qt.rgba(1,1,1,0.06)
                            Column {
                                id: syntaxCol
                                anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 8
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 4
                                Repeater {
                                    model: getHintNewContent().newSyntax || []
                                    Text {
                                        text: modelData
                                        color: Theme.statusRunning; font.family: Theme.fontCode; font.pixelSize: 12; font.weight: Font.Bold
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
            "get_tick": {funcName: "get_tick", name: "get_tick()", desc: "返回当前已执行的tick数（整数）。"},
            "get_goals": {funcName: "get_goals", name: "get_goals()", desc: "返回目标列表，每项含: description, current, target, completed, starTier。"},
            "debug": {funcName: "debug", name: "debug()", desc: "在控制台打印当前格详细信息，同时返回格子信息字典。"}
        }
        var result = []
        var apis = appVm.allowedApis
        for (var i = 0; i < apis.length; i++) {
            if (docs[apis[i]]) result.push(docs[apis[i]])
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
