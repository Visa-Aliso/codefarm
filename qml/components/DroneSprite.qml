pragma ComponentBehavior: Bound
import QtQuick

Item {
    id: drone
    width: 54
    height: 44

    y: hoverAnimation.running ? -1 : 0

    SequentialAnimation on y {
        id: hoverAnimation
        loops: Animation.Infinite
        NumberAnimation { to: -3; duration: 680; easing.type: Easing.InOutQuad }
        NumberAnimation { to: 1; duration: 680; easing.type: Easing.InOutQuad }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        width: 28
        height: 10
        radius: 5
        color: Qt.rgba(0, 0, 0, 0.18)
        scale: 0.95
    }

    Item {
        id: craft
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        y: -3
        width: 42
        height: 28

        Rectangle {
            anchors.centerIn: parent
            width: 34
            height: 4
            radius: 2
            rotation: 25
            color: "#5F6C73"
        }

        Rectangle {
            anchors.centerIn: parent
            width: 34
            height: 4
            radius: 2
            rotation: -25
            color: "#5F6C73"
        }

        Repeater {
            model: 4

            delegate: Item {
                required property int index
                width: 14
                height: 14
                x: index % 2 === 0 ? 1 : craft.width - width - 1
                y: index < 2 ? 0 : craft.height - height

                Item {
                    anchors.fill: parent

                    Rectangle {
                        anchors.centerIn: parent
                        width: 12
                        height: 2
                        radius: 1
                        color: "#BFD0D5"
                    }

                    Rectangle {
                        anchors.centerIn: parent
                        width: 2
                        height: 12
                        radius: 1
                        color: "#BFD0D5"
                    }

                    NumberAnimation on rotation {
                        from: 0
                        to: 360
                        duration: 180
                        loops: Animation.Infinite
                    }
                }

                Rectangle {
                    anchors.centerIn: parent
                    width: 5
                    height: 5
                    radius: 2.5
                    color: "#4C5A61"
                }
            }
        }

        Rectangle {
            anchors.centerIn: parent
            width: 18
            height: 14
            radius: 7
            color: "#D79C48"
            border.width: 1
            border.color: "#F5D7A0"
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            anchors.verticalCenterOffset: -4
            width: 22
            height: 10
            radius: 5
            color: "#33454D"
            border.width: 1
            border.color: "#75858B"
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 7
            width: 10
            height: 4
            radius: 2
            color: "#EAE1C9"
        }
    }
}
