import QtQuick
import QtQuick.Controls
import CodeFarm

Rectangle {
    id: root
    width: parent ? parent.width : 0
    height: parent ? parent.height : 0
    color: Theme.bgMain

    // Top bar
    Rectangle {
        id: topBar
        width: parent.width
        height: 48
        color: Qt.rgba(0,0,0,0.2)
        z: 10

        Text {
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "←  返回"
            color: backMa.containsMouse ? Theme.textLight : Theme.textDim
            font.family: Theme.fontUI
            font.pixelSize: 14
            MouseArea { id: backMa; anchors.fill: parent; anchors.margins: -8; hoverEnabled: true; onClicked: navigator.pop() }
        }

        Text {
            anchors.centerIn: parent
            text: "设置"
            color: Theme.textLight
            font.family: Theme.fontUI
            font.pixelSize: 18
            font.weight: Font.Bold
        }
    }

    // Settings content
    Column {
        anchors.centerIn: parent
        width: 420
        spacing: 0

        // Section: Display
        Text { text: "显示"; color: Theme.textDim; font.family: Theme.fontUI; font.pixelSize: 12; bottomPadding: 8 }

        Rectangle {
            width: parent.width; height: settingsCol.height + 32
            radius: 10; color: Qt.rgba(1,1,1,0.05)

            Column {
                id: settingsCol
                anchors.left: parent.left; anchors.right: parent.right
                anchors.top: parent.top; anchors.margins: 16
                spacing: 20

                // Particles toggle
                Row {
                    width: parent.width
                    Text { text: "环境粒子效果"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14; anchors.verticalCenter: parent.verticalCenter }
                    Item { width: parent.width - 200; height: 1 }
                    Rectangle {
                        width: 44; height: 24; radius: 12
                        color: appVm.particlesEnabled ? Theme.btnGreen : "#555"
                        anchors.verticalCenter: parent.verticalCenter
                        Rectangle {
                            width: 18; height: 18; radius: 9
                            color: "white"
                            x: appVm.particlesEnabled ? parent.width - width - 3 : 3
                            anchors.verticalCenter: parent.verticalCenter
                            Behavior on x { NumberAnimation { duration: 150 } }
                        }
                        MouseArea { anchors.fill: parent; onClicked: appVm.setParticlesEnabled(!appVm.particlesEnabled) }
                    }
                }

                // Motion scale slider
                Column {
                    width: parent.width; spacing: 8
                    Row {
                        width: parent.width
                        Text { text: "动效强度"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14 }
                        Item { width: parent.width - 160; height: 1 }
                        Text { text: Math.round(motionSlider.value * 100) + "%"; color: Theme.textDim; font.family: Theme.fontCode; font.pixelSize: 12 }
                    }
                    Slider {
                        id: motionSlider
                        width: parent.width; from: 0; to: 1.5; value: appVm.motionScale
                        onMoved: appVm.setMotionScale(value)
                        background: Rectangle { x: motionSlider.leftPadding; y: motionSlider.topPadding + motionSlider.availableHeight/2 - 2; width: motionSlider.availableWidth; height: 4; radius: 2; color: "#444"
                            Rectangle { width: motionSlider.visualPosition * parent.width; height: parent.height; radius: 2; color: Theme.btnGreen }
                        }
                        handle: Rectangle { x: motionSlider.leftPadding + motionSlider.visualPosition * (motionSlider.availableWidth - width); y: motionSlider.topPadding + motionSlider.availableHeight/2 - 8; width: 16; height: 16; radius: 8; color: "white" }
                    }
                }

                // Font size slider
                Column {
                    width: parent.width; spacing: 8
                    Row {
                        width: parent.width
                        Text { text: "编辑器字号"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14 }
                        Item { width: parent.width - 160; height: 1 }
                        Text { text: Math.round(fontSlider.value) + "px"; color: Theme.textDim; font.family: Theme.fontCode; font.pixelSize: 12 }
                    }
                    Slider {
                        id: fontSlider
                        width: parent.width; from: 11; to: 18; value: appVm.editorFontSize; stepSize: 1
                        onMoved: appVm.setEditorFontSize(Math.round(value))
                        background: Rectangle { x: fontSlider.leftPadding; y: fontSlider.topPadding + fontSlider.availableHeight/2 - 2; width: fontSlider.availableWidth; height: 4; radius: 2; color: "#444"
                            Rectangle { width: fontSlider.visualPosition * parent.width; height: parent.height; radius: 2; color: Theme.btnGreen }
                        }
                        handle: Rectangle { x: fontSlider.leftPadding + fontSlider.visualPosition * (fontSlider.availableWidth - width); y: fontSlider.topPadding + fontSlider.availableHeight/2 - 8; width: 16; height: 16; radius: 8; color: "white" }
                    }
                }
            }
        }

        Item { width: 1; height: 24 }

        // Reset button
        MenuButton {
            text: "恢复默认设置"
            anchors.horizontalCenter: parent.horizontalCenter
            implicitW: 160; implicitH: 38
            onClicked: appVm.resetUiPreferences()
        }
    }
}
