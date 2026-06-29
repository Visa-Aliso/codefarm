import QtQuick
import QtQuick.Controls
import CodeFarm

Item {
    id: root
    property string text: ""
    property bool primary: false
    property real implicitW: 260
    property real implicitH: 56

    signal clicked()

    width: implicitW
    height: implicitH
    opacity: enabled ? 1.0 : 0.4

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 6
        color: {
            if (!root.enabled) return Theme.btnSecondary
            if (mouseArea.pressed) return root.primary ? Theme.btnGreenPress : Theme.btnSecondary
            if (mouseArea.containsMouse) return root.primary ? Theme.btnGreenHover : Theme.btnSecondaryHover
            return root.primary ? Theme.btnGreen : Theme.btnSecondary
        }
        border.width: root.primary ? 0 : 1
        border.color: Theme.borderDim

        Behavior on color { ColorAnimation { duration: 100 } }
    }

    Text {
        anchors.centerIn: parent
        text: root.text
        color: Theme.textLight
        font.family: Theme.fontUI
        font.pixelSize: 18
        font.weight: Font.DemiBold
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        enabled: root.enabled
        onClicked: root.clicked()
    }

    scale: mouseArea.pressed ? 0.96 : 1.0
    Behavior on scale { NumberAnimation { duration: 80 } }
}
