import QtQuick
import CodeFarm

Rectangle {
    id: root
    color: Theme.overlayDark
    visible: false

    property string reason: ""

    MouseArea { anchors.fill: parent; onClicked: {} }

    Rectangle {
        width: 360
        height: contentCol.height + 48
        anchors.centerIn: parent
        radius: 12
        color: Theme.editorBg
        border.width: 1
        border.color: Theme.statusError

        scale: root.visible ? 1.0 : 0.95
        Behavior on scale { NumberAnimation { duration: 200; easing.type: Easing.OutQuad } }

        Column {
            id: contentCol
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 24
            spacing: 16

            // Icon
            Rectangle {
                width: 48; height: 48; radius: 24
                color: Theme.statusError
                anchors.horizontalCenter: parent.horizontalCenter
                Text {
                    anchors.centerIn: parent
                    text: "!"
                    color: Theme.textLight
                    font.pixelSize: 24
                    font.weight: Font.Bold
                }
            }

            Text {
                text: {
                    switch (root.reason) {
                        case "timeout": return "时间到了"
                        default: return "任务未完成"
                    }
                }
                color: Theme.statusError
                font.family: Theme.fontUI
                font.pixelSize: 20
                font.weight: Font.Bold
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: appVm.failureHint(root.reason)
                color: Theme.textDim
                font.family: Theme.fontUI
                font.pixelSize: 13
                width: parent.width
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Row {
                spacing: 12
                anchors.horizontalCenter: parent.horizontalCenter
                topPadding: 8

                MenuButton {
                    text: "返回"
                    implicitW: 100; implicitH: 38
                    onClicked: { root.visible = false; navigator.pop() }
                }
                MenuButton {
                    text: "重新挑战"
                    primary: true
                    implicitW: 120; implicitH: 38
                    onClicked: { root.visible = false; appVm.resetLevel() }
                }
            }
        }
    }
}
