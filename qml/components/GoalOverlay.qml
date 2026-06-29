import QtQuick
import CodeFarm

Rectangle {
    id: root
    width: 240
    height: goalsColumn.height + 32
    radius: 8
    color: Theme.panelBg
    border.width: 1
    border.color: Theme.borderDim

    property var goals: []

    Column {
        id: goalsColumn
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 12
        spacing: 8

        Text {
            text: "GOALS"
            color: Theme.textDim
            font.family: Theme.fontCode
            font.pixelSize: 11
            font.weight: Font.Medium
        }

        Repeater {
            model: root.goals

            Item {
                width: parent.width
                height: goalRow.height + progressBar.height + 4

                Column {
                    id: goalCol
                    width: parent.width
                    spacing: 4

                    Row {
                        id: goalRow
                        spacing: 6
                        width: parent.width

                        Text {
                            text: modelData.completed ? "✓" : "○"
                            color: modelData.completed ? Theme.statusRunning : Theme.textDim
                            font.pixelSize: 12
                        }
                        Text {
                            text: modelData.description || ""
                            color: modelData.completed ? Theme.statusRunning : Theme.textLight
                            font.family: Theme.fontUI
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            width: parent.width - 40
                        }
                    }

                    Rectangle {
                        id: progressBar
                        width: parent.width
                        height: 4
                        radius: 2
                        color: "#30FFFFFF"

                        Rectangle {
                            width: {
                                var target = modelData.target || 1
                                var current = modelData.current || 0
                                return parent.width * Math.min(1.0, current / target)
                            }
                            height: parent.height
                            radius: 2
                            color: modelData.completed ? Theme.statusRunning : Theme.btnGreen
                        }
                    }
                }
            }
        }
    }
}
