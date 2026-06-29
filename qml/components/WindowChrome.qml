import QtQuick
import QtQuick.Layouts
import QtQuick.Window
import CodeFarm

Rectangle {
    id: chrome
    property var targetWindow
    property string title: "Code Farm"
    property string subtitle: "清新自然的农场编程工作台"
    property bool showCloseButton: true
    property point dragStart: Qt.point(0, 0)

    height: 54
    radius: 0
    color: Theme.chromeBg
    border.width: 1
    border.color: Theme.chromeBorder

    function beginMove(mouse) {
        if (!chrome.targetWindow) {
            return
        }
        if (typeof chrome.targetWindow.startSystemMove === "function") {
            chrome.targetWindow.startSystemMove()
        } else {
            chrome.dragStart = Qt.point(mouse.x, mouse.y)
        }
    }

    function continueMove(mouse) {
        if (!chrome.targetWindow || typeof chrome.targetWindow.startSystemMove === "function") {
            return
        }
        if (mouse.buttons & Qt.LeftButton) {
            chrome.targetWindow.x += mouse.x - chrome.dragStart.x
            chrome.targetWindow.y += mouse.y - chrome.dragStart.y
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: Theme.shellBorder
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 14
        anchors.rightMargin: 10
        spacing: 10

        Item {
            Layout.preferredWidth: 12
            Layout.preferredHeight: 12

            Rectangle {
                anchors.centerIn: parent
                width: 12
                height: 12
                radius: 6
                color: Theme.primaryGreen
            }
        }

        ColumnLayout {
            spacing: -2

            Text {
                text: chrome.title
                font.family: Theme.fontUI
                font.pixelSize: 14
                font.weight: Font.Bold
                color: Theme.textPrimary
            }

            Text {
                text: chrome.subtitle
                font.family: Theme.fontUI
                font.pixelSize: 11
                color: Theme.textSecondary
            }
        }

        Item { Layout.fillWidth: true }

        Row {
            spacing: 8

            Rectangle {
                id: minButton
                width: 30
                height: 30
                radius: 15
                color: minArea.containsMouse ? Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.12)
                                             : "transparent"
                border.width: 1
                border.color: minArea.containsMouse ? Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.16)
                                                    : Theme.border

                Text {
                    anchors.centerIn: parent
                    text: "–"
                    font.family: Theme.fontUI
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    color: Theme.textPrimary
                }

                MouseArea {
                    id: minArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (chrome.targetWindow) {
                            chrome.targetWindow.visibility = Window.Minimized
                        }
                    }
                }
            }

            Rectangle {
                width: 30
                height: 30
                radius: 15
                visible: chrome.showCloseButton
                color: closeArea.containsMouse ? Qt.rgba(Theme.danger.r, Theme.danger.g, Theme.danger.b, 0.12)
                                               : "transparent"
                border.width: 1
                border.color: closeArea.containsMouse ? Qt.rgba(Theme.danger.r, Theme.danger.g, Theme.danger.b, 0.18)
                                                      : Theme.border

                Text {
                    anchors.centerIn: parent
                    text: "×"
                    font.family: Theme.fontUI
                    font.pixelSize: 16
                    font.weight: Font.Bold
                    color: Theme.textPrimary
                }

                MouseArea {
                    id: closeArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: Qt.quit()
                }
            }
        }
    }

    MouseArea {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: chrome.showCloseButton ? 86 : 46
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.SizeAllCursor
        onPressed: (mouse) => chrome.beginMove(mouse)
        onPositionChanged: (mouse) => chrome.continueMove(mouse)
    }
}
