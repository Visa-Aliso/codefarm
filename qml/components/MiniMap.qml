import QtQuick
import CodeFarm

Rectangle {
    id: root
    width: 90
    height: 90
    radius: 8
    color: Theme.panelBg
    border.width: 1
    border.color: Theme.borderDim

    property int gridWidth: 1
    property int gridHeight: 1
    property int droneX: 0
    property int droneY: 0
    property var mapModel: null

    Canvas {
        id: miniCanvas
        anchors.fill: parent
        anchors.margins: 8

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            var cellW = width / root.gridWidth
            var cellH = height / root.gridHeight

            for (var gy = 0; gy < root.gridHeight; gy++) {
                for (var gx = 0; gx < root.gridWidth; gx++) {
                    var cell = appVm.cellAt(gx, gy)
                    var color = "#666666"
                    if (cell) {
                        switch (cell.state) {
                            case 0: color = Theme.tileEmpty; break
                            case 1: color = Theme.tileTilled; break
                            case 2: color = Theme.tilePlanted; break
                            case 3: color = Theme.tileMature; break
                            case 4: color = Theme.tileBug; break
                        }
                    }
                    ctx.fillStyle = color
                    ctx.fillRect(gx * cellW + 1, gy * cellH + 1, cellW - 2, cellH - 2)
                }
            }

            // Drone position
            ctx.fillStyle = "#5AAFCF"
            ctx.beginPath()
            ctx.arc(root.droneX * cellW + cellW/2, root.droneY * cellH + cellH/2, 3, 0, Math.PI * 2)
            ctx.fill()
        }
    }

    Connections {
        target: farmMap
        function onCellChanged() { miniCanvas.requestPaint() }
    }

    Connections {
        target: appVm
        function onRuntimeChanged() { miniCanvas.requestPaint() }
    }
}
