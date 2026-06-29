import QtQuick
import CodeFarm

Rectangle {
    id: root
    height: 40
    color: Theme.barBg
    radius: 8

    property int timeElapsed: 0
    property int tickCount: 0
    property real energy: 0
    property real maxEnergy: 20
    property int state: 0
    property string levelName: ""

    Row {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 20

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.levelName
            color: Theme.textLight
            font.family: Theme.fontUI
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        Rectangle { width: 1; height: 20; color: Theme.borderDim; anchors.verticalCenter: parent.verticalCenter }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6
            Text { text: "⏱"; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter; color: Theme.textDim }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: formatTime(root.timeElapsed)
                color: Theme.textLight
                font.family: Theme.fontCode
                font.pixelSize: 13
            }
        }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6
            Text { text: "⚡"; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter; color: Theme.statusPaused }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: Math.round(root.energy) + "/" + Math.round(root.maxEnergy)
                color: root.energy < 5 ? Theme.statusError : Theme.textLight
                font.family: Theme.fontCode
                font.pixelSize: 13
            }
        }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6
            Text { text: "T"; font.pixelSize: 12; anchors.verticalCenter: parent.verticalCenter; color: Theme.textDim; font.family: Theme.fontCode }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: root.tickCount.toString()
                color: Theme.textLight
                font.family: Theme.fontCode
                font.pixelSize: 13
            }
        }
    }

    function formatTime(sec) {
        var m = Math.floor(sec / 60)
        var s = sec % 60
        return (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
    }
}
