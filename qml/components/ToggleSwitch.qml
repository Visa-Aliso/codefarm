import QtQuick
import CodeFarm

Item {
    id: root
    width: 44; height: 24

    property bool checked: false
    signal toggled()

    Rectangle {
        anchors.fill: parent
        radius: 12
        color: root.checked ? Theme.btnGreen : "#555"

        Behavior on color { ColorAnimation { duration: 150 } }

        Rectangle {
            width: 18; height: 18; radius: 9
            color: "white"
            x: root.checked ? parent.width - width - 3 : 3
            anchors.verticalCenter: parent.verticalCenter
            Behavior on x { NumberAnimation { duration: 150 } }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.toggled()
    }
}
