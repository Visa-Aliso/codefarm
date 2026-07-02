import QtQuick
import CodeFarm

Item {
    id: root
    property int gridWidth: 1
    property int gridHeight: 1
    property var mapModel: null
    property int droneX: 0
    property int droneY: 0

    property real cellSize: 110
    property real perspectiveScale: 0.82
    property real perspectiveRatio: 0.25
    property real vpDepth: 0.3
    property real borderWidth: 3
    property color borderColor: "#5A7A30"
    property real cornerRadius: 12

    property real offsetX: 0
    property real offsetY: 0

    property real panX: 0
    property real panY: 0

    property int cropRefresh: 0
    property real pulsePhase: 0

    NumberAnimation on pulsePhase {
        from: 0; to: 1
        duration: 1200
        loops: Animation.Infinite
        easing.type: Easing.InOutSine
    }

    onPulsePhaseChanged: farmCanvas.requestPaint()

    signal cellClicked(int x, int y)
    signal cellHovered(int x, int y, real screenX, real screenY)
    signal cellExited()

    onGridWidthChanged: computeOffsets()
    onGridHeightChanged: computeOffsets()
    onWidthChanged: computeOffsets()
    onHeightChanged: computeOffsets()
    Component.onCompleted: computeOffsets()

    // --- Perspective helper functions ---

    function depth(t) {
        return vpDepth + (1.0 - vpDepth) * t
    }

    function cellWidth(gy) {
        return cellSize * (1 + perspectiveRatio * depth(gy / gridHeight))
    }

    function cellLeft(gx, gy) {
        return (gx - gridWidth / 2) * cellWidth(gy)
    }

    function cellRight(gx, gy) {
        return (gx + 1 - gridWidth / 2) * cellWidth(gy)
    }

    function rowY(gy) {
        var sum = 0
        for (var r = 0; r < gy; r++) {
            sum += cellSize * perspectiveScale * (1 + perspectiveRatio * depth(r / gridHeight))
        }
        return sum
    }

    // Drone cell center X and width (for GameView.qml binding)
    property real droneCellCenterX: {
        var cw = cellWidth(droneY)
        return offsetX + (droneX - gridWidth / 2 + 0.5) * cw
    }
    property real droneCellWidth: cellWidth(droneY)

    function computeOffsets() {
        var totalW = gridWidth * cellSize * (1 + perspectiveRatio)
        var totalH = rowY(gridHeight)
        offsetX = (width - totalW) / 2 + panX
        offsetY = (height - totalH) / 2 + 10 + panY
        farmCanvas.requestPaint()
    }

    function screenToGrid(mx, my) {
        var sy = my - offsetY
        var gy = 0
        for (var r = 0; r < gridHeight; r++) {
            var rh = cellSize * perspectiveScale * (1 + perspectiveRatio * depth(r / gridHeight))
            if (sy < rh) { gy = r; break }
            sy -= rh
            gy = r + 1
        }
        gy = Math.max(0, Math.min(gridHeight - 1, gy))

        var dw = cellWidth(gy)
        var gx = Math.floor((mx - offsetX) / dw + gridWidth / 2)
        gx = Math.max(0, Math.min(gridWidth - 1, gx))
        return Qt.point(gx, gy)
    }

    Canvas {
        id: farmCanvas
        anchors.fill: parent

        property int hoveredX: -1
        property int hoveredY: -1

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            if (root.gridWidth <= 0 || root.gridHeight <= 0) return

            var ox = root.offsetX
            var oy = root.offsetY
            var gw = root.gridWidth
            var gh = root.gridHeight

            // Rounded trapezoid clip path for corner rounding
            var cr = root.cornerRadius
            var tl_x = ox + root.cellLeft(0, 0)
            var tl_y = oy + root.rowY(0)
            var tr_x = ox + root.cellRight(gw - 1, 0)
            var tr_y = oy + root.rowY(0)
            var br_x = ox + root.cellRight(gw - 1, gh)
            var br_y = oy + root.rowY(gh)
            var bl_x = ox + root.cellLeft(0, gh)
            var bl_y = oy + root.rowY(gh)

            function roundedTrapPath() {
                ctx.beginPath()
                ctx.moveTo(tl_x + cr, tl_y)
                ctx.lineTo(tr_x - cr, tr_y)
                ctx.arcTo(tr_x, tr_y, tr_x, tr_y + cr, cr)
                ctx.lineTo(br_x, br_y - cr)
                ctx.arcTo(br_x, br_y, br_x - cr, br_y, cr)
                ctx.lineTo(bl_x + cr, bl_y)
                ctx.arcTo(bl_x, bl_y, bl_x, bl_y - cr, cr)
                ctx.lineTo(tl_x, tl_y + cr)
                ctx.arcTo(tl_x, tl_y, tl_x + cr, tl_y, cr)
                ctx.closePath()
            }

            // Draw cells with rounded corner clipping
            ctx.save()
            roundedTrapPath()
            ctx.clip()
            for (var gy2 = 0; gy2 < gh; gy2++) {
                for (var gx2 = 0; gx2 < gw; gx2++) {
                    var tlx2 = ox + root.cellLeft(gx2, gy2)
                    var trx2 = ox + root.cellRight(gx2, gy2)
                    var blx2 = ox + root.cellLeft(gx2, gy2 + 1)
                    var brx2 = ox + root.cellRight(gx2, gy2 + 1)
                    var tly2 = oy + root.rowY(gy2)
                    var bly2 = oy + root.rowY(gy2 + 1)
                    drawTopFace(ctx, tlx2, tly2, trx2, tly2, brx2, bly2, blx2, bly2, gx2, gy2)
                }
            }

            // Draw crop images on top of cells
            for (var cy = 0; cy < gh; cy++) {
                for (var cx = 0; cx < gw; cx++) {
                    var cell = root.mapModel ? appVm.cellAt(cx, cy) : null
                    if (!cell || !cell.crop || cell.crop === "" || cell.state === "empty" || cell.state === "tilled" || cell.state === "rock")
                        continue
                    var stage = "small"
                    if (cell.progress >= 0.8) stage = "mature"
                    else if (cell.progress >= 0.3) stage = "growing"
                    var src = "qrc:/CodeFarm/resources/sprites/crops/" + cell.crop + "_" + stage + ".png"
                    var img = root.getCropImage(src)
                    if (!img || img.status !== 1) continue // Image.Ready = 1

                    var cw = root.cellWidth(cy)
                    var rh = root.rowY(cy + 1) - root.rowY(cy)
                    var imgW = cw * 0.55
                    var imgH = rh * 0.55
                    var centerX = ox + (root.cellLeft(cx, cy) + root.cellRight(cx, cy)) / 2
                    var centerY = oy + (root.rowY(cy) + root.rowY(cy + 1)) / 2
                    // Shift center toward bottom (visual centroid of trapezoid)
                    var topW = root.cellWidth(cy)
                    var botW = root.cellWidth(cy + 1)
                    var centroidY = oy + root.rowY(cy) + rh * (2 * botW + topW) / (3 * (botW + topW))

                    // Position: bottom of sprite aligns with cell center
                    // This makes the crop's base (stem/root) sit at the cell center
                    ctx.drawImage(img, centerX - imgW / 2, centroidY - imgH + imgH * 0.25, imgW, imgH)
                }
            }

            ctx.restore()

            // Border: static rounded trapezoid outline
            roundedTrapPath()
            ctx.strokeStyle = root.borderColor
            ctx.lineWidth = root.borderWidth
            ctx.stroke()
        }

        function drawTopFace(ctx, tlx, tly, trx, try_, brx, bry, blx, bly, gx, gy) {
            var cell = root.mapModel ? appVm.cellAt(gx, gy) : null
            var state = cell ? cell.state : "empty"
            var colors = getBlockColors(state, cell)
            var isHovered = (gx === hoveredX && gy === hoveredY)

            function traceTrap() {
                ctx.beginPath()
                ctx.moveTo(tlx, tly)
                ctx.lineTo(trx, try_)
                ctx.lineTo(brx, bry)
                ctx.lineTo(blx, bly)
                ctx.closePath()
            }

            // Fill trapezoid
            traceTrap()
            ctx.fillStyle = colors.top
            ctx.fill()

            // Grid lines
            traceTrap()
            ctx.strokeStyle = "rgba(0,0,0,0.1)"
            ctx.lineWidth = 1
            ctx.stroke()

            // Top edge highlight
            ctx.beginPath()
            ctx.moveTo(tlx, tly + 0.5)
            ctx.lineTo(trx, try_ + 0.5)
            ctx.strokeStyle = "rgba(255,255,255,0.15)"
            ctx.lineWidth = 1
            ctx.stroke()

            // Grass texture dots (empty cells only)
            if (state === "empty") {
                ctx.fillStyle = "rgba(100,160,60,0.2)"
                var dots = [[0.2,0.3],[0.65,0.7],[0.8,0.2],[0.4,0.6],[0.7,0.45]]
                var cw = trx - tlx
                for (var i = 0; i < dots.length; i++) {
                    ctx.beginPath()
                    ctx.arc(tlx + cw * dots[i][0], tly + (bly - tly) * dots[i][1], 1.2, 0, Math.PI * 2)
                    ctx.fill()
                }
            }

            // Watered cells use dark brown base color (handled in getBlockColors)

            // Drone cell breathing highlight
            if (gx === root.droneX && gy === root.droneY) {
                var opacity = 0.06 + 0.14 * Math.sin(root.pulsePhase * Math.PI)
                traceTrap()
                ctx.fillStyle = "rgba(255, 255, 255," + opacity.toFixed(3) + ")"
                ctx.fill()
            }

            // Bug indicator - red tint overlay on infested cells
            if (cell && cell.hasBug) {
                traceTrap()
                ctx.fillStyle = "rgba(45, 35, 30, 0.55)"
                ctx.fill()
            }
        }

        function getBlockColors(state, cell) {
            if (state === "rock")
                return { top: Theme.blockRockTop }
            if (cell && cell.water > 0.3)
                return { top: Theme.blockWetSoilTop }
            switch (state) {
                case "tilled":
                case "planted":
                case "mature":
                case "bug":
                    return { top: Theme.blockSoilTop }
                default:
                    return { top: Theme.blockGrassTop }
            }
        }

        MouseArea {
            id: dragArea
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton

            property real pressX: 0
            property real pressY: 0
            property bool dragging: false

            onPressed: function(mouse) {
                pressX = mouse.x
                pressY = mouse.y
                dragging = false
            }

            onPositionChanged: function(mouse) {
                if (pressed) {
                    var dx = mouse.x - pressX
                    var dy = mouse.y - pressY
                    if (Math.abs(dx) > 3 || Math.abs(dy) > 3) {
                        dragging = true
                    }
                    if (dragging) {
                        root.panX += dx
                        root.panY += dy
                        // Clamp so field stays partially visible
                        var fieldW = root.gridWidth * root.cellSize * (1 + root.perspectiveRatio)
                        var fieldH = root.rowY(root.gridHeight)
                        var minVis = 100
                        root.panX = Math.max(-root.width/2 + fieldW/2 - minVis, Math.min(root.width/2 - fieldW/2 + minVis, root.panX))
                        root.panY = Math.max(-root.height/2 + fieldH/2 - minVis + 10, Math.min(root.height/2 - fieldH/2 + minVis + 10, root.panY))
                        pressX = mouse.x
                        pressY = mouse.y
                        root.computeOffsets()
                        return
                    }
                }
                var gp = root.screenToGrid(mouse.x, mouse.y)
                var gx = gp.x
                var gy = gp.y
                if (gx >= 0 && gx < root.gridWidth && gy >= 0 && gy < root.gridHeight) {
                    if (farmCanvas.hoveredX !== gx || farmCanvas.hoveredY !== gy) {
                        farmCanvas.hoveredX = gx
                        farmCanvas.hoveredY = gy
                        farmCanvas.requestPaint()
                        root.cellHovered(gx, gy, mouse.x, mouse.y)
                    }
                } else {
                    if (farmCanvas.hoveredX !== -1) {
                        farmCanvas.hoveredX = -1
                        farmCanvas.hoveredY = -1
                        farmCanvas.requestPaint()
                        root.cellExited()
                    }
                }
            }

            onReleased: function(mouse) {
                if (!dragging && farmCanvas.hoveredX >= 0) {
                    root.cellClicked(farmCanvas.hoveredX, farmCanvas.hoveredY)
                }
                dragging = false
            }

            onExited: {
                farmCanvas.hoveredX = -1
                farmCanvas.hoveredY = -1
                farmCanvas.requestPaint()
                root.cellExited()
            }
        }
    }

    // Crop image cache (loaded once, drawn on canvas)
    property var cropImageCache: ({})

    function getCropImage(src) {
        if (cropImageCache[src]) return cropImageCache[src]
        var img = Qt.createQmlObject('import QtQuick; Image { source: "' + src + '"; visible: false; width: 64; height: 64 }', root)
        img.statusChanged.connect(function() { if (img.status === Image.Ready) farmCanvas.requestPaint() })
        cropImageCache[src] = img
        return img
    }

    Connections {
        target: farmMap
        function onCellChanged() { farmCanvas.requestPaint(); root.cropRefresh++ }
        function onDimensionsChanged() { computeOffsets() }
    }
}
