import QtQuick
import CodeFarm

Rectangle {
    id: btn
    property bool primary: false
    property string text: ""
    property bool compact: false
    signal clicked()

    width: compact ? 132 : Theme.btnWidth
    height: compact ? 42 : Theme.btnHeight
    radius: height / 2
    color: "transparent"
    border.width: 0

    readonly property bool hovering: mouseArea.containsMouse
    readonly property bool pressed: mouseArea.pressed

    scale: pressed ? 0.988 : (hovering ? 1.014 : 1.0)
    y: pressed ? 1 : 0

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: btn.compact ? 3 : 4
        radius: parent.radius
        color: btn.primary ? Qt.rgba(0.22, 0.37, 0.29, 0.16) : Qt.rgba(0.18, 0.22, 0.18, 0.08)
        visible: !btn.pressed
    }

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0.0
                color: btn.primary
                       ? (btn.hovering ? Theme.primaryGreenHover : Theme.primaryGreenLight)
                       : (btn.hovering ? "#FFFFFF" : Theme.surfaceRaised)
            }
            GradientStop {
                position: 1.0
                color: btn.primary
                       ? Theme.primaryGreen
                       : "#F7FBF8"
            }
        }
        border.width: 1
        border.color: btn.primary
                      ? Qt.rgba(1, 1, 1, 0.12)
                      : (btn.hovering ? Theme.borderFocused : Theme.borderStrong)
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: btn.compact ? 6 : 8
        height: btn.compact ? 8 : 10
        radius: height / 2
        color: Qt.rgba(1, 1, 1, btn.primary ? 0.20 : 0.76)
        opacity: btn.pressed ? 0.0 : (btn.hovering ? 0.86 : 1.0)
    }

    Text {
        anchors.centerIn: parent
        text: btn.text
        font.family: Theme.fontUI
        font.pixelSize: btn.compact ? 14 : 15
        font.weight: Font.DemiBold
        color: btn.primary ? Theme.textOnDark : Theme.textPrimary
        style: Text.Normal
    }

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: btn.primary
               ? Qt.rgba(1, 1, 1, btn.hovering ? 0.08 : 0.0)
               : Qt.rgba(Theme.primaryGreen.r, Theme.primaryGreen.g, Theme.primaryGreen.b, btn.hovering ? 0.05 : 0.0)
        border.width: 0
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: btn.clicked()
    }

    Behavior on scale {
        NumberAnimation {
            duration: 120
            easing.type: Easing.OutQuad
        }
    }

    Behavior on y {
        NumberAnimation {
            duration: 90
            easing.type: Easing.OutQuad
        }
    }
}
