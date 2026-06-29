import QtQuick
import CodeFarm
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: gameViewRoot
    property int levelId: 1
    readonly property var engine: gameEngine
    readonly property var navigator: gameViewRoot.StackView ? gameViewRoot.StackView.view : null
    property bool confirmVisible: false
    property string confirmMode: ""
    property bool introReady: false

    Component.onCompleted: {
        gameViewRoot.engine.loadLevel(gameViewRoot.levelId)
        gameViewRoot.introReady = true
    }

    SceneBackdrop {
        anchors.fill: parent
        showParticles: false
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(1, 1, 1, 0.04)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 14

        TopBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 74
            opacity: gameViewRoot.introReady ? 1 : 0

            transform: Translate {
                id: gameTopShift
                y: gameViewRoot.introReady ? 0 : -18

                Behavior on y {
                    NumberAnimation {
                        duration: 300
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
            onBackRequested: {
                gameViewRoot.confirmMode = "leave"
                gameViewRoot.confirmVisible = true
            }
            onGiveUpRequested: {
                gameViewRoot.confirmMode = "give_up"
                gameViewRoot.confirmVisible = true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: 860
                opacity: gameViewRoot.introReady ? 1 : 0
                scale: gameViewRoot.introReady ? 1 : 0.985

                Behavior on opacity {
                    NumberAnimation {
                        duration: 320
                        easing.type: Easing.OutQuad
                    }
                }

                Behavior on scale {
                    NumberAnimation {
                        duration: 320
                        easing.type: Easing.OutCubic
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    anchors.topMargin: 8
                    radius: 32
                    color: Theme.panelShadow
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 32
                    gradient: Gradient {
                        orientation: Gradient.Vertical
                        GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.72) }
                        GradientStop { position: 1.0; color: Qt.rgba(Theme.surfaceStrong.r, Theme.surfaceStrong.g, Theme.surfaceStrong.b, 0.92) }
                    }
                    border.width: 1
                    border.color: Theme.borderStrong
                }

                IsometricMap {
                    anchors.fill: parent
                    anchors.margins: 20
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 448
                Layout.fillHeight: true
                spacing: 14
                opacity: gameViewRoot.introReady ? 1 : 0

                transform: Translate {
                    id: gameSideShift
                    y: gameViewRoot.introReady ? 0 : 20

                    Behavior on y {
                        NumberAnimation {
                            duration: 340
                            easing.type: Easing.OutCubic
                        }
                    }
                }

                Behavior on opacity {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.OutQuad
                    }
                }

                GoalPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 208
                }

                CodeEditor {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    levelId: gameViewRoot.levelId
                }

                DroneLog {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 188
                }
            }
        }
    }

    Connections {
        target: gameViewRoot.engine
        function onLevelCleared(stars) {
            if (gameViewRoot.navigator) {
                gameViewRoot.navigator.replace(null, "qrc:/CodeFarm/qml/pages/ResultScreen.qml", {
                    levelId: gameViewRoot.levelId,
                    stars: stars,
                    timeUsed: gameViewRoot.engine.timeElapsed,
                    goals: gameViewRoot.engine.goals
                })
            }
        }
        function onLevelFailed(reason) {
            if (gameViewRoot.navigator) {
                gameViewRoot.navigator.replace(null, "qrc:/CodeFarm/qml/pages/FailScreen.qml", {
                    levelId: gameViewRoot.levelId,
                    reason: reason,
                    hint: reason === "timeout" ? "超出关卡限制时间。"
                         : reason === "script_completed" ? "脚本已经执行结束，但还没有完成关卡目标。通常需要补充 wait()、spray() 或后续收割逻辑。"
                         : reason === "player_quit" ? "本次挑战已结束。"
                         : "请检查脚本日志和关卡目标。"
                })
            }
        }
    }

    ConfirmDialog {
        visible: gameViewRoot.confirmVisible
        title: gameViewRoot.confirmMode === "leave" ? "离开关卡？" : "放弃本关？"
        message: gameViewRoot.confirmMode === "leave"
                 ? "离开后会返回上一页，并重置当前关卡进度。"
                 : "本次挑战将直接判负，并进入失败结算。"
        acceptText: gameViewRoot.confirmMode === "leave" ? "离开" : "放弃"
        cancelText: "继续挑战"

        onAccepted: {
            gameViewRoot.confirmVisible = false
            if (gameViewRoot.confirmMode === "leave") {
                gameViewRoot.engine.pause()
                gameViewRoot.engine.reset()
                if (gameViewRoot.navigator) {
                    gameViewRoot.navigator.pop()
                }
            } else {
                gameViewRoot.engine.giveUp()
            }
        }

        onRejected: {
            gameViewRoot.confirmVisible = false
        }
    }
}
