import QtQuick
import CodeFarm
import QtQuick.Layouts

FloatingPanel {
    id: cellTooltip
    property int cellX: -1
    property int cellY: -1
    property var cellData: ({})
    readonly property bool hasData: cellData && cellData.state !== undefined

    width: 256
    height: 214
    title: hasData ? "地块 (%1, %2)".arg(cellX).arg(cellY) : ""
    subtitle: hasData ? "传感器回传与状态详情" : ""
    accentColor: Theme.mint
    visible: hasData

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            radius: 16
            color: Qt.rgba(1, 1, 1, 0.48)
            border.width: 1
            border.color: Theme.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                Text {
                    text: "状态"
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    color: Theme.textSecondary
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: cellTooltip.cellData.state || "empty"
                    font.family: Theme.fontUI
                    font.pixelSize: 13
                    font.weight: Font.Bold
                    color: Theme.textPrimary
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            radius: 16
            color: Qt.rgba(1, 1, 1, 0.40)
            border.width: 1
            border.color: Theme.border

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                Text {
                    text: "作物"
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    color: Theme.textSecondary
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: (cellTooltip.cellData.crop || "").length > 0 ? cellTooltip.cellData.crop : "无"
                    font.family: Theme.fontUI
                    font.pixelSize: 13
                    font.weight: Font.Bold
                    color: Theme.textPrimary
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "湿度 " + Math.round((cellTooltip.cellData.water || 0) * 100) + "%"
                font.family: Theme.fontCode
                font.pixelSize: 11
                color: Theme.textSecondary
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 8
                radius: 4
                color: Qt.rgba(0.15, 0.18, 0.14, 0.08)

                Rectangle {
                    width: parent.width * Math.max(0, Math.min(1, cellTooltip.cellData.water || 0))
                    height: parent.height
                    radius: parent.radius
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: Theme.secondaryBlue }
                        GradientStop { position: 1.0; color: "#9FC6E2" }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: "成长 " + Math.round((cellTooltip.cellData.progress || 0) * 100) + "%"
                font.family: Theme.fontCode
                font.pixelSize: 11
                color: Theme.textSecondary
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 8
                radius: 4
                color: Qt.rgba(0.15, 0.18, 0.14, 0.08)

                Rectangle {
                    width: parent.width * Math.max(0, Math.min(1, cellTooltip.cellData.progress || 0))
                    height: parent.height
                    radius: parent.radius
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: Theme.primaryGreenLight }
                        GradientStop { position: 1.0; color: Theme.fieldGreenDeep }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 42
                radius: 16
                color: Qt.rgba(Theme.starGold.r, Theme.starGold.g, Theme.starGold.b, 0.14)
                border.width: 1
                border.color: Qt.rgba(Theme.starGold.r, Theme.starGold.g, Theme.starGold.b, 0.24)

                Text {
                    anchors.centerIn: parent
                    text: (cellTooltip.cellData.fertilized || false) ? "已施肥" : "未施肥"
                    font.family: Theme.fontUI
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    color: Theme.textPrimary
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 42
                radius: 16
                color: (cellTooltip.cellData.hasBug || false)
                       ? Qt.rgba(Theme.danger.r, Theme.danger.g, Theme.danger.b, 0.16)
                       : Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.12)
                border.width: 1
                border.color: (cellTooltip.cellData.hasBug || false)
                              ? Qt.rgba(Theme.danger.r, Theme.danger.g, Theme.danger.b, 0.28)
                              : Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.18)

                Text {
                    anchors.centerIn: parent
                    text: (cellTooltip.cellData.hasBug || false) ? "虫害告警" : "状态健康"
                    font.family: Theme.fontUI
                    font.pixelSize: 12
                    font.weight: Font.DemiBold
                    color: (cellTooltip.cellData.hasBug || false) ? Theme.danger : Theme.primaryGreen
                }
            }
        }
    }
}
