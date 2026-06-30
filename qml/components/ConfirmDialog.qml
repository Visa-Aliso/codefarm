import QtQuick
import CodeFarm

Rectangle {
    id: root
    anchors.fill: parent
    color: Theme.overlayDark
    visible: false
    z: 1000

    property string title: ""
    property string message: ""

    signal confirmed()
    signal cancelled()

    function show(t, m) {
        title = t
        message = m
        visible = true
    }

    MouseArea { anchors.fill: parent; onClicked: {} }

    Rectangle {
        width: 320
        height: col.height + 48
        anchors.centerIn: parent
        radius: 12
        color: Theme.editorBg
        border.width: 1
        border.color: Theme.borderDim

        Column {
            id: col
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 24
            spacing: 16

            Text {
                text: root.title
                color: Theme.textLight
                font.family: Theme.fontUI
                font.pixelSize: 18
                font.weight: Font.Bold
            }

            Text {
                text: root.message
                color: Theme.textDim
                font.family: Theme.fontUI
                font.pixelSize: 14
                width: parent.width
                wrapMode: Text.WordWrap
            }

            Row {
                spacing: 12
                anchors.horizontalCenter: parent.horizontalCenter

                MenuButton {
                    text: "取消"
                    implicitW: 100
                    implicitH: 36
                    onClicked: { root.visible = false; root.cancelled() }
                }
                MenuButton {
                    text: "确认"
                    primary: true
                    implicitW: 100
                    implicitH: 36
                    onClicked: { root.visible = false; root.confirmed() }
                }
            }
        }
    }
}
