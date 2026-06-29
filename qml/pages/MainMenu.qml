import QtQuick
import QtQuick.Controls
import CodeFarm

Rectangle {
    id: root
    width: parent ? parent.width : 0
    height: parent ? parent.height : 0
    color: Theme.bgMain

    // Static background pattern (painted once)
    Canvas {
        id: bgCanvas
        anchors.fill: parent
        opacity: 0.05
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = "#FFFFFF"
            ctx.lineWidth = 0.5
            var spacing = 60
            for (var x = 0; x < width + spacing; x += spacing) {
                for (var y = 0; y < height + spacing; y += spacing) {
                    ctx.beginPath()
                    ctx.moveTo(x + spacing/2, y)
                    ctx.lineTo(x + spacing, y + spacing/4)
                    ctx.lineTo(x + spacing/2, y + spacing/2)
                    ctx.lineTo(x, y + spacing/4)
                    ctx.closePath()
                    ctx.stroke()
                }
            }
        }
        Component.onCompleted: requestPaint()
    }

    // Left panel
    Column {
        anchors.left: parent.left
        anchors.leftMargin: 200
        anchors.verticalCenter: parent.verticalCenter
        spacing: 24

        // Title text
        Text {
            text: "CODE FARM"
            color: Theme.textLight
            font.family: "Fredoka One"
            font.pixelSize: 72
            font.weight: Font.DemiBold
            font.letterSpacing: 2.0
            style: Text.Raised
            styleColor: "#30000000"
        }
        Item { width: 1; height: 30 }

        Column {
            leftPadding: 80
            spacing: 16

            MenuButton {
                text: "▶  开始游戏"
                primary: true
                onClicked: navigator.push(levelSelectPage)
            }
            MenuButton {
                text: "↺  新游戏"
                onClicked: { appVm.resetAllProgress(); appVm.openLevel(1); navigator.push(gameViewPage) }
            }
            MenuButton {
                text: "?  帮助"
                onClicked: navigator.push(helpPage)
            }
            MenuButton {
                text: "✕  退出"
                onClicked: Qt.quit()
            }
        }
    }

    // Jackstraw image (right side, bottom-aligned, larger)
    Image {
        source: "qrc:/CodeFarm/resources/pictures/jackstraw.png"
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 40
        width: 580
        height: width * sourceSize.height / sourceSize.width
        fillMode: Image.PreserveAspectFit
    }

    // Version
    Text {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 16
        text: "v0.2.0"
        color: Theme.textMuted
        font.family: Theme.fontCode
        font.pixelSize: 11
    }
}