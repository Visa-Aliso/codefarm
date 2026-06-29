import QtQuick
import CodeFarm
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: failRoot
    property int levelId: 1
    property string reason: "timeout"
    property string hint: ""
    readonly property var navigator: failRoot.StackView ? failRoot.StackView.view : null
    property bool introReady: false

    Component.onCompleted: introReady = true

    SceneBackdrop {
        anchors.fill: parent
        showParticles: false
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.10)
    }

    Rectangle {
        id: panel
        anchors.centerIn: parent
        width: 540
        height: 392
        radius: 34
        opacity: failRoot.introReady ? 1 : 0
        scale: failRoot.introReady ? 1 : 0.96
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Theme.surfaceRaised }
            GradientStop { position: 1.0; color: Theme.cardBg }
        }
        border.width: 1
        border.color: Theme.borderStrong

        Behavior on opacity {
            NumberAnimation {
                duration: 240
                easing.type: Easing.OutQuad
            }
        }

        Behavior on scale {
            NumberAnimation {
                duration: 260
                easing.type: Easing.OutCubic
            }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 32
            spacing: 18

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 150
                Layout.preferredHeight: 34
                radius: 17
                color: Qt.rgba(Theme.danger.r, Theme.danger.g, Theme.danger.b, 0.12)
                border.width: 1
                border.color: Qt.rgba(Theme.danger.r, Theme.danger.g, Theme.danger.b, 0.22)

                Text {
                    anchors.centerIn: parent
                    text: "MISSION FAILED"
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    color: Theme.danger
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: failRoot.reason === "timeout" ? "时间到"
                    : failRoot.reason === "script_completed" ? "脚本已结束"
                    : "挑战结束"
                font.family: Theme.fontUI
                font.pixelSize: 28
                font.weight: Font.Bold
                color: Theme.danger
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: failRoot.hint
                font.family: Theme.fontUI
                font.pixelSize: 14
                color: Theme.textPrimary
                visible: failRoot.hint.length > 0
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 12

                PillButton {
                    text: "重新挑战"
                    onClicked: {
                        if (failRoot.navigator) {
                            failRoot.navigator.replace(
                                        null,
                                        "qrc:/CodeFarm/qml/pages/GameView.qml",
                                        { levelId: failRoot.levelId }
                                    )
                        }
                    }
                }

                PillButton {
                    text: "关卡选择"
                    onClicked: {
                        if (failRoot.navigator) {
                            failRoot.navigator.replace(null, "qrc:/CodeFarm/qml/pages/LevelSelect.qml")
                        }
                    }
                }
            }
        }
    }
}
