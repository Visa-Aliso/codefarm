pragma ComponentBehavior: Bound
import QtQuick
import CodeFarm
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: briefingRoot
    property int levelId: 1
    readonly property var levelsStore: levelManager
    readonly property var levelInfo: briefingRoot.levelsStore.getLevel(levelId)
    readonly property var navigator: briefingRoot.StackView ? briefingRoot.StackView.view : null
    property bool introReady: false

    Component.onCompleted: introReady = true

    SceneBackdrop {
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 18
        opacity: briefingRoot.introReady ? 1 : 0

        transform: Translate {
            id: briefingShift
            y: briefingRoot.introReady ? 0 : 20

            Behavior on y {
                NumberAnimation {
                    duration: 320
                    easing.type: Easing.OutCubic
                }
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 260
                easing.type: Easing.OutQuad
            }
        }

        RowLayout {
            Layout.fillWidth: true

            PillButton {
                text: "← 返回"
                onClicked: {
                    if (briefingRoot.navigator) {
                        briefingRoot.navigator.pop()
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                Layout.preferredWidth: 158
                Layout.preferredHeight: 40
                radius: 20
                color: Qt.rgba(1, 1, 1, 0.66)
                border.width: 1
                border.color: Theme.borderStrong

                Text {
                    anchors.centerIn: parent
                    text: "关卡 %1".arg(briefingRoot.levelId)
                    font.family: Theme.fontCode
                    font.pixelSize: 12
                    color: Theme.textPrimary
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 18

            FloatingPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "Mission Brief"
                subtitle: "任务说明与目标拆解"
                accentColor: Theme.fieldGold

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 18

                    Text {
                        text: briefingRoot.levelInfo.name || ""
                        font.family: Theme.fontUI
                        font.pixelSize: 32
                        font.weight: Font.Bold
                        color: Theme.textPrimary
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: briefingRoot.levelInfo.description || ""
                        font.family: Theme.fontUI
                        font.pixelSize: 15
                        color: Theme.textSecondary
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    RowLayout {
                        spacing: 12

                        Repeater {
                            model: [
                                "地图 %1 × %2".arg(briefingRoot.levelInfo.gridW || 0).arg(briefingRoot.levelInfo.gridH || 0),
                                "限时 %1s".arg(briefingRoot.levelInfo.maxTimeSec || 0),
                                "可用指令 %1".arg((briefingRoot.levelInfo.allowedFunctions || []).length)
                            ]

                            Rectangle {
                                id: statChip
                                required property string modelData
                                width: statText.implicitWidth + 22
                                height: 36
                                radius: 18
                                color: Qt.rgba(1, 1, 1, 0.44)
                                border.width: 1
                                border.color: Theme.border

                                Text {
                                    id: statText
                                    anchors.centerIn: parent
                                    text: statChip.modelData
                                    font.family: Theme.fontUI
                                    font.pixelSize: 12
                                    color: Theme.textPrimary
                                }
                            }
                        }
                    }

                    Text {
                        text: "本关目标"
                        font.family: Theme.fontUI
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        color: Theme.textPrimary
                    }

                    Repeater {
                        model: briefingRoot.levelInfo.goals || []

                        delegate: Rectangle {
                            id: goalRow
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.preferredHeight: 58
                            radius: 18
                            color: Qt.rgba(1, 1, 1, 0.48)
                            border.width: 1
                            border.color: Theme.border

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 14
                                spacing: 12

                                Rectangle {
                                    Layout.preferredWidth: 34
                                    Layout.preferredHeight: 34
                                    radius: 17
                                    color: Qt.rgba(Theme.starGold.r, Theme.starGold.g, Theme.starGold.b, 0.16)
                                    border.width: 1
                                    border.color: Qt.rgba(Theme.starGold.r, Theme.starGold.g, Theme.starGold.b, 0.26)

                                    Text {
                                        anchors.centerIn: parent
                                        text: "★" + (goalRow.modelData.starTier || 1)
                                        font.family: Theme.fontCode
                                        font.pixelSize: 11
                                        color: Theme.starGold
                                    }
                                }

                                Text {
                                    text: goalRow.modelData.description || ""
                                    Layout.fillWidth: true
                                    font.family: Theme.fontUI
                                    font.pixelSize: 14
                                    color: Theme.textPrimary
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 360
                Layout.fillHeight: true
                spacing: 16

                FloatingPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 232
                    title: "Allowed API"
                    subtitle: "本关允许调用的方法"
                    accentColor: Theme.primaryGreen

                    Flow {
                        anchors.fill: parent
                        spacing: 8

                        Repeater {
                            model: briefingRoot.levelInfo.allowedFunctions || []

                            delegate: Rectangle {
                                id: apiChip
                                required property var modelData
                                width: apiText.implicitWidth + 22
                                height: 34
                                radius: 17
                                color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.10)
                                border.width: 1
                                border.color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.20)

                                Text {
                                    id: apiText
                                    anchors.centerIn: parent
                                    text: apiChip.modelData + "()"
                                    font.family: Theme.fontCode
                                    font.pixelSize: 12
                                    color: Theme.primaryGreen
                                }
                            }
                        }
                    }
                }

                FloatingPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    title: "Launch Checklist"
                    subtitle: "开始前的关键提示"
                    accentColor: Theme.secondaryBlue

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        Text {
                            text: "1. 先观察目标与限时，再决定脚本是单次流程还是巡逻循环。"
                            font.family: Theme.fontUI
                            font.pixelSize: 13
                            color: Theme.textPrimary
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Text {
                            text: "2. 如果脚本太早结束但目标未完成，通常缺少 wait()、巡逻或收割阶段。"
                            font.family: Theme.fontUI
                            font.pixelSize: 13
                            color: Theme.textPrimary
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Text {
                            text: "3. 运行中注意日志、tick 与能量，必要时用单步定位。"
                            font.family: Theme.fontUI
                            font.pixelSize: 13
                            color: Theme.textPrimary
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }

                        Item { Layout.fillHeight: true }

                        PillButton {
                            Layout.alignment: Qt.AlignHCenter
                            primary: true
                            text: "开始挑战"
                            onClicked: {
                                if (briefingRoot.navigator) {
                                    briefingRoot.navigator.push(
                                                "qrc:/CodeFarm/qml/pages/GameView.qml",
                                                { levelId: briefingRoot.levelId }
                                            )
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
