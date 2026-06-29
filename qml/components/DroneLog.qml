import QtQuick
import CodeFarm

Rectangle {
    id: logPanel
    readonly property var engine: gameEngine
    radius: Theme.radiusLarge
    color: Qt.rgba(1, 1, 1, 0.94)
    border.width: 1
    border.color: Theme.borderStrong
    clip: true

    Column {
        anchors.fill: parent
        anchors.margins: 0
        spacing: 0

        Rectangle {
            width: parent.width
            height: 52
            color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.08)
            border.width: 1
            border.color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.10)

            Row {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 16
                spacing: 10

                Rectangle {
                    width: 10
                    height: 10
                    radius: 5
                    color: Theme.mint
                }

                Column {
                    spacing: 0

                    Text {
                        text: "FLIGHT LOG"
                        font.family: Theme.fontCode
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        color: Theme.textSecondary
                    }

                    Text {
                        text: "无人机执行与脚本反馈"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        font.weight: Font.Bold
                        color: Theme.textPrimary
                    }
                }
            }
        }

        ListView {
            id: logList
            width: parent.width
            height: parent.height - 52
            clip: true
            spacing: 6
            leftMargin: 12
            rightMargin: 12
            topMargin: 10
            bottomMargin: 10
            model: ListModel { id: logModel }
            delegate: Rectangle {
                id: logEntry
                required property int index
                required property string message
                required property string messageColor

                width: logList.width - logList.leftMargin - logList.rightMargin
                height: Math.max(34, logText.implicitHeight + 14)
                radius: 14
                color: logEntry.index % 2 === 0 ? Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.05)
                                                : Qt.rgba(Theme.secondaryBlue.r, Theme.secondaryBlue.g, Theme.secondaryBlue.b, 0.04)
                border.width: 1
                border.color: Theme.border

                Rectangle {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 4
                    radius: 2
                    color: logEntry.messageColor || Theme.textSecondary
                }

                Text {
                    id: logText
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 14
                    anchors.rightMargin: 12
                    text: logEntry.message
                    font.family: Theme.fontCode
                    font.pixelSize: 12
                    color: Theme.textPrimary
                    wrapMode: Text.WordWrap
                }
            }

            Text {
                anchors.centerIn: parent
                text: "等待脚本运行..."
                visible: logModel.count === 0
                font.family: Theme.fontCode
                font.pixelSize: 12
                color: Theme.textDisabled
            }
        }
    }

    Connections {
        target: logPanel.engine
        function onCurrentLevelChanged() {
            logModel.clear()
        }
        function onLogMessage(text, color) {
            logModel.append({message: text, messageColor: color})
            logList.positionViewAtEnd()
        }
    }
}
