import QtQuick
import CodeFarm

Item {
    id: backdrop
    property bool showParticles: true
    property color topColor: Theme.bgGradientStart
    property color bottomColor: Theme.bgGradientEnd
    property color sunColor: Theme.horizonGlow
    property color hillColor: Theme.fieldGreen
    property color hillShade: Theme.fieldGreenDeep

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: backdrop.topColor }
            GradientStop { position: 1.0; color: backdrop.bottomColor }
        }
    }

    Rectangle {
        id: sunDisc
        width: parent.width * 0.52
        height: width
        radius: width / 2
        x: parent.width * 0.60
        y: -height * 0.28
        color: Qt.rgba(backdrop.sunColor.r, backdrop.sunColor.g, backdrop.sunColor.b, 0.62)

        SequentialAnimation on scale {
            loops: Animation.Infinite
            NumberAnimation { to: 1.03; duration: 4800; easing.type: Easing.InOutQuad }
            NumberAnimation { to: 1.0; duration: 4800; easing.type: Easing.InOutQuad }
        }
    }

    Repeater {
        model: [
            { w: 0.22, h: 0.10, x: 0.12, y: 0.12, alpha: 0.22, drift: 34, duration: 12000 },
            { w: 0.16, h: 0.07, x: 0.48, y: 0.16, alpha: 0.18, drift: 24, duration: 10000 },
            { w: 0.20, h: 0.09, x: 0.72, y: 0.10, alpha: 0.14, drift: 28, duration: 13000 }
        ]

        Rectangle {
            required property var modelData
            width: parent.width * modelData.w
            height: parent.height * modelData.h
            radius: height / 2
            x: parent.width * modelData.x
            y: parent.height * modelData.y
            color: Qt.rgba(1, 1, 1, modelData.alpha)

            SequentialAnimation on x {
                loops: Animation.Infinite
                NumberAnimation { to: parent.width * modelData.x + modelData.drift; duration: modelData.duration; easing.type: Easing.InOutQuad }
                NumberAnimation { to: parent.width * modelData.x; duration: modelData.duration; easing.type: Easing.InOutQuad }
            }
        }
    }

    Repeater {
        model: 5

        Rectangle {
            required property int index
            width: parent.width * 1.2
            height: 2
            x: -parent.width * 0.1
            y: parent.height * (0.18 + index * 0.08)
            rotation: -6
            color: Qt.rgba(1, 1, 1, 0.07)
        }
    }

    Rectangle {
        width: parent.width * 1.2
        height: parent.height * 0.40
        x: -parent.width * 0.08
        y: parent.height * 0.65
        radius: height / 2
        color: backdrop.hillShade
        opacity: 0.16
    }

    Rectangle {
        width: parent.width * 1.18
        height: parent.height * 0.32
        x: -parent.width * 0.06
        y: parent.height * 0.72
        radius: height / 2
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: Qt.lighter(backdrop.hillColor, 1.05) }
            GradientStop { position: 1.0; color: backdrop.hillShade }
        }
    }

    Repeater {
        model: 6

        Rectangle {
            required property int index
            width: parent.width * 0.44
            height: 2
            x: -20
            y: parent.height * (0.73 + index * 0.035)
            rotation: -8
            color: Qt.rgba(1, 1, 1, 0.10 - index * 0.01)
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: parent.height * 0.08
        color: Qt.rgba(0.10, 0.17, 0.11, 0.05)
    }

    AmbientParticles {
        anchors.fill: parent
        visible: backdrop.showParticles
        opacity: backdrop.showParticles ? 1 : 0
    }
}
