pragma ComponentBehavior: Bound
import QtQuick
import CodeFarm

Item {
    id: mapRoot
    readonly property var engine: gameEngine
    readonly property var mapModel: farmMap

    property real tileW: 64
    property real tileH: 32
    property real zoomLevel: 1.0
    property int hoveredX: -1
    property int hoveredY: -1
    property int selectedX: -1
    property int selectedY: -1
    property var hoveredCell: ({})
    property var selectedCell: ({})
    readonly property real boardWidth: (mapRoot.mapModel.gridWidth + mapRoot.mapModel.gridHeight) * mapRoot.tileW / 2
    readonly property real boardHeight: (mapRoot.mapModel.gridWidth + mapRoot.mapModel.gridHeight) * mapRoot.tileH / 2 + 32

    function updateHoveredCell(x, y) {
        mapRoot.hoveredX = x
        mapRoot.hoveredY = y
        mapRoot.hoveredCell = x >= 0 && y >= 0 ? mapRoot.mapModel.getCellAt(x, y) : ({})
    }

    function updateSelectedCell(x, y) {
        mapRoot.selectedX = x
        mapRoot.selectedY = y
        mapRoot.selectedCell = x >= 0 && y >= 0 ? mapRoot.mapModel.getCellAt(x, y) : ({})
    }

    Rectangle {
        anchors.fill: parent
        radius: 30
        color: Qt.rgba(1, 1, 1, 0.02)
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.04)
    }

    Repeater {
        model: 5

        Rectangle {
            required property int index
            width: parent.width * 0.7
            height: 1
            x: parent.width * 0.15
            y: parent.height * (0.18 + index * 0.12)
            color: Qt.rgba(1, 1, 1, 0.06)
        }
    }

    Item {
        id: viewport
        anchors.fill: parent
        clip: true

        Scale {
            id: zoomTransform
            origin.x: viewport.width / 2
            origin.y: viewport.height / 2
            xScale: mapRoot.zoomLevel
            yScale: mapRoot.zoomLevel
        }

        Rectangle {
            width: Math.max(140, mapRoot.boardWidth * 0.72)
            height: Math.max(42, mapRoot.boardHeight * 0.20)
            radius: height / 2
            anchors.horizontalCenter: board.horizontalCenter
            y: board.y + mapRoot.boardHeight * 0.74
            color: Qt.rgba(0, 0, 0, 0.20)
            transform: [zoomTransform]
        }

        Item {
            id: board
            width: Math.max(mapRoot.tileW, mapRoot.boardWidth)
            height: Math.max(mapRoot.tileH + 24, mapRoot.boardHeight)
            anchors.horizontalCenter: parent.horizontalCenter
            y: Math.max(76, (parent.height - height) * 0.24)
            transform: [zoomTransform]

            Repeater {
                model: mapRoot.mapModel

                delegate: Item {
                    id: cellItem
                    required property var model

                    x: (model.gridX - model.gridY) * mapRoot.tileW / 2 + board.width / 2 - mapRoot.tileW / 2
                    y: (model.gridX + model.gridY) * mapRoot.tileH / 2
                    z: model.gridX + model.gridY
                    width: mapRoot.tileW
                    height: mapRoot.tileH + 24

                    FarmTile {
                        anchors.fill: parent
                        cellState: cellItem.model.state
                        crop: cellItem.model.crop
                        progress: cellItem.model.progress
                        water: cellItem.model.water
                        fertilized: cellItem.model.fertilized
                        hasBug: cellItem.model.hasBug
                        selected: mapRoot.selectedX === cellItem.model.gridX && mapRoot.selectedY === cellItem.model.gridY
                        hovered: mapRoot.hoveredX === cellItem.model.gridX && mapRoot.hoveredY === cellItem.model.gridY
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor

                        onEntered: mapRoot.updateHoveredCell(cellItem.model.gridX, cellItem.model.gridY)
                        onExited: {
                            if (mapRoot.hoveredX === cellItem.model.gridX &&
                                    mapRoot.hoveredY === cellItem.model.gridY) {
                                mapRoot.updateHoveredCell(-1, -1)
                            }
                        }
                        onClicked: mapRoot.updateSelectedCell(cellItem.model.gridX, cellItem.model.gridY)
                    }
                }
            }

            DroneSprite {
                id: droneSprite
                x: (mapRoot.engine.droneX - mapRoot.engine.droneY) * mapRoot.tileW / 2 + board.width / 2 - width / 2
                y: (mapRoot.engine.droneX + mapRoot.engine.droneY) * mapRoot.tileH / 2 - 18
                z: 5000

                Behavior on x {
                    NumberAnimation {
                        duration: 280
                        easing.type: Easing.OutQuad
                    }
                }

                Behavior on y {
                    NumberAnimation {
                        duration: 280
                        easing.type: Easing.OutQuad
                    }
                }
            }
        }
    }

    FloatingPanel {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 12
        width: 224
        height: 106
        title: "Field Scanner"
        subtitle: "滚轮缩放，点击锁定地块"
        accentColor: Theme.primaryGreen

        Column {
            anchors.fill: parent
            spacing: 6

            Text {
                text: "缩放 " + mapRoot.zoomLevel.toFixed(2) + "x"
                font.family: Theme.fontCode
                font.pixelSize: 12
                color: Theme.textPrimary
            }

            Text {
                text: mapRoot.selectedX >= 0
                      ? "选中 (%1, %2)".arg(mapRoot.selectedX).arg(mapRoot.selectedY)
                      : "当前未锁定地块"
                font.family: Theme.fontUI
                font.pixelSize: 12
                color: Theme.textSecondary
            }

            Text {
                text: "无人机 (%1, %2)".arg(mapRoot.engine.droneX).arg(mapRoot.engine.droneY)
                font.family: Theme.fontCode
                font.pixelSize: 12
                color: Theme.textPrimary
            }
        }
    }

    MiniMap {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 12
        engine: mapRoot.engine
        mapModel: mapRoot.mapModel
        focusX: mapRoot.selectedX >= 0 ? mapRoot.selectedX : mapRoot.hoveredX
        focusY: mapRoot.selectedY >= 0 ? mapRoot.selectedY : mapRoot.hoveredY
    }

    CellTooltip {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 12
        cellX: mapRoot.selectedX >= 0 ? mapRoot.selectedX : mapRoot.hoveredX
        cellY: mapRoot.selectedY >= 0 ? mapRoot.selectedY : mapRoot.hoveredY
        cellData: mapRoot.selectedX >= 0 ? mapRoot.selectedCell : mapRoot.hoveredCell
    }

    WheelHandler {
        onWheel: (event) => {
            mapRoot.zoomLevel = Math.max(0.5, Math.min(2.0, mapRoot.zoomLevel + event.angleDelta.y * 0.001))
        }
    }

    Behavior on zoomLevel {
        NumberAnimation {
            duration: 120
            easing.type: Easing.OutQuad
        }
    }

    Connections {
        target: mapRoot.mapModel
        function onDataChanged() {
            mapRoot.updateHoveredCell(mapRoot.hoveredX, mapRoot.hoveredY)
            mapRoot.updateSelectedCell(mapRoot.selectedX, mapRoot.selectedY)
        }
        function onModelReset() {
            mapRoot.updateHoveredCell(-1, -1)
            mapRoot.updateSelectedCell(-1, -1)
        }
    }
}
