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

    property bool isQuitting: false

    // Background clouds (static, painted once for performance)
    Canvas {
        id: bgClouds
        anchors.fill: parent
        opacity: 0.12
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.fillStyle = "#FFFFFF"
            var clouds = [
                [120, 70, 80, 30], [320, 50, 60, 22], [550, 90, 100, 35],
                [780, 40, 70, 25], [1000, 80, 90, 32], [200, 160, 50, 18],
                [650, 150, 65, 24], [900, 130, 55, 20], [1150, 60, 75, 28]
            ]
            for (var i = 0; i < clouds.length; i++) {
                var cx = clouds[i][0], cy = clouds[i][1], rw = clouds[i][2], rh = clouds[i][3]
                ctx.beginPath()
                ctx.ellipse(cx - rw/2, cy - rh/2, rw, rh)
                ctx.fill()
                ctx.beginPath()
                ctx.ellipse(cx - rw*0.3, cy - rh*0.7, rw*0.6, rh*0.8)
                ctx.fill()
                ctx.beginPath()
                ctx.ellipse(cx + rw*0.2, cy - rh*0.5, rw*0.5, rh*0.7)
                ctx.fill()
            }
        }
        Component.onCompleted: requestPaint()
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

    // Top-center: resource bar
    ResourceBar {
        id: resourceBar
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width * 0.45, 520)
        z: 100
        timeElapsed: appVm.timeElapsed
        tickCount: appVm.tickCount
        energy: appVm.energy
        maxEnergy: appVm.maxEnergy
        state: appVm.state
        levelName: level.name || ""
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

    // Drone sprite (positioned relative to map center)
    Item {
        id: droneContainer
        anchors.centerIn: farmMapView
        width: 0; height: 0
        z: 50

        Canvas {
            id: droneCanvas
            width: 40; height: 40
            property real floatY: 0
            SequentialAnimation on floatY {
                loops: Animation.Infinite
                NumberAnimation { from: 0; to: -6; duration: 1200; easing.type: Easing.InOutSine }
                NumberAnimation { from: -6; to: 0; duration: 1200; easing.type: Easing.InOutSine }
            }

            x: {
                var ox = farmMap.gridHeight * farmMapView.tileW / 2
                var sx = ox + (appVm.droneX - appVm.droneY) * farmMapView.tileW / 2
                return sx - width/2
            }
            y: {
                var oy = 16
                var sy = oy + (appVm.droneX + appVm.droneY) * farmMapView.tileH / 2
                return sy - height + floatY - 10
            }

            Behavior on x { SmoothedAnimation { velocity: 250 } }
            Behavior on y { SmoothedAnimation { velocity: 250 } }

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                var cx = width/2, cy = height/2

                ctx.fillStyle = "#E0E0E0"
                ctx.beginPath()
                ctx.roundedRect(cx-10, cy-3, 20, 12, 4, 4)
                ctx.fill()

                ctx.strokeStyle = "#777"
                ctx.lineWidth = 2
                ctx.beginPath()
                ctx.moveTo(cx-8, cy); ctx.lineTo(cx-18, cy-6)
                ctx.moveTo(cx+8, cy); ctx.lineTo(cx+18, cy-6)
                ctx.stroke()

                ctx.fillStyle = "rgba(76,175,80,0.4)"
                ctx.beginPath()
                ctx.arc(cx-18, cy-6, 7, 0, Math.PI*2)
                ctx.arc(cx+18, cy-6, 7, 0, Math.PI*2)
                ctx.fill()

                ctx.fillStyle = "#4CAF50"
                ctx.beginPath()
                ctx.arc(cx, cy+4, 2.5, 0, Math.PI*2)
                ctx.fill()
            }
            Component.onCompleted: requestPaint()
        }

        // Drone shadow
        Rectangle {
            width: 16; height: 6; radius: 3
            color: Qt.rgba(0,0,0,0.15)
            x: droneCanvas.x + droneCanvas.width/2 - 8
            y: droneCanvas.y + droneCanvas.height + 4
        }
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
        onResetClicked: {
            codeEditor.x = root.width - codeEditor.width - 24
            codeEditor.y = 60
            codeEditor.width = Math.min(460, root.width * 0.35)
            codeEditor.height = (root.height - 100) / 2
        }
        onCloseRequested: codeEditor.visible = false
    }

    // Hint/tutorial window (draggable, hidden by default)
    Item {
        id: hintWindow
        x: 56
        y: 50
        width: 280
        height: Math.min(hintCol.height + 42, root.height - 80)
        z: 95
        visible: false

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
                    Text { anchors.centerIn: parent; text: "─"; color: "white"; font.pixelSize: 11; font.weight: Font.Bold }
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

                    Rectangle { width: parent.width; height: 1; color: Theme.borderDim }

                    // Function documentation
                    Text { text: "可用函数："; color: Theme.textDim; font.family: Theme.fontCode; font.pixelSize: 11 }

                    Column {
                        width: parent.width
                        spacing: 4
                        Repeater {
                            model: getFunctionDocs()
                            Rectangle {
                                width: parent.width; height: funcDocCol.height + 8
                                radius: 4; color: Qt.rgba(1,1,1,0.04)
                                Column {
                                    id: funcDocCol
                                    anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 6
                                    anchors.verticalCenter: parent.verticalCenter
                                    Text { text: modelData.name; color: Theme.statusRunning; font.family: Theme.fontCode; font.pixelSize: 11; font.weight: Font.Bold }
                                    Text { text: modelData.desc; color: Theme.textDim; font.family: Theme.fontUI; font.pixelSize: 10; width: parent.width; wrapMode: Text.WordWrap }
                                }
                            }
                        }
                    }

                    Rectangle { width: parent.width; height: 1; color: Theme.borderDim }

                    // Answer section
                    Text { text: "参考答案："; color: Theme.textDim; font.family: Theme.fontCode; font.pixelSize: 11 }
                    Rectangle {
                        width: parent.width
                        height: answerText.height + 12
                        radius: 4; color: Qt.rgba(0,0,0,0.3)
                        Text {
                            id: answerText
                            anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 8
                            anchors.verticalCenter: parent.verticalCenter
                            text: level.tutorialCode || ""
                            color: Theme.textLight
                            font.family: Theme.fontCode; font.pixelSize: 11
                            lineHeight: 1.4
                            wrapMode: Text.WordWrap
                        }
                    }

                    Item { width: 1; height: 8 }
                }
            }
        }
    }

    function getFunctionDocs() {
        var docs = {
            "move": {name: "move(方向)", desc: "移动无人机。方向: \"up\"/\"down\"/\"left\"/\"right\""},
            "harvest": {name: "harvest()", desc: "收割当前格成熟的作物"},
            "till": {name: "till()", desc: "开垦当前格土地"},
            "plant": {name: "plant(作物)", desc: "种植作物: \"wheat\"/\"carrot\"/\"tomato\"/\"corn\"/\"sunflower\""},
            "water": {name: "water()", desc: "给当前格浇水，促进作物生长"},
            "fertilize": {name: "fertilize()", desc: "施肥，加速生长1.5倍"},
            "spray": {name: "spray()", desc: "喷洒农药，清除虫害"},
            "debug": {name: "debug()", desc: "修复当前格的虫害"},
            "wait": {name: "wait()", desc: "等待一个tick，不执行操作"},
            "get_pos": {name: "get_pos()", desc: "返回无人机当前坐标 (x, y)"},
            "get_current": {name: "get_current()", desc: "返回当前格信息"},
            "get_map_size": {name: "get_map_size()", desc: "返回地图尺寸 (w, h)"},
            "get_energy": {name: "get_energy()", desc: "返回剩余能量"},
            "get_tick": {name: "get_tick()", desc: "返回当前tick数"},
            "get_goals": {name: "get_goals()", desc: "返回关卡目标列表"}
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
        }
        function onLevelFailed(reason) {
            if (root.isQuitting) return
            failOverlay.reason = reason
            failOverlay.visible = true
        }
    }
}
