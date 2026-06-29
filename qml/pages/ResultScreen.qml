import QtQuick
import CodeFarm

Rectangle {
    id: root
    color: Theme.overlayDark
    visible: false

    property int stars: 0
    property var goals: appVm.activeGoals
    property int timeUsed: appVm.timeElapsed

    MouseArea { anchors.fill: parent; onClicked: {} }

    Rectangle {
        width: 400
        height: contentCol.height + 48
        anchors.centerIn: parent
        radius: 12
        color: Theme.editorBg
        border.width: 1
        border.color: Theme.statusRunning

        Column {
            id: contentCol
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 24
            spacing: 16

            Text {
                text: "关卡完成！"
                color: Theme.statusRunning
                font.family: Theme.fontUI
                font.pixelSize: 24
                font.weight: Font.Bold
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // Stars
            Row {
                spacing: 8
                anchors.horizontalCenter: parent.horizontalCenter

                Repeater {
                    model: 3
                    Text {
                        text: "★"
                        color: index < root.stars ? Theme.starGold : Theme.textMuted
                        font.pixelSize: 32

                        scale: starAnim.running ? 1.0 : 0.8
                        NumberAnimation on scale {
                            id: starAnim
                            from: 0.5; to: 1.0
                            duration: 300
                            easing.type: Easing.OutBack
                            running: root.visible && index < root.stars
                        }
                    }
                }
            }

            // Stats
            Row {
                spacing: 24
                anchors.horizontalCenter: parent.horizontalCenter

                Column {
                    spacing: 2
                    Text { text: "TIME"; color: Theme.textMuted; font.family: Theme.fontCode; font.pixelSize: 10 }
                    Text { text: root.timeUsed + "s"; color: Theme.textLight; font.family: Theme.fontCode; font.pixelSize: 16 }
                }
                Column {
                    spacing: 2
                    Text { text: "STARS"; color: Theme.textMuted; font.family: Theme.fontCode; font.pixelSize: 10 }
                    Text { text: root.stars + "/3"; color: Theme.starGold; font.family: Theme.fontCode; font.pixelSize: 16 }
                }
            }

            // Goals list
            Column {
                width: parent.width
                spacing: 6

                Repeater {
                    model: root.goals
                    Row {
                        spacing: 8
                        Text {
                            text: modelData.completed ? "✓" : "✗"
                            color: modelData.completed ? Theme.statusRunning : Theme.statusError
                            font.pixelSize: 14
                        }
                        Text {
                            text: modelData.description || ""
                            color: Theme.textDim
                            font.family: Theme.fontUI
                            font.pixelSize: 13
                        }
                    }
                }
            }

            // Buttons
            Row {
                spacing: 12
                anchors.horizontalCenter: parent.horizontalCenter
                topPadding: 8

                MenuButton {
                    text: "重试"
                    implicitW: 100; implicitH: 38
                    onClicked: { root.visible = false; appVm.resetLevel() }
                }
                MenuButton {
                    text: "下一关"
                    primary: true
                    implicitW: 120; implicitH: 38
                    onClicked: {
                        root.visible = false
                        var next = appVm.nextUnlockedLevel()
                        if (next > 0) { appVm.openLevel(next) }
                        else { navigator.pop() }
                    }
                }
                MenuButton {
                    text: "返回"
                    implicitW: 100; implicitH: 38
                    onClicked: { root.visible = false; navigator.pop() }
                }
            }
        }
    }
}
