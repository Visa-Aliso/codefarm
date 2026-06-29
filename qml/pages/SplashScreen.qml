import QtQuick
import CodeFarm
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: splash
    readonly property var navigator: splash.StackView ? splash.StackView.view : null

    SceneBackdrop {
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(1, 1, 1, 0.04)
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: 540
        spacing: 18

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 138
            Layout.preferredHeight: 34
            radius: 17
            color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.12)
            border.width: 1
            border.color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.18)

            Text {
                anchors.centerIn: parent
                text: "FIELD OPS SIM"
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    color: Theme.primaryGreen
                }
            }

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "Code Farm"
            font.family: Theme.fontUI
            font.pixelSize: 52
            font.weight: Font.Bold
            color: Theme.textPrimary
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: "用 Python 调度无人机，让农场在 tick 中真正运转起来"
            font.family: Theme.fontUI
            font.pixelSize: 16
            color: Theme.textSecondary
        }

        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 360
            Layout.preferredHeight: 8
            radius: 4
            color: Qt.rgba(0.14, 0.18, 0.12, 0.08)

            Rectangle {
                id: progressFill
                width: 0
                height: parent.height
                radius: parent.radius
                gradient: Gradient {
                    orientation: Gradient.Horizontal
                    GradientStop { position: 0.0; color: Theme.primaryGreen }
                    GradientStop { position: 0.5; color: Theme.fieldGold }
                    GradientStop { position: 1.0; color: Theme.secondaryBlue }
                }

                NumberAnimation {
                    target: progressFill
                    property: "width"
                    from: 0
                    to: 360
                    running: true
                    duration: 1200
                    easing.type: Easing.InOutQuad
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12

            Repeater {
                model: [
                    "真实 tick 驱动",
                    "脚本执行闭环",
                    "关卡目标判定"
                ]

                Rectangle {
                    id: splashChip
                    required property string modelData
                    width: pillText.implicitWidth + 20
                    height: 34
                    radius: 17
                    color: Qt.rgba(1, 1, 1, 0.45)
                    border.width: 1
                    border.color: Theme.border

                    Text {
                        id: pillText
                        anchors.centerIn: parent
                        text: splashChip.modelData
                        font.family: Theme.fontUI
                        font.pixelSize: 12
                        color: Theme.textPrimary
                    }
                }
            }
        }
    }

    Timer {
        interval: 1400
        running: true
        onTriggered: {
            if (splash.navigator) {
                splash.navigator.replace(null, "qrc:/CodeFarm/qml/pages/MainMenu.qml")
            }
        }
    }
}
