import QtQuick
import CodeFarm
import QtQuick.Layouts

Rectangle {
    id: topBar
    readonly property var engine: gameEngine
    signal backRequested()
    signal giveUpRequested()

    height: 74
    radius: 26
    color: "transparent"
    border.width: 0

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 8
        radius: parent.radius
        color: Theme.panelShadow
    }

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.96) }
            GradientStop { position: 1.0; color: Theme.surfaceSoft }
        }
        border.width: 1
        border.color: Theme.borderStrong
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 12

        PillButton {
            Layout.preferredWidth: 100
            Layout.preferredHeight: 38
            compact: true
            text: "← 返回"
            onClicked: topBar.backRequested()
        }

        Rectangle {
            Layout.preferredWidth: 282
            Layout.fillHeight: true
            radius: 20
            color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.07)
            border.width: 1
            border.color: Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.12)

            ColumnLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                anchors.topMargin: 10
                anchors.bottomMargin: 10
                spacing: 0

                Text {
                    text: "FIELD TASK  %1".arg(topBar.engine.currentLevelId)
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    color: Theme.textSecondary
                }

                Text {
                    text: topBar.engine.currentLevelName
                    font.family: Theme.fontUI
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    color: Theme.textPrimary
                }
            }
        }

        Item { Layout.fillWidth: true }

        Repeater {
            model: [
                { label: "TIME", value: topBar.engine.timeElapsed + "s", warning: false },
                { label: "ENERGY", value: "%1 / %2".arg(Math.round(topBar.engine.energy)).arg(Math.round(topBar.engine.maxEnergy)), warning: topBar.engine.energy < 5 },
                { label: "TICK", value: String(topBar.engine.tickCount), warning: false }
            ]

            delegate: Rectangle {
                required property var modelData
                Layout.preferredWidth: modelData.label === "ENERGY" ? 132 : 110
                Layout.preferredHeight: 44
                radius: 18
                color: Qt.rgba(1, 1, 1, 0.58)
                border.width: 1
                border.color: Theme.border

                Column {
                    anchors.centerIn: parent
                    spacing: 0

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.label
                        font.family: Theme.fontCode
                        font.pixelSize: 10
                        color: Theme.textSecondary
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.value
                        font.family: Theme.fontUI
                        font.pixelSize: 16
                        font.weight: Font.Bold
                        color: modelData.warning ? Theme.warning : Theme.textPrimary
                    }
                }
            }
        }

        PillButton {
            Layout.preferredWidth: 92
            Layout.preferredHeight: 38
            compact: true
            text: "放弃"
            onClicked: topBar.giveUpRequested()
        }

        PillButton {
            Layout.preferredWidth: 98
            Layout.preferredHeight: 38
            compact: true
            primary: true
            text: topBar.engine.state === 1 ? "暂停" : "运行"
            onClicked: topBar.engine.state === 1 ? topBar.engine.pause() : topBar.engine.run()
        }
    }
}
