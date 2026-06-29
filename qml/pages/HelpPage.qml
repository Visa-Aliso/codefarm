import QtQuick
import CodeFarm
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: helpRoot
    readonly property var navigator: helpRoot.StackView ? helpRoot.StackView.view : null
    property bool introReady: false
    readonly property var apiList: [
        "move(direction)",
        "till()",
        "plant(crop)",
        "water()",
        "fertilize()",
        "spray()",
        "harvest()",
        "wait()"
    ]

    Component.onCompleted: introReady = true

    SceneBackdrop {
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 26
        spacing: 18
        opacity: helpRoot.introReady ? 1 : 0

        transform: Translate {
            id: helpShift
            y: helpRoot.introReady ? 0 : 18

            Behavior on y {
                NumberAnimation {
                    duration: 280
                    easing.type: Easing.OutCubic
                }
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 220
                easing.type: Easing.OutQuad
            }
        }

        RowLayout {
            Layout.fillWidth: true

            PillButton {
                text: "← 返回"
                onClicked: {
                    if (helpRoot.navigator) {
                        helpRoot.navigator.pop()
                    }
                }
            }

            Text {
                text: "帮助与玩法说明"
                font.family: Theme.fontUI
                font.pixelSize: 30
                font.weight: Font.Bold
                color: Theme.textPrimary
            }

            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            FloatingPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Play Loop"
                subtitle: "从编写脚本到完成目标"
                accentColor: Theme.primaryGreen

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Text {
                        text: "1. 进入关卡先阅读目标、限时和可用 API。"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "2. 在右侧编辑器编写 Python 脚本，用运行或单步观察执行。"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "3. 每个动作通常消耗一个 tick；作物成长、虫害、能量恢复也都在 tick 中推进。"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "4. 如果脚本结束但目标未完成，通常说明缺少等待、生长、巡逻或收割逻辑。"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Item { Layout.fillHeight: true }
                }
            }

            FloatingPanel {
                Layout.preferredWidth: 360
                Layout.fillHeight: true
                title: "Quick API"
                subtitle: "常用指令速览"
                accentColor: Theme.secondaryBlue

                Flow {
                    anchors.fill: parent
                    spacing: 8

                    Repeater {
                        model: helpRoot.apiList

                        delegate: Rectangle {
                            id: apiChip
                            required property string modelData
                            width: chipText.implicitWidth + 22
                            height: 34
                            radius: 17
                            color: Qt.rgba(Theme.secondaryBlue.r, Theme.secondaryBlue.g, Theme.secondaryBlue.b, 0.12)
                            border.width: 1
                            border.color: Qt.rgba(Theme.secondaryBlue.r, Theme.secondaryBlue.g, Theme.secondaryBlue.b, 0.20)

                            Text {
                                id: chipText
                                anchors.centerIn: parent
                                text: apiChip.modelData
                                font.family: Theme.fontCode
                                font.pixelSize: 12
                                color: Theme.secondaryBlue
                            }
                        }
                    }
                }
            }
        }
    }
}
