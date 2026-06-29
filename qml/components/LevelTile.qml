import QtQuick
import CodeFarm

Item {
    id: root
    width: 160
    height: 120

    property int levelId: 0
    property string levelName: ""
    property int stars: 0
    property int bestTime: -1
    property string status: "locked"
    property int gridW: 0
    property int gridH: 0

    signal clicked()

    opacity: status === "locked" ? 0.4 : 1.0

    Rectangle {
        anchors.fill: parent
        radius: 8
        color: {
            if (root.status === "completed") return "#2A4A3A"
            if (root.status === "unlocked") return "#3A4A5C"
            return "#2A3040"
        }
        border.width: root.status === "unlocked" ? 2 : 1
        border.color: root.status === "unlocked" ? Theme.btnGreen : Theme.borderDim

        Behavior on border.color { ColorAnimation { duration: 150 } }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 6

        Text {
            text: "LEVEL " + root.levelId
            color: Theme.textDim
            font.family: Theme.fontCode
            font.pixelSize: 11
            font.weight: Font.Medium
        }

        Text {
            text: root.levelName
            color: Theme.textLight
            font.family: Theme.fontUI
            font.pixelSize: 14
            font.weight: Font.DemiBold
            elide: Text.ElideRight
            width: parent.width
        }

        Text {
            text: root.gridW + "×" + root.gridH
            color: Theme.textDim
            font.family: Theme.fontCode
            font.pixelSize: 11
            visible: root.status !== "locked"
        }

        Item { width: 1; height: 4 }

        Row {
            spacing: 4
            visible: root.status !== "locked"
            Repeater {
                model: 3
                Text {
                    text: "★"
                    color: index < root.stars ? Theme.starGold : Theme.textMuted
                    font.pixelSize: 16
                }
            }
        }

        Text {
            text: root.bestTime > 0 ? root.bestTime + "s" : ""
            color: Theme.textDim
            font.family: Theme.fontCode
            font.pixelSize: 11
            visible: root.bestTime > 0
        }

        Text {
            text: "🔒"
            font.pixelSize: 24
            anchors.horizontalCenter: parent.horizontalCenter
            visible: root.status === "locked"
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: root.status !== "locked" ? Qt.PointingHandCursor : Qt.ArrowCursor
        enabled: root.status !== "locked"
        onClicked: root.clicked()
    }

    scale: {
        if (root.status === "locked") return 1.0
        return mouseArea.containsMouse ? 1.03 : 1.0
    }
    Behavior on scale { NumberAnimation { duration: 150 } }
}
