import QtQuick
import QtQuick.Controls
import CodeFarm

Rectangle {
    id: root
    width: parent ? parent.width : 0
    height: parent ? parent.height : 0
    color: Theme.bgMain

    property var levelList: appVm.levels

    // Static background decoration (painted once)
    Canvas {
        anchors.fill: parent
        opacity: 0.04
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = "#FFFFFF"
            ctx.lineWidth = 0.5
            for (var i = 0; i < 8; i++) {
                var x = 80 + i * 160
                var y = 200 + Math.sin(i * 1.2) * 80
                ctx.beginPath()
                ctx.arc(x, y, 25 + i * 5, 0, Math.PI * 2)
                ctx.stroke()
            }
        }
        Component.onCompleted: requestPaint()
    }

    // Back button (green square, top-left)
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 12
        anchors.leftMargin: 16
        z: 10
        width: 32; height: 32; radius: 6
        color: backMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
        Text { anchors.centerIn: parent; text: "←"; color: "white"; font.pixelSize: 16; font.weight: Font.Bold }
        MouseArea { id: backMa; anchors.fill: parent; hoverEnabled: true; onClicked: navigator.pop() }
    }

    // Level grid
    GridView {
        id: grid
        anchors.fill: parent
        anchors.topMargin: 60
        anchors.bottomMargin: 20
        anchors.leftMargin: 40
        anchors.rightMargin: 40
        cellWidth: 190
        cellHeight: 150
        model: root.levelList

        delegate: Item {
            width: 180
            height: 140

            LevelTile {
                anchors.centerIn: parent
                width: 170
                height: 130
                levelId: modelData.id || (index + 1)
                levelName: modelData.name || ""
                stars: modelData.stars || 0
                bestTime: modelData.bestTime || -1
                gridW: modelData.gridW || 0
                gridH: modelData.gridH || 0
                status: {
                    if (modelData.stars > 0) return "completed"
                    if (modelData.unlocked) return "unlocked"
                    return "locked"
                }
                onClicked: {
                    appVm.openLevel(modelData.id || (index + 1))
                    navigator.push(gameViewPage)
                }
            }
        }
    }
}
