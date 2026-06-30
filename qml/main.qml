import QtQuick
import QtQuick.Window
import QtQuick.Controls
import CodeFarm

Window {
    id: window
    width: 1360
    height: 860
    minimumWidth: 1180
    minimumHeight: 760
    visible: true
    title: "Code Farm"
    flags: Qt.FramelessWindowHint | Qt.Window
    color: Theme.bgMain

    // Custom title bar drag area (center only, avoids buttons on left/right)
    MouseArea {
        id: dragArea
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 60
        anchors.rightMargin: 100
        height: 32
        z: 500
        property real startX: 0
        property real startY: 0
        onPressed: function(mouse) { startX = mouse.x; startY = mouse.y }
        onPositionChanged: function(mouse) {
            window.x += mouse.x - startX
            window.y += mouse.y - startY
        }
    }

    // Window control buttons (top-right, green square style)
    Row {
        id: windowControls
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 12
        anchors.rightMargin: 12
        z: 10000
        spacing: 6

        Rectangle {
            width: 32; height: 32; radius: 6
            color: helpBtnMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
            visible: navigator.depth >= 2
                    && !(navigator.currentItem && (navigator.currentItem.isHelpPage || navigator.currentItem.isSettingsPage))
            Text { anchors.centerIn: parent; text: "?"; color: "white"; font.pixelSize: 16; font.weight: Font.Bold }
            MouseArea { id: helpBtnMa; anchors.fill: parent; hoverEnabled: true
                onClicked: {
                    var gv = navigator.currentItem
                    if (gv && gv.toggleHint) {
                        gv.toggleHint()
                    } else {
                        navigator.push(helpPage)
                    }
                }
            }
        }
        Rectangle {
            width: 32; height: 32; radius: 6
            color: minMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
            Text { anchors.centerIn: parent; text: "─"; color: "white"; font.pixelSize: 16; font.weight: Font.Bold }
            MouseArea { id: minMa; anchors.fill: parent; hoverEnabled: true; onClicked: window.showMinimized() }
        }
        Rectangle {
            width: 32; height: 32; radius: 6
            color: closeMa.containsMouse ? "#8B3030" : Theme.btnGreen
            Text { anchors.centerIn: parent; text: "×"; color: "white"; font.pixelSize: 16; font.weight: Font.Bold }
            MouseArea { id: closeMa; anchors.fill: parent; hoverEnabled: true; onClicked: Qt.quit() }
        }
    }

    StackView {
        id: navigator
        anchors.fill: parent
        initialItem: mainMenuPage

        pushEnter: Transition {
            PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
        }
        pushExit: Transition {
            PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 150 }
        }
        popEnter: Transition {
            PropertyAnimation { property: "opacity"; from: 0; to: 1; duration: 200 }
        }
        popExit: Transition {
            PropertyAnimation { property: "opacity"; from: 1; to: 0; duration: 150 }
        }
    }

    Component { id: mainMenuPage; MainMenu {} }
    Component { id: levelSelectPage; LevelSelect {} }
    Component { id: gameViewPage; GameView {} }
    Component { id: helpPage; HelpPage {} }
    Component { id: settingsPage; SettingsPage {} }
}
