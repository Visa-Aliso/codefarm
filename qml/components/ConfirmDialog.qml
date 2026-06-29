import QtQuick
import CodeFarm
import QtQuick.Layouts

Item {
    id: confirmDialog
    property string title: "确认"
    property string message: ""
    property string acceptText: "确认"
    property string cancelText: "取消"
    signal accepted()
    signal rejected()

    anchors.fill: parent
    visible: false
    z: 100

    Rectangle {
        anchors.fill: parent
        color: Theme.overlayDark
    }

    MouseArea {
        anchors.fill: parent
        onClicked: confirmDialog.rejected()
    }

    FloatingPanel {
        id: panel
        anchors.centerIn: parent
        width: 408
        height: 238
        title: confirmDialog.title
        subtitle: "请确认这次操作"
        accentColor: Theme.warning
        scale: confirmDialog.visible ? 1.0 : 0.96
        opacity: confirmDialog.visible ? 1.0 : 0.0

        Behavior on scale {
            NumberAnimation {
                duration: 160
                easing.type: Easing.OutQuad
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 140
                easing.type: Easing.OutQuad
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 16

            Text {
                text: confirmDialog.message
                Layout.fillWidth: true
                font.family: Theme.fontUI
                font.pixelSize: 14
                color: Theme.textPrimary
                wrapMode: Text.WordWrap
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Item { Layout.fillWidth: true }

                PillButton {
                    text: confirmDialog.cancelText
                    onClicked: confirmDialog.rejected()
                }

                PillButton {
                    primary: true
                    text: confirmDialog.acceptText
                    onClicked: confirmDialog.accepted()
                }
            }
        }
    }
}
