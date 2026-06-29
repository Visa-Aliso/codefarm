import QtQuick
import CodeFarm
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: menuRoot
    readonly property var navigator: menuRoot.StackView ? menuRoot.StackView.view : null
    property bool introReady: false

    Component.onCompleted: introReady = true

    SceneBackdrop {
        anchors.fill: parent
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(1, 1, 1, 0.02)
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 50
        spacing: 34
        opacity: menuRoot.introReady ? 1 : 0

        transform: Translate {
            id: menuShift
            y: menuRoot.introReady ? 0 : 24

            Behavior on y {
                NumberAnimation {
                    duration: 380
                    easing.type: Easing.OutCubic
                }
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 320
                easing.type: Easing.OutQuad
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 22

            Item { Layout.fillHeight: true }

            Rectangle {
                Layout.preferredWidth: 170
                Layout.preferredHeight: 34
                radius: 17
                color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.11)
                border.width: 1
                border.color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.18)

                Text {
                    anchors.centerIn: parent
                    text: "FARM AUTOMATION"
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    color: Theme.primaryGreen
                }
            }

            Text {
                text: "Code Farm"
                font.family: Theme.fontUI
                font.pixelSize: 72
                font.weight: Font.Bold
                color: Theme.textPrimary
            }

            Text {
                Layout.maximumWidth: 640
                text: "把写代码、观察地图和调整策略合成一套清晰自然的农场调度体验。无人机、能量、作物成长和目标系统会在同一个 tick 回路里真实推进。"
                font.family: Theme.fontUI
                font.pixelSize: 19
                color: Theme.textSecondary
                wrapMode: Text.WordWrap
            }

            FloatingPanel {
                Layout.preferredWidth: 640
                Layout.preferredHeight: 204
                title: "Today On The Farm"
                subtitle: "用更自然的方式开始一局"
                accentColor: Theme.primaryGreen

                RowLayout {
                    anchors.fill: parent
                    spacing: 12

                    Repeater {
                        model: [
                            { label: "脚本控制", value: "Python" },
                            { label: "地图视角", value: "Isometric" },
                            { label: "目标反馈", value: "Star tiers" }
                        ]

                        Rectangle {
                            id: statCard
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 20
                            color: Qt.rgba(1, 1, 1, 0.62)
                            border.width: 1
                            border.color: Theme.border

                            Column {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 8

                                Rectangle {
                                    width: 34
                                    height: 34
                                    radius: 17
                                    color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.10)
                                    border.width: 1
                                    border.color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.14)

                                    Text {
                                        anchors.centerIn: parent
                                        text: statCard.modelData.label.charAt(0)
                                        font.family: Theme.fontCode
                                        font.pixelSize: 13
                                        font.weight: Font.Bold
                                        color: Theme.primaryGreen
                                    }
                                }

                                Text {
                                    text: statCard.modelData.label
                                    font.family: Theme.fontUI
                                    font.pixelSize: 13
                                    color: Theme.textSecondary
                                }

                                Text {
                                    text: statCard.modelData.value
                                    font.family: Theme.fontUI
                                    font.pixelSize: 22
                                    font.weight: Font.Bold
                                    color: Theme.textPrimary
                                }
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }

        FloatingPanel {
            Layout.preferredWidth: 388
            Layout.fillHeight: true
            title: "进入作业台"
            subtitle: "简单、整齐、直接开始"
            accentColor: Theme.secondaryBlue

            ColumnLayout {
                anchors.fill: parent
                spacing: 16

                Text {
                    text: "从关卡选择进入，阅读任务、编写脚本、观察地图反馈，再逐步把农场跑顺。"
                    font.family: Theme.fontUI
                    font.pixelSize: 14
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: Theme.border
                }

                PillButton {
                    Layout.alignment: Qt.AlignHCenter
                    primary: true
                    text: "开始挑战"
                    onClicked: {
                        if (menuRoot.navigator) {
                            menuRoot.navigator.push("qrc:/CodeFarm/qml/pages/LevelSelect.qml")
                        }
                    }
                }

                PillButton {
                    Layout.alignment: Qt.AlignHCenter
                    text: "设置"
                    onClicked: {
                        if (menuRoot.navigator) {
                            menuRoot.navigator.push("qrc:/CodeFarm/qml/pages/SettingsPage.qml")
                        }
                    }
                }

                PillButton {
                    Layout.alignment: Qt.AlignHCenter
                    text: "帮助"
                    onClicked: {
                        if (menuRoot.navigator) {
                            menuRoot.navigator.push("qrc:/CodeFarm/qml/pages/HelpPage.qml")
                        }
                    }
                }

                PillButton {
                    Layout.alignment: Qt.AlignHCenter
                    text: "退出"
                    onClicked: Qt.quit()
                }

                Item { Layout.fillHeight: true }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    radius: 22
                    color: Qt.rgba(Theme.secondaryBlue.r, Theme.secondaryBlue.g, Theme.secondaryBlue.b, 0.08)
                    border.width: 1
                    border.color: Qt.rgba(Theme.secondaryBlue.r, Theme.secondaryBlue.g, Theme.secondaryBlue.b, 0.14)

                    Column {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 6

                        Text {
                            text: "视觉方向"
                            font.family: Theme.fontUI
                            font.pixelSize: 14
                            font.weight: Font.Bold
                            color: Theme.textPrimary
                        }

                        Text {
                            text: "这版会更偏清新自然：浅色层次、轻一点的玻璃感、明确的版式和更柔和的动画。"
                            font.family: Theme.fontUI
                            font.pixelSize: 12
                            color: Theme.textSecondary
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }

    Text {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 22
        text: "v0.1.0"
        font.family: Theme.fontUI
        font.pixelSize: 12
        color: Theme.textDisabled
    }
}
