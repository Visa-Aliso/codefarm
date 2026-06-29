import QtQuick
import CodeFarm

Item {
    id: root
    property int gridX: 0
    property int gridY: 0
    property real tileW: 64
    property real tileH: 32
    property real mapOffsetX: 0
    property real mapOffsetY: 0
    property int mapGridHeight: 1

    width: 32
    height: 32

    x: {
        var ox = mapGridHeight * tileW / 2
        return mapOffsetX + ox + (gridX - gridY) * tileW / 2 + tileW/2 - width/2
    }
    y: {
        var oy = 16
        return mapOffsetY + oy + (gridX + gridY) * tileH / 2 - height + tileH/2 - floatOffset
    }

    property real floatOffset: 0

    Behavior on x { SmoothedAnimation { velocity: 200 } }
    Behavior on y { SmoothedAnimation { velocity: 200 } }

    SequentialAnimation on floatOffset {
        loops: Animation.Infinite
        NumberAnimation { from: 0; to: 4; duration: 1000; easing.type: Easing.InOutSine }
        NumberAnimation { from: 4; to: 0; duration: 1000; easing.type: Easing.InOutSine }
    }

    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            // Body
            ctx.fillStyle = "#E8E8E8"
            ctx.beginPath()
            ctx.roundedRect(8, 12, 16, 10, 3, 3)
            ctx.fill()

            // Rotors
            ctx.fillStyle = "#555555"
            ctx.fillRect(4, 10, 10, 2)
            ctx.fillRect(18, 10, 10, 2)

            // Rotor tips
            ctx.fillStyle = "#333333"
            ctx.beginPath()
            ctx.arc(4, 11, 2, 0, Math.PI * 2)
            ctx.arc(28, 11, 2, 0, Math.PI * 2)
            ctx.fill()

            // Light
            ctx.fillStyle = "#4CAF50"
            ctx.beginPath()
            ctx.arc(16, 15, 2, 0, Math.PI * 2)
            ctx.fill()
        }
    }

    // Shadow below drone
    Rectangle {
        width: 20
        height: 8
        radius: 4
        color: Qt.rgba(0, 0, 0, 0.2)
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height + root.floatOffset
    }
}
