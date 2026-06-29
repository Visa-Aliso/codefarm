pragma ComponentBehavior: Bound
import QtQuick
import CodeFarm
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: resultRoot
    property int levelId: 1
    property int stars: 2
    property var goals: []
    property int timeUsed: 0
    readonly property var levelStore: levelManager
    readonly property var levelInfo: resultRoot.levelStore.getLevel(resultRoot.levelId)
    readonly property var navigator: resultRoot.StackView ? resultRoot.StackView.view : null
    property bool introReady: false

    Component.onCompleted: introReady = true

    SceneBackdrop {
        anchors.fill: parent
        showParticles: false
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(1, 1, 1, 0.10)
    }

    Rectangle {
        id: panel
        anchors.centerIn: parent
        width: 620
        height: 560
        radius: 34
        opacity: resultRoot.introReady ? 1 : 0
        scale: resultRoot.introReady ? 1 : 0.96
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
                Layout.preferredWidth: 156
                Layout.preferredHeight: 34
                radius: 17
                color: Qt.rgba(Theme.success.r, Theme.success.g, Theme.success.b, 0.12)
                border.width: 1
                border.color: Qt.rgba(Theme.success.r, Theme.success.g, Theme.success.b, 0.22)

                Text {
                    anchors.centerIn: parent
                    text: "MISSION CLEAR"
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    color: Theme.success
                }
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: "关卡通关"
                font.family: Theme.fontUI
                font.pixelSize: 30
                font.weight: Font.Bold
                color: Theme.textPrimary
            }

            Text {
                Layout.alignment: Qt.AlignHCenter
                text: (resultRoot.levelInfo.name || "") + " · 用时 %1s".arg(resultRoot.timeUsed)
                font.family: Theme.fontUI
                font.pixelSize: 14
                color: Theme.textSecondary
            }

            StarRating {
                Layout.alignment: Qt.AlignHCenter
                count: resultRoot.stars
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.border
            }

            Repeater {
                model: resultRoot.goals

                delegate: Rectangle {
                    id: goalRow
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.preferredHeight: 52
                    radius: 18
                    color: goalRow.modelData.completed
                           ? Qt.rgba(Theme.success.r, Theme.success.g, Theme.success.b, 0.10)
                           : Qt.rgba(1, 1, 1, 0.36)
                    border.width: 1
                    border.color: goalRow.modelData.completed
                                  ? Qt.rgba(Theme.success.r, Theme.success.g, Theme.success.b, 0.24)
                                  : Theme.border

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 14
                        anchors.rightMargin: 14
                        spacing: 10

                        Rectangle {
                            Layout.preferredWidth: 28
                            Layout.preferredHeight: 28
                            radius: 14
                            color: goalRow.modelData.completed ? Theme.success : Qt.rgba(0.2, 0.22, 0.16, 0.10)

                            Text {
                                anchors.centerIn: parent
                                text: goalRow.modelData.completed ? "✓" : "•"
                                font.pixelSize: 13
                                font.weight: Font.Bold
                                color: goalRow.modelData.completed ? Theme.textOnDark : Theme.textSecondary
                            }
                        }

                        Text {
                            text: goalRow.modelData.description || ""
                            Layout.fillWidth: true
                            font.family: Theme.fontUI
                            font.pixelSize: 14
                            color: Theme.textPrimary
                        }

                        Text {
                            text: "★" + (goalRow.modelData.starTier || 1)
                            font.family: Theme.fontCode
                            font.pixelSize: 11
                            color: Theme.starGold
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 12

                PillButton {
                    text: "重新挑战"
                    onClicked: {
                        if (resultRoot.navigator) {
                            resultRoot.navigator.replace(
                                        null,
                                        "qrc:/CodeFarm/qml/pages/GameView.qml",
                                        { levelId: resultRoot.levelId }
                                    )
                        }
                    }
                }

                PillButton {
                    primary: true
                    text: resultRoot.levelId < resultRoot.levelStore.levelCount() ? "下一关 →" : "返回关卡"
                    onClicked: {
                        if (!resultRoot.navigator) {
                            return
                        }
                        if (resultRoot.levelId < resultRoot.levelStore.levelCount()) {
                            resultRoot.navigator.replace(
                                        null,
                                        "qrc:/CodeFarm/qml/pages/LevelBriefing.qml",
                                        { levelId: resultRoot.levelId + 1 }
                                    )
                        } else {
                            resultRoot.navigator.replace(null, "qrc:/CodeFarm/qml/pages/LevelSelect.qml")
                        }
                    }
                }

                PillButton {
                    text: "关卡选择"
                    onClicked: {
                        if (resultRoot.navigator) {
                            resultRoot.navigator.replace(null, "qrc:/CodeFarm/qml/pages/LevelSelect.qml")
                        }
                    }
                }
            }
        }
    }
}
