import QtQuick
import CodeFarm
import QtQuick.Layouts

Rectangle {
    id: panel
    property string title: ""
    property string subtitle: ""
    property color accentColor: Theme.primaryGreen
    property int contentPadding: 16
    default property alias content: contentItem.data

    implicitWidth: 260
    implicitHeight: title.length > 0 ? 180 : 126
    radius: 26
    color: "transparent"
    border.width: 0
    clip: false

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 8
        radius: panel.radius
        color: Theme.panelShadow
        opacity: 0.95
    }

    Rectangle {
        anchors.fill: parent
        radius: panel.radius
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.96) }
            GradientStop { position: 1.0; color: Theme.surfaceSoft }
        }
        border.width: 1
        border.color: Theme.borderStrong
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 14
        height: panel.title.length > 0 ? 48 : 14
        radius: 20
        color: Qt.rgba(1, 1, 1, panel.title.length > 0 ? 0.58 : 0.24)
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 18
        width: 42
        height: 5
        radius: 3
        color: panel.accentColor
        opacity: 0.9
        visible: panel.title.length > 0
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        height: 1
        color: Theme.panelHighlight
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: panel.contentPadding
        spacing: 10

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1
            visible: panel.title.length > 0

            Text {
                text: panel.title
                font.family: Theme.fontUI
                font.pixelSize: 16
                font.weight: Font.Bold
                color: Theme.textPrimary
            }

            Text {
                text: panel.subtitle
                font.family: Theme.fontUI
                font.pixelSize: 11
                color: Theme.textSecondary
                visible: panel.subtitle.length > 0
            }
        }

        Item {
            id: contentItem
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
