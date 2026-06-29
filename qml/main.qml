import QtQuick
import QtQuick.Controls
import QtQuick.Window
import CodeFarm

ApplicationWindow {
    id: root
    width: 1280
    height: 800
    minimumWidth: 1180
    minimumHeight: 760
    visible: true
    title: "Code Farm：智能果园"
    flags: Qt.Window | Qt.FramelessWindowHint
    color: Theme.windowBg

    Rectangle {
        anchors.fill: parent
        color: Theme.windowBg
        border.width: 1
        border.color: Theme.shellBorder
    }

    WindowChrome {
        id: windowChrome
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        targetWindow: root
    }

    StackView {
        id: navigator
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: windowChrome.bottom
        anchors.bottom: parent.bottom

        pushEnter: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 240
                    easing.type: Easing.OutQuad
                }
                PropertyAnimation {
                    property: "x"
                    from: navigator.width * 0.025
                    to: 0
                    duration: 280
                    easing.type: Easing.OutCubic
                }
                PropertyAnimation {
                    property: "scale"
                    from: 1.01
                    to: 1.0
                    duration: 240
                    easing.type: Easing.OutQuad
                }
            }
        }
        pushExit: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 170
                    easing.type: Easing.OutQuad
                }
                PropertyAnimation {
                    property: "x"
                    from: 0
                    to: -navigator.width * 0.015
                    duration: 170
                    easing.type: Easing.OutQuad
                }
            }
        }
        popEnter: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 240
                    easing.type: Easing.OutQuad
                }
                PropertyAnimation {
                    property: "x"
                    from: -navigator.width * 0.02
                    to: 0
                    duration: 240
                    easing.type: Easing.OutCubic
                }
                PropertyAnimation {
                    property: "scale"
                    from: 1.01
                    to: 1.0
                    duration: 220
                    easing.type: Easing.OutQuad
                }
            }
        }
        popExit: Transition {
            ParallelAnimation {
                PropertyAnimation {
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 170
                    easing.type: Easing.OutQuad
                }
                PropertyAnimation {
                    property: "x"
                    from: 0
                    to: navigator.width * 0.02
                    duration: 170
                    easing.type: Easing.OutQuad
                }
            }
        }

        Component.onCompleted: {
            push("qrc:/CodeFarm/qml/pages/SplashScreen.qml")
        }
    }
}
