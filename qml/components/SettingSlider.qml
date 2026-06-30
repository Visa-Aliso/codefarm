import QtQuick
import CodeFarm

Item {
    id: root
    width: parent ? parent.width : 200
    height: 26

    property real from: 0
    property real to: 1
    property real value: 0
    property real stepSize: 0
    property bool enabled: true
    property int decimals: 0

    signal moved(real value)

    readonly property real range: to - from
    readonly property real norm: range > 0 ? (value - from) / range : 0

    function quantize(v) {
        if (stepSize <= 0) return v
        return Math.round((v - from) / stepSize) * stepSize + from
    }

    // Track
    Rectangle {
        id: track
        anchors.verticalCenter: parent.verticalCenter
        x: handle.width / 2
        width: parent.width - handle.width
        height: 8
        radius: 4
        color: "#2A2F33"
        opacity: root.enabled ? 1.0 : 0.4

        // Filled portion
        Rectangle {
            width: parent.width * root.norm
            height: parent.height
            radius: parent.radius
            color: Theme.btnGreen
        }
    }

    // Handle
    Rectangle {
        id: handle
        width: 22; height: 22; radius: 11
        anchors.verticalCenter: parent.verticalCenter
        x: track.x + root.norm * track.width - width / 2
        color: "white"
        border.width: 2
        border.color: Theme.btnGreen
        opacity: root.enabled ? 1.0 : 0.4

        // subtle ring shadow for depth
        Rectangle {
            anchors.centerIn: parent
            width: parent.width + 2; height: parent.height + 2
            radius: parent.radius + 1
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(0,0,0,0.25)
            z: -1
        }
    }

    // Whole-track interactive area: click anywhere to jump, drag to scrub.
    MouseArea {
        id: ma
        anchors.fill: parent
        enabled: root.enabled
        cursorShape: root.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        preventStealing: true
        onPressed: function(mouse) { applyFromMouse(mouse.x) }
        onPositionChanged: function(mouse) { if (pressed) applyFromMouse(mouse.x) }

        function applyFromMouse(mx) {
            var p = mapToItem(track, mx, 0)
            var t = Math.max(0, Math.min(1, p.x / track.width))
            var raw = from + t * range
            root.value = quantize(raw)
            root.moved(root.value)
        }
    }
}
