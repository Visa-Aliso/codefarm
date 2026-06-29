import QtQuick
import CodeFarm

Item {
    id: root
    property int gridX: 0
    property int gridY: 0
    property real cellSize: 56
    property real mapOffsetX: 0
    property real mapOffsetY: 0

    width: 36
    height: 36

    x: {
        return root.mapOffsetX + root.gridX * root.cellSize + root.cellSize / 2 - width / 2
    }
    y: {
        var base = root.mapOffsetY + root.gridY * root.cellSize + root.cellSize / 2 - height - 8
        return base + floatOffset
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
            var cx = width / 2, cy = height / 2

            // Landing gear
            ctx.strokeStyle = Theme.droneArm
            ctx.lineWidth = 1.5
            ctx.beginPath()
            ctx.moveTo(cx - 5, cy + 3); ctx.lineTo(cx - 7, cy + 8)
            ctx.moveTo(cx + 5, cy + 3); ctx.lineTo(cx + 7, cy + 8)
            ctx.stroke()

            // Body
            ctx.fillStyle = Theme.droneBody
            ctx.beginPath()
            ctx.roundedRect(cx - 9, cy - 3, 18, 12, 4, 4)
            ctx.fill()

            // Highlight
            ctx.fillStyle = Theme.droneHighlight
            ctx.beginPath()
            ctx.roundedRect(cx - 7, cy - 2, 14, 5, 3, 3)
            ctx.fill()

            // Arms
            ctx.strokeStyle = Theme.droneArm
            ctx.lineWidth = 2
            ctx.beginPath()
            ctx.moveTo(cx - 7, cy); ctx.lineTo(cx - 16, cy - 6)
            ctx.moveTo(cx + 7, cy); ctx.lineTo(cx + 16, cy - 6)
            ctx.stroke()

            // Rotors
            ctx.fillStyle = Theme.droneRotor + "55"
            ctx.beginPath()
            ctx.arc(cx - 16, cy - 6, 6, 0, Math.PI * 2)
            ctx.arc(cx + 16, cy - 6, 6, 0, Math.PI * 2)
            ctx.fill()

            // LED
            ctx.fillStyle = Theme.droneLed
            ctx.beginPath()
            ctx.arc(cx, cy + 5, 2, 0, Math.PI * 2)
            ctx.fill()
        }
    }

    // Shadow below drone
    Rectangle {
        width: 22; height: 6; radius: 3
        color: Qt.rgba(0, 0, 0, 0.18)
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height + root.floatOffset
    }
}
