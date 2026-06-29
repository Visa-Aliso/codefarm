import QtQuick
import CodeFarm

Item {
    id: root
    property int gridWidth: 1
    property int gridHeight: 1
    property var mapModel: null
    property int droneX: 0
    property int droneY: 0
    property real tileW: 96
    property real tileH: 48
    property real zoomLevel: 1.0
    property real panX: 0
    property real panY: 0

    signal cellClicked(int x, int y)
    signal cellHovered(int x, int y, real screenX, real screenY)
    signal cellExited()

    function toScreen(gx, gy) {
        var sx = (gx - gy) * tileW / 2
        var sy = (gx + gy) * tileH / 2
        return Qt.point(sx, sy)
    }

    function toGrid(sx, sy) {
        var gx = (sx / (tileW / 2) + sy / (tileH / 2)) / 2
        var gy = (sy / (tileH / 2) - sx / (tileW / 2)) / 2
        return Qt.point(Math.floor(gx), Math.floor(gy))
    }

    Item {
        id: mapContainer
        anchors.centerIn: parent
        scale: root.zoomLevel
        x: root.panX
        y: root.panY

        Behavior on scale { NumberAnimation { duration: 150 } }

        Canvas {
            id: mapCanvas
            width: (root.gridWidth + root.gridHeight) * root.tileW / 2 + root.tileW
            height: (root.gridWidth + root.gridHeight) * root.tileH / 2 + root.tileH + 16
            x: -width / 2
            y: -height / 4

            property int hoveredX: -1
            property int hoveredY: -1

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)
                var ox = root.gridHeight * root.tileW / 2
                var oy = 16

                for (var gy = 0; gy < root.gridHeight; gy++) {
                    for (var gx = 0; gx < root.gridWidth; gx++) {
                        var sx = ox + (gx - gy) * root.tileW / 2
                        var sy = oy + (gx + gy) * root.tileH / 2
                        drawTile(ctx, sx, sy, gx, gy)
                    }
                }
            }

            function drawTile(ctx, sx, sy, gx, gy) {
                var cell = root.mapModel ? appVm.cellAt(gx, gy) : null
                var state = cell ? cell.state : 0
                var topColor = getTopColor(state, cell)
                var leftColor = Qt.darker(topColor, 1.3)
                var rightColor = Qt.darker(topColor, 1.15)
                var tW = root.tileW
                var tH = root.tileH
                var depth = 18
                var isHovered = (gx === hoveredX && gy === hoveredY)

                // Top face
                ctx.beginPath()
                ctx.moveTo(sx + tW/2, sy)
                ctx.lineTo(sx + tW, sy + tH/2)
                ctx.lineTo(sx + tW/2, sy + tH)
                ctx.lineTo(sx, sy + tH/2)
                ctx.closePath()
                ctx.fillStyle = topColor
                ctx.fill()

                // Left face
                ctx.beginPath()
                ctx.moveTo(sx, sy + tH/2)
                ctx.lineTo(sx + tW/2, sy + tH)
                ctx.lineTo(sx + tW/2, sy + tH + depth)
                ctx.lineTo(sx, sy + tH/2 + depth)
                ctx.closePath()
                ctx.fillStyle = leftColor
                ctx.fill()

                // Right face
                ctx.beginPath()
                ctx.moveTo(sx + tW/2, sy + tH)
                ctx.lineTo(sx + tW, sy + tH/2)
                ctx.lineTo(sx + tW, sy + tH/2 + depth)
                ctx.lineTo(sx + tW/2, sy + tH + depth)
                ctx.closePath()
                ctx.fillStyle = rightColor
                ctx.fill()

                // Hover highlight
                if (isHovered) {
                    ctx.beginPath()
                    ctx.moveTo(sx + tW/2, sy)
                    ctx.lineTo(sx + tW, sy + tH/2)
                    ctx.lineTo(sx + tW/2, sy + tH)
                    ctx.lineTo(sx, sy + tH/2)
                    ctx.closePath()
                    ctx.strokeStyle = "rgba(255,255,255,0.6)"
                    ctx.lineWidth = 2
                    ctx.stroke()
                }

                // Water overlay
                if (cell && cell.water > 0.3) {
                    ctx.beginPath()
                    ctx.moveTo(sx + tW/2, sy)
                    ctx.lineTo(sx + tW, sy + tH/2)
                    ctx.lineTo(sx + tW/2, sy + tH)
                    ctx.lineTo(sx, sy + tH/2)
                    ctx.closePath()
                    ctx.fillStyle = "rgba(90,175,207," + (cell.water * 0.2) + ")"
                    ctx.fill()
                }

                // Crop indicator
                if (cell && cell.crop > 0 && state >= 2) {
                    drawCrop(ctx, sx + tW/2, sy + tH/2 - 4, cell)
                }
            }

            function getTopColor(state, cell) {
                switch (state) {
                    case 0: return Theme.tileEmpty
                    case 1: return Theme.tileTilled
                    case 2: return Theme.tilePlanted
                    case 3: return Theme.tileMature
                    case 4: return Theme.tileBug
                    default: return Theme.tileEmpty
                }
            }

            function drawCrop(ctx, cx, cy, cell) {
                var crops = ["", "🌾", "🥕", "🍅", "🌽", "🌻"]
                var emoji = crops[cell.crop] || ""
                var size = 8 + cell.progress * 10
                ctx.font = Math.round(size) + "px serif"
                ctx.textAlign = "center"
                ctx.textBaseline = "middle"
                ctx.fillText(emoji, cx, cy)
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.MiddleButton

                property real lastX: 0
                property real lastY: 0
                property bool panning: false

                onPositionChanged: function(mouse) {
                    if (panning) {
                        root.panX += mouse.x - lastX
                        root.panY += mouse.y - lastY
                        lastX = mouse.x
                        lastY = mouse.y
                        return
                    }
                    var ox = root.gridHeight * root.tileW / 2
                    var oy = 16
                    var mx = mouse.x - ox
                    var my = mouse.y - oy
                    var gp = root.toGrid(mx, my)
                    var gx = Math.round(gp.x)
                    var gy = Math.round(gp.y)
                    if (gx >= 0 && gx < root.gridWidth && gy >= 0 && gy < root.gridHeight) {
                        if (mapCanvas.hoveredX !== gx || mapCanvas.hoveredY !== gy) {
                            mapCanvas.hoveredX = gx
                            mapCanvas.hoveredY = gy
                            mapCanvas.requestPaint()
                            root.cellHovered(gx, gy, mouse.x, mouse.y)
                        }
                    } else {
                        if (mapCanvas.hoveredX !== -1) {
                            mapCanvas.hoveredX = -1
                            mapCanvas.hoveredY = -1
                            mapCanvas.requestPaint()
                            root.cellExited()
                        }
                    }
                }

                onPressed: function(mouse) {
                    if (mouse.button === Qt.MiddleButton) {
                        panning = true
                        lastX = mouse.x
                        lastY = mouse.y
                    }
                }

                onReleased: function(mouse) {
                    if (mouse.button === Qt.MiddleButton) {
                        panning = false
                    }
                }

                onClicked: function(mouse) {
                    if (mouse.button === Qt.LeftButton && mapCanvas.hoveredX >= 0) {
                        root.cellClicked(mapCanvas.hoveredX, mapCanvas.hoveredY)
                    }
                }

                onWheel: function(wheel) {
                    var delta = wheel.angleDelta.y > 0 ? 0.1 : -0.1
                    root.zoomLevel = Math.max(0.5, Math.min(2.5, root.zoomLevel + delta))
                }

                onExited: {
                    mapCanvas.hoveredX = -1
                    mapCanvas.hoveredY = -1
                    mapCanvas.requestPaint()
                    root.cellExited()
                }
            }
        }
    }

    Connections {
        target: farmMap
        function onCellChanged() { mapCanvas.requestPaint() }
        function onDimensionsChanged() { mapCanvas.requestPaint() }
    }
}
