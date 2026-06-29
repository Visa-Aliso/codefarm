import QtQuick
import CodeFarm
import QtQuick.Layouts

Rectangle {
    id: card
    property int levelId: 1
    property string levelName: ""
    property bool unlocked: false
    property int stars: 0
    property int bestTime: -1

    width: 188
    height: 214
    radius: 26
    color: "transparent"
    border.width: 0
    opacity: unlocked ? 1.0 : 0.68

    signal clicked()

    readonly property bool hovering: hoverArea.containsMouse && unlocked
    y: hovering ? -6 : 0
    scale: hovering ? 1.02 : 1.0

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
            GradientStop {
                position: 0.0
                color: card.unlocked
                       ? Theme.surfaceRaised
                       : "#F4F6F1"
            }
            GradientStop {
                position: 1.0
                color: card.unlocked
                       ? Qt.rgba(Theme.mint.r, Theme.mint.g, Theme.mint.b, card.stars > 0 ? 0.22 : 0.08)
                       : "#EBEFE8"
            }
        }
        border.width: 1
        border.color: card.hovering ? Theme.borderFocused : Theme.borderStrong
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 14
        height: 8
        radius: 4
        color: card.unlocked
               ? (card.stars > 0 ? Theme.fieldGold : Theme.mint)
               : Theme.locked
        opacity: 0.95
    }

    Rectangle {
        width: 58
        height: 58
        radius: 18
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 18
        color: card.unlocked
               ? Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.12)
               : Qt.rgba(0.35, 0.37, 0.33, 0.10)
        border.width: 1
        border.color: card.unlocked ? Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, 0.18)
                                   : Theme.border

        Text {
            anchors.centerIn: parent
            text: card.unlocked ? (card.levelId < 10 ? "0" + card.levelId : String(card.levelId)) : "🔒"
            font.family: Theme.fontCode
            font.pixelSize: card.unlocked ? 18 : 24
            font.weight: Font.Bold
            color: card.unlocked ? Theme.primaryGreen : Theme.locked
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 8

        Item { Layout.preferredHeight: 48 }

        Text {
            text: card.levelName
            font.family: Theme.fontUI
            font.pixelSize: 16
            font.weight: Font.Bold
            color: Theme.textPrimary
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            visible: card.unlocked
        }

        Text {
            text: card.unlocked ? "挑战编号 %1".arg(card.levelId) : "尚未解锁"
            font.family: Theme.fontUI
            font.pixelSize: 12
            color: card.unlocked ? Theme.textSecondary : Theme.locked
        }

        Item { Layout.fillHeight: true }

        StarRating {
            count: card.stars
            visible: card.unlocked
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            radius: 16
            color: Qt.rgba(1, 1, 1, 0.70)
            border.width: 1
            border.color: Theme.border
            visible: card.unlocked

            Text {
                anchors.centerIn: parent
                text: card.bestTime > 0 ? "最佳成绩  " + card.bestTime + "s" : "首次进入"
                font.family: Theme.fontCode
                font.pixelSize: 11
                color: card.bestTime > 0 ? Theme.textPrimary : Theme.textSecondary
            }
        }
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        enabled: card.unlocked
        hoverEnabled: true
        cursorShape: card.unlocked ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: card.clicked()
    }

    Behavior on y {
        NumberAnimation {
            duration: 150
            easing.type: Easing.OutQuad
        }
    }

    Behavior on scale {
        NumberAnimation {
            duration: 150
            easing.type: Easing.OutQuad
        }
    }
}
