import QtQuick
import CodeFarm
import QtQuick.Layouts

FloatingPanel {
    id: goalPanel
    readonly property var engine: gameEngine

    title: "Mission Board"
    subtitle: "本关目标与星级进度"
    accentColor: Theme.fieldGold

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Repeater {
            model: goalPanel.engine.goals

            delegate: Rectangle {
                id: goalCard
                required property var modelData
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                radius: 18
                color: goalCard.modelData.completed
                       ? Qt.rgba(Theme.success.r, Theme.success.g, Theme.success.b, 0.10)
                       : Qt.rgba(1, 1, 1, 0.45)
                border.width: 1
                border.color: goalCard.modelData.completed
                              ? Qt.rgba(Theme.success.r, Theme.success.g, Theme.success.b, 0.24)
                              : Theme.border

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 10

                    Rectangle {
                        Layout.preferredWidth: 28
                        Layout.preferredHeight: 28
                        radius: 14
                        color: goalCard.modelData.completed ? Theme.success : Qt.rgba(0.28, 0.34, 0.26, 0.12)

                        Text {
                            anchors.centerIn: parent
                            text: goalCard.modelData.completed ? "✓" : "•"
                            font.pixelSize: 14
                            font.weight: Font.Bold
                            color: goalCard.modelData.completed ? Theme.textOnDark : Theme.textSecondary
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Text {
                            text: goalCard.modelData.description || ""
                            font.family: Theme.fontUI
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                            color: Theme.textPrimary
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }

                        Text {
                            text: "%1 / %2".arg(goalCard.modelData.current || 0).arg(goalCard.modelData.target || 0)
                            font.family: Theme.fontCode
                            font.pixelSize: 11
                            color: goalCard.modelData.completed ? Theme.success : Theme.textSecondary
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 24
                        radius: 12
                        color: Qt.rgba(Theme.starGold.r, Theme.starGold.g, Theme.starGold.b, 0.16)
                        border.width: 1
                        border.color: Qt.rgba(Theme.starGold.r, Theme.starGold.g, Theme.starGold.b, 0.26)

                        Text {
                            anchors.centerIn: parent
                            text: "★" + (goalCard.modelData.starTier || 1)
                            font.family: Theme.fontCode
                            font.pixelSize: 11
                            color: Theme.starGold
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
