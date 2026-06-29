import QtQuick
import QtQuick.Controls
import CodeFarm

Item {
    id: root
    width: 480
    height: 360
    z: focused ? 100 : 10

    property bool focused: true
    property string fileName: "solution.py"
    property alias text: textArea.text
    property int executingLine: -1
    property int state: 0
    property string statusText: ""

    signal runClicked()
    signal pauseClicked()
    signal stopClicked()
    signal stepClicked()
    signal resetClicked()
    signal codeEdited(string newText)
    signal closeRequested()

    property real minWidth: 320
    property real minHeight: 240

    Rectangle {
        id: background
        anchors.fill: parent
        radius: 8
        color: Theme.editorBg
        border.width: root.focused ? 1 : 0
        border.color: Theme.btnGreen
        opacity: root.focused ? 1.0 : 0.92

        // Title bar
        Rectangle {
            id: titleBar
            width: parent.width
            height: 34
            radius: 8
            color: Theme.editorTitleBar

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: parent.radius
                color: parent.color
            }

            MouseArea {
                id: dragArea
                anchors.fill: parent
                property real startX: 0
                property real startY: 0
                onPressed: function(mouse) {
                    startX = mouse.x; startY = mouse.y
                    root.focused = true
                }
                onPositionChanged: function(mouse) {
                    root.x += mouse.x - startX
                    root.y += mouse.y - startY
                }
            }

            // Left buttons: Run/Pause+Stop (green square style)
            Row {
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                // Run button (when idle/paused/error)
                Rectangle {
                    width: 26; height: 26; radius: 5
                    color: runBtnMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
                    visible: root.state !== 1
                    Text { anchors.centerIn: parent; text: "▶"; color: "white"; font.pixelSize: 11 }
                    MouseArea { id: runBtnMa; anchors.fill: parent; hoverEnabled: true; onClicked: root.runClicked() }
                }
                // Pause button (when running)
                Rectangle {
                    width: 26; height: 26; radius: 5
                    color: pauseBtnMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
                    visible: root.state === 1
                    Text { anchors.centerIn: parent; text: "⏸"; color: "white"; font.pixelSize: 11 }
                    MouseArea { id: pauseBtnMa; anchors.fill: parent; hoverEnabled: true; onClicked: root.pauseClicked() }
                }
                // Stop button (when running)
                Rectangle {
                    width: 26; height: 26; radius: 5
                    color: stopBtnMa.containsMouse ? "#8B3030" : Theme.btnGreen
                    visible: root.state === 1
                    Text { anchors.centerIn: parent; text: "■"; color: "white"; font.pixelSize: 11 }
                    MouseArea { id: stopBtnMa; anchors.fill: parent; hoverEnabled: true; onClicked: root.stopClicked() }
                }
            }

            // File name
            Text {
                anchors.centerIn: parent
                text: root.fileName
                color: Theme.textDim
                font.family: Theme.fontCode
                font.pixelSize: 12
            }

            // Right button: Minimize only
            Row {
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                Rectangle {
                    width: 26; height: 26; radius: 5
                    color: minBtnMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
                    Text { anchors.centerIn: parent; text: minBtnMa.isMin ? "□" : "─"; color: "white"; font.pixelSize: minBtnMa.isMin ? 12 : 11; font.weight: Font.Bold }
                    MouseArea { id: minBtnMa; anchors.fill: parent; hoverEnabled: true
                        property real prevH: 0
                        property bool isMin: false
                        onClicked: {
                            if (!isMin) {
                                prevH = root.height
                                root.height = 34
                                isMin = true
                            } else {
                                root.height = prevH > 34 ? prevH : 360
                                isMin = false
                            }
                        }
                    }
                }
            }
        }

        // Code editor area
        Item {
            id: editorArea
            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: statusBar.top

            // Text editor
            Flickable {
                id: flickable
                anchors.fill: parent
                anchors.right: parent.right
                contentWidth: textArea.width
                contentHeight: textArea.height
                clip: true
                flickableDirection: Flickable.VerticalFlick

                TextArea {
                    id: textArea
                    width: flickable.width
                    color: Theme.textLight
                    selectionColor: "#3A5A8C"
                    font.family: Theme.fontCode
                    font.pixelSize: 14
                    font.letterSpacing: 1.5
                    padding: 8
                    background: null
                    wrapMode: TextEdit.NoWrap
                    tabStopDistance: 4 * fontMetrics.advanceWidth(' ')
                    onTextChanged: root.codeEdited(text)

                    FontMetrics { id: fontMetrics; font: textArea.font }

                    SyntaxHighlighter {
                        id: highlighter
                        document: textArea.textDocument
                    }
                }
            }
        }

        // Status bar
        Rectangle {
            id: statusBar
            anchors.bottom: parent.bottom
            width: parent.width
            height: 24
            radius: 8
            color: Theme.editorStatusBar
            visible: root.height > 60

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: parent.radius
                color: parent.color
            }

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                Rectangle {
                    width: 8; height: 8; radius: 4
                    anchors.verticalCenter: parent.verticalCenter
                    color: {
                        switch (root.state) {
                            case 1: return Theme.statusRunning
                            case 2: return Theme.statusPaused
                            case 3: return Theme.statusError
                            default: return Theme.textMuted
                        }
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: {
                        switch (root.state) {
                            case 1: return "Running"
                            case 2: return "Paused"
                            case 3: return "Error"
                            default: return "Idle"
                        }
                    }
                    color: Theme.textDim
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                }
            }

            Text {
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                text: root.statusText
                color: Theme.textMuted
                font.family: Theme.fontCode
                font.pixelSize: 11
                elide: Text.ElideRight
                width: Math.min(implicitWidth, 200)
            }
        }
    }

    // Resize handles (right edge, bottom edge, bottom-right corner only)
    // Bottom-right corner
    MouseArea {
        width: 14; height: 14
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        cursorShape: Qt.SizeFDiagCursor
        property real sx: 0
        property real sy: 0
        property real sw: 0
        property real sh: 0
        onPressed: function(mouse) { sx = mouseX; sy = mouseY; sw = root.width; sh = root.height }
        onPositionChanged: function(mouse) {
            root.width = Math.max(root.minWidth, sw + mouseX - sx)
            root.height = Math.max(root.minHeight, sh + mouseY - sy)
        }
    }
    // Right edge
    MouseArea {
        width: 6; anchors.right: parent.right
        anchors.top: parent.top; anchors.topMargin: 34
        anchors.bottom: parent.bottom; anchors.bottomMargin: 14
        cursorShape: Qt.SizeHorCursor
        property real sx: 0
        property real sw: 0
        onPressed: function(mouse) { sx = mouseX; sw = root.width }
        onPositionChanged: function(mouse) { root.width = Math.max(root.minWidth, sw + mouseX - sx) }
    }
    // Bottom edge
    MouseArea {
        height: 6; anchors.bottom: parent.bottom
        anchors.left: parent.left; anchors.leftMargin: 6
        anchors.right: parent.right; anchors.rightMargin: 14
        cursorShape: Qt.SizeVerCursor
        property real sy: 0
        property real sh: 0
        onPressed: function(mouse) { sy = mouseY; sh = root.height }
        onPositionChanged: function(mouse) { root.height = Math.max(root.minHeight, sh + mouseY - sy) }
    }
}
