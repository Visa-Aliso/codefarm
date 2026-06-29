import QtQuick
import CodeFarm

Item {
    id: tile
    property string cellState: "empty"
    property string crop: ""
    property real progress: 0.0
    property real water: 0.0
    property bool fertilized: false
    property bool hasBug: false
    property bool selected: false
    property bool hovered: false

    onCellStateChanged: diamond.requestPaint()
    onCropChanged: diamond.requestPaint()
    onProgressChanged: diamond.requestPaint()
    onWaterChanged: diamond.requestPaint()
    onFertilizedChanged: diamond.requestPaint()
    onHasBugChanged: diamond.requestPaint()
    onSelectedChanged: diamond.requestPaint()
    onHoveredChanged: diamond.requestPaint()

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        width: parent.width * 0.66
        height: 10
        radius: 5
        color: Qt.rgba(0, 0, 0, 0.14)
    }

    Canvas {
        id: diamond
        anchors.fill: parent
        renderTarget: Canvas.Image

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var w = width
            var h = height
            var topY = 4
            var midY = Math.round(h * 0.34)
            var bottomY = Math.round(h * 0.60)
            var depth = 13
            var leftX = 2
            var rightX = w - 2
            var centerX = w / 2

            function fillDiamond(fillColor, strokeColor, lineWidth) {
                ctx.beginPath()
                ctx.moveTo(centerX, topY)
                ctx.lineTo(rightX, midY)
                ctx.lineTo(centerX, bottomY)
                ctx.lineTo(leftX, midY)
                ctx.closePath()
                ctx.fillStyle = fillColor
                ctx.fill()
                ctx.lineWidth = lineWidth
                ctx.strokeStyle = strokeColor
                ctx.stroke()
            }

            function fillSide(points, fillColor) {
                ctx.beginPath()
                ctx.moveTo(points[0][0], points[0][1])
                ctx.lineTo(points[1][0], points[1][1])
                ctx.lineTo(points[2][0], points[2][1])
                ctx.lineTo(points[3][0], points[3][1])
                ctx.closePath()
                ctx.fillStyle = fillColor
                ctx.fill()
            }

            var groundColor = "#D6C59A"
            if (tile.cellState === "tilled") groundColor = "#AA7556"
            else if (tile.cellState === "planted") groundColor = "#7EA45E"
            else if (tile.cellState === "mature") groundColor = "#679952"
            else if (tile.cellState === "bug") groundColor = "#B86B57"

            fillSide([[centerX, bottomY], [rightX, midY], [rightX, midY + depth], [centerX, bottomY + depth]],
                     Qt.darker(groundColor, 1.36))
            fillSide([[leftX, midY], [centerX, bottomY], [centerX, bottomY + depth], [leftX, midY + depth]],
                     Qt.darker(groundColor, 1.18))

            fillDiamond(
                        tile.water > 0.6 && tile.cellState !== "empty"
                        ? Qt.lighter(groundColor, 1.08)
                        : groundColor,
                        tile.selected ? "#F7E3B1" : (tile.hovered ? "#D7E7C8" : Qt.rgba(0, 0, 0, 0.15)),
                        tile.selected ? 2.6 : (tile.hovered ? 1.8 : 1.0)
                        )

            if (tile.water > 0 && tile.cellState !== "empty") {
                ctx.beginPath()
                ctx.moveTo(centerX, topY)
                ctx.lineTo(rightX, midY)
                ctx.lineTo(centerX, bottomY)
                ctx.lineTo(leftX, midY)
                ctx.closePath()
                ctx.fillStyle = Qt.rgba(0.38, 0.62, 0.88, Math.min(0.22, tile.water * 0.18))
                ctx.fill()
            }

            if (tile.cellState !== "empty") {
                ctx.strokeStyle = Qt.rgba(1, 1, 1, 0.12)
                ctx.lineWidth = 1
                ctx.beginPath()
                ctx.moveTo(centerX, topY + 8)
                ctx.lineTo(rightX - 10, midY)
                ctx.moveTo(centerX, topY + 8)
                ctx.lineTo(leftX + 10, midY)
                ctx.stroke()
            }

            if (tile.crop !== "" && tile.cellState !== "empty") {
                var growth = tile.cellState === "mature" ? 1.0 : Math.max(0.15, tile.progress)
                var plantBaseY = midY + 2
                var stemHeight = 8 + growth * 10
                ctx.lineCap = "round"
                ctx.lineJoin = "round"

                if (growth < 0.24) {
                    ctx.strokeStyle = "#477A45"
                    ctx.lineWidth = 2
                    ctx.beginPath()
                    ctx.moveTo(centerX, plantBaseY + 4)
                    ctx.lineTo(centerX, plantBaseY - stemHeight * 0.35)
                    ctx.stroke()
                    ctx.beginPath()
                    ctx.moveTo(centerX, plantBaseY - 2)
                    ctx.lineTo(centerX - 4, plantBaseY - 6)
                    ctx.moveTo(centerX, plantBaseY - 1)
                    ctx.lineTo(centerX + 4, plantBaseY - 5)
                    ctx.stroke()
                } else if (tile.crop === "wheat") {
                    for (var i = -2; i <= 2; i++) {
                        ctx.strokeStyle = "#547641"
                        ctx.lineWidth = 2
                        ctx.beginPath()
                        ctx.moveTo(centerX + i * 3, plantBaseY + 4)
                        ctx.lineTo(centerX + i * 2, plantBaseY - stemHeight)
                        ctx.stroke()

                        ctx.fillStyle = "#E4C56A"
                        ctx.beginPath()
                        ctx.arc(centerX + i * 2 + 2, plantBaseY - stemHeight + 2, 1.6 + growth, 0, Math.PI * 2)
                        ctx.fill()
                    }
                } else if (tile.crop === "carrot") {
                    ctx.fillStyle = "#CB7C45"
                    ctx.beginPath()
                    ctx.moveTo(centerX, plantBaseY + 5)
                    ctx.lineTo(centerX - 4, plantBaseY - 1)
                    ctx.lineTo(centerX + 4, plantBaseY - 1)
                    ctx.closePath()
                    ctx.fill()

                    ctx.strokeStyle = "#4B7D46"
                    ctx.lineWidth = 2
                    ctx.beginPath()
                    ctx.moveTo(centerX, plantBaseY - 1)
                    ctx.lineTo(centerX, plantBaseY - stemHeight * 0.75)
                    ctx.moveTo(centerX, plantBaseY - 2)
                    ctx.lineTo(centerX - 5, plantBaseY - stemHeight * 0.45)
                    ctx.moveTo(centerX, plantBaseY - 2)
                    ctx.lineTo(centerX + 5, plantBaseY - stemHeight * 0.45)
                    ctx.stroke()
                } else if (tile.crop === "tomato") {
                    ctx.strokeStyle = "#4E7B46"
                    ctx.lineWidth = 2
                    ctx.beginPath()
                    ctx.moveTo(centerX, plantBaseY + 4)
                    ctx.lineTo(centerX, plantBaseY - stemHeight)
                    ctx.moveTo(centerX, plantBaseY - stemHeight * 0.6)
                    ctx.lineTo(centerX - 6, plantBaseY - stemHeight * 0.2)
                    ctx.moveTo(centerX, plantBaseY - stemHeight * 0.58)
                    ctx.lineTo(centerX + 6, plantBaseY - stemHeight * 0.16)
                    ctx.stroke()

                    ctx.fillStyle = "#D45E55"
                    ctx.beginPath()
                    ctx.arc(centerX - 5, plantBaseY - stemHeight * 0.06, 3 + growth * 2.2, 0, Math.PI * 2)
                    ctx.arc(centerX + 5, plantBaseY + 1, 3 + growth * 2.1, 0, Math.PI * 2)
                    ctx.fill()
                } else if (tile.crop === "corn") {
                    ctx.strokeStyle = "#4F7E42"
                    ctx.lineWidth = 2
                    ctx.beginPath()
                    ctx.moveTo(centerX, plantBaseY + 4)
                    ctx.lineTo(centerX, plantBaseY - stemHeight)
                    ctx.moveTo(centerX, plantBaseY - stemHeight * 0.15)
                    ctx.lineTo(centerX - 6, plantBaseY - stemHeight * 0.55)
                    ctx.moveTo(centerX, plantBaseY - stemHeight * 0.2)
                    ctx.lineTo(centerX + 6, plantBaseY - stemHeight * 0.65)
                    ctx.stroke()

                    ctx.fillStyle = "#E0BF59"
                    ctx.fillRect(centerX - 2, plantBaseY - stemHeight * 0.42, 4, 8 + growth * 4)
                } else if (tile.crop === "sunflower") {
                    ctx.strokeStyle = "#547941"
                    ctx.lineWidth = 2.2
                    ctx.beginPath()
                    ctx.moveTo(centerX, plantBaseY + 4)
                    ctx.lineTo(centerX, plantBaseY - stemHeight)
                    ctx.stroke()

                    var petals = 8
                    var flowerY = plantBaseY - stemHeight - 2
                    for (var p = 0; p < petals; p++) {
                        var angle = (Math.PI * 2 / petals) * p
                        ctx.fillStyle = "#E0B650"
                        ctx.beginPath()
                        ctx.arc(centerX + Math.cos(angle) * 6, flowerY + Math.sin(angle) * 6, 2.4, 0, Math.PI * 2)
                        ctx.fill()
                    }
                    ctx.fillStyle = "#7A5639"
                    ctx.beginPath()
                    ctx.arc(centerX, flowerY, 4.2, 0, Math.PI * 2)
                    ctx.fill()
                }
            }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 8
        width: parent.width * 0.62
        height: 5
        radius: 3
        visible: tile.cellState === "planted"
        color: Qt.rgba(1, 1, 1, 0.18)

        Rectangle {
            width: parent.width * tile.progress
            height: parent.height
            radius: parent.radius
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#AED97A" }
                GradientStop { position: 1.0; color: "#6AA958" }
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 8
        anchors.topMargin: 14
        width: 16
        height: 16
        radius: 8
        visible: tile.fertilized
        color: Theme.starGold

        Text {
            anchors.centerIn: parent
            text: "F"
            font.family: Theme.fontCode
            font.pixelSize: 10
            font.weight: Font.Bold
            color: "#5E411A"
        }
    }

    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.rightMargin: 8
        anchors.topMargin: 12
        width: 18
        height: 18
        radius: 9
        visible: tile.hasBug
        color: Theme.danger

        Text {
            anchors.centerIn: parent
            text: "!"
            font.family: Theme.fontCode
            font.pixelSize: 12
            font.weight: Font.Bold
            color: Theme.textOnDark
        }
    }
}
