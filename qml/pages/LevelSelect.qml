pragma ComponentBehavior: Bound
import QtQuick
import CodeFarm
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: levelSelectRoot
    readonly property var levelsStore: levelManager
    readonly property var navigator: levelSelectRoot.StackView ? levelSelectRoot.StackView.view : null
    property bool introReady: false

    Component.onCompleted: introReady = true

    SceneBackdrop {
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 28
        spacing: 18
        opacity: levelSelectRoot.introReady ? 1 : 0

        transform: Translate {
            id: levelSelectShift
            y: levelSelectRoot.introReady ? 0 : 20

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
            spacing: 16

            PillButton {
                text: "← 返回"
                onClicked: {
                    if (levelSelectRoot.navigator) {
                        levelSelectRoot.navigator.pop()
                    }
                }
            }

            Item { Layout.fillWidth: true }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: "选择作业区域"
                    font.family: Theme.fontUI
                    font.pixelSize: 32
                    font.weight: Font.Bold
                    color: Theme.textPrimary
                }

                Text {
                    text: "已通关 %1/%2 · 累计星级 %3"
                        .arg(levelSelectRoot.levelsStore.completedCount)
                        .arg(levelSelectRoot.levelsStore.levelCount())
                        .arg(levelSelectRoot.levelsStore.totalStars)
                    font.family: Theme.fontUI
                    font.pixelSize: 14
                    color: Theme.textSecondary
                }
            }

            Rectangle {
                Layout.preferredWidth: 240
                Layout.preferredHeight: 84
                radius: 24
                color: Qt.rgba(1, 1, 1, 0.64)
                border.width: 1
                border.color: Theme.borderStrong

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 14

                    Column {
                        spacing: 2

                        Text {
                            text: "FIELD ACCESS"
                            font.family: Theme.fontCode
                            font.pixelSize: 11
                            color: Theme.textSecondary
                        }

                        Text {
                            text: levelSelectRoot.levelsStore.completedCount + " 解锁"
                            font.family: Theme.fontUI
                            font.pixelSize: 22
                            font.weight: Font.Bold
                            color: Theme.textPrimary
                        }
                    }

                    Item { Layout.fillWidth: true }

                    StarRating {
                        count: Math.min(3, levelSelectRoot.levelsStore.totalStars)
                    }
                }
            }
        }

        ScrollView {
            id: levelScroll
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Item {
                width: levelScroll.availableWidth
                implicitHeight: levelFlow.implicitHeight + 24

                Flow {
                    id: levelFlow
                    width: parent.width
                    spacing: 20

                    Repeater {
                        model: levelSelectRoot.levelsStore.levelCount()

                        delegate: LevelCard {
                            required property int index
                            readonly property var levelInfo: levelSelectRoot.levelsStore.getLevel(index + 1)

                            levelId: index + 1
                            levelName: levelInfo.name || ""
                            unlocked: levelSelectRoot.levelsStore.isUnlocked(index + 1)
                            stars: levelSelectRoot.levelsStore.getStars(index + 1)
                            bestTime: levelSelectRoot.levelsStore.getBestTime(index + 1)

                            onClicked: {
                                if (levelSelectRoot.navigator) {
                                    levelSelectRoot.navigator.push(
                                                "qrc:/CodeFarm/qml/pages/LevelBriefing.qml",
                                                { levelId: index + 1 }
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
