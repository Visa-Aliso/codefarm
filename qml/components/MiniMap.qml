pragma ComponentBehavior: Bound
import QtQuick
import CodeFarm

FloatingPanel {
    id: miniMap
    required property var engine
    required property var mapModel
    property int focusX: -1
    property int focusY: -1

    width: 192
    height: 202
    title: "Field Grid"
    subtitle: "无人机与焦点位置"
    accentColor: Theme.secondaryBlue
    visible: mapModel.gridWidth > 0 && mapModel.gridHeight > 0

    function cellColor(state, crop, hasBug) {
        if (hasBug) return "#D96A6A"
        if (state === "mature") return "#7EBC68"
        if (state === "planted") {
            return crop === "corn" ? "#B7C74B"
                 : crop === "carrot" ? "#D88F54"
                 : crop === "tomato" ? "#C96F62"
                 : crop === "sunflower" ? "#D5B45A"
                 : "#A7C46A"
        }
        if (state === "tilled") return "#9B7A52"
        return "#D6D0BD"
    }

    Item {
        id: mapArea
        anchors.fill: parent

        readonly property real rawCellSize: Math.min(
                                                width / Math.max(1, miniMap.mapModel.gridWidth),
                                                height / Math.max(1, miniMap.mapModel.gridHeight)
                                            )
        readonly property real cellSize: Math.max(12, Math.floor(rawCellSize))
        readonly property real boardWidth: cellSize * miniMap.mapModel.gridWidth
        readonly property real boardHeight: cellSize * miniMap.mapModel.gridHeight

        Rectangle {
            anchors.centerIn: parent
            width: mapArea.boardWidth + 16
            height: mapArea.boardHeight + 16
            radius: 16
            color: Qt.rgba(0.10, 0.18, 0.14, 0.10)
            border.width: 1
            border.color: Qt.rgba(0.12, 0.18, 0.14, 0.10)

            Item {
                anchors.centerIn: parent
                width: mapArea.boardWidth
                height: mapArea.boardHeight

                Repeater {
                    model: miniMap.mapModel

                    delegate: Rectangle {
                        id: miniCell
                        required property var model

                        x: miniCell.model.gridX * mapArea.cellSize
                        y: miniCell.model.gridY * mapArea.cellSize
                        width: Math.max(10, mapArea.cellSize - 2)
                        height: Math.max(10, mapArea.cellSize - 2)
                        radius: 4
                        color: miniMap.cellColor(miniCell.model.state, miniCell.model.crop, miniCell.model.hasBug)
                        border.width: miniCell.model.gridX === miniMap.focusX &&
                                      miniCell.model.gridY === miniMap.focusY ? 2 : 0
                        border.color: Theme.textOnDark

                        Rectangle {
                            anchors.centerIn: parent
                            width: 6
                            height: 6
                            radius: 3
                            color: Theme.textOnDark
                            visible: miniCell.model.gridX === miniMap.engine.droneX &&
                                     miniCell.model.gridY === miniMap.engine.droneY
                        }
                    }
                }
            }
        }
    }
}
