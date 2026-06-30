import QtQuick
import QtQuick.Controls
import CodeFarm

Rectangle {
    id: root
    width: parent ? parent.width : 0
    height: parent ? parent.height : 0
    color: Theme.bgMain

    property bool isSettingsPage: true

    // Back button (matches LevelSelect style)
    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 12
        anchors.leftMargin: 16
        z: 100
        width: 32; height: 32; radius: 6
        color: backMa.containsMouse ? Theme.btnGreenHover : Theme.btnGreen
        Text { anchors.centerIn: parent; text: "←"; color: "white"; font.pixelSize: 16; font.weight: Font.Bold }
        MouseArea { id: backMa; anchors.fill: parent; hoverEnabled: true; onClicked: navigator.pop() }
    }

    // Top bar (keeps geometry for Flickable anchoring + centered title)
    Item {
        id: topBar
        width: parent.width
        height: 56
        z: 10

        Text {
            anchors.centerIn: parent
            text: "设置"
            color: Theme.textLight
            font.family: Theme.fontUI
            font.pixelSize: 18
            font.weight: Font.Bold
        }
    }

    // Scrollable settings
    Flickable {
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width - 80, 460)
        contentHeight: settingsContainer.height + 40
        clip: true
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: settingsContainer
            width: parent.width
            anchors.top: parent.top
            anchors.topMargin: 24
            spacing: 24

            // ============ Section: 音频 ============
            Text { text: "音频"; color: Theme.btnGreen; font.family: Theme.fontUI; font.pixelSize: 16; font.weight: Font.Bold; bottomPadding: 8 }

            Rectangle {
                width: parent.width
                height: audioCol.height + 32
                radius: 8; color: Qt.rgba(0,0,0,0.3)

                Column {
                    id: audioCol
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.top: parent.top; anchors.margins: 16
                    spacing: 20

                    // Mute toggle
                    Item {
                        width: parent.width; height: 28
                        Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "静音"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14 }
                        ToggleSwitch {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            checked: audioManager.muted
                            onToggled: audioManager.muted = !audioManager.muted
                        }
                    }

                    // BGM track selection
                    Column {
                        width: parent.width; spacing: 8
                        Text { text: "背景曲目"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14 }
                        Row {
                            width: parent.width; spacing: 8

                            Rectangle {
                                width: (parent.width - 8) / 2; height: 32; radius: 6
                                color: audioManager.bgmTrack === "town" ? Theme.btnGreen : Qt.rgba(1,1,1,0.08)
                                border.width: 1
                                border.color: audioManager.bgmTrack === "town" ? Theme.btnGreenHover : Theme.borderDim
                                Text { anchors.centerIn: parent; text: "小镇"; color: audioManager.bgmTrack === "town" ? "white" : Theme.textDim; font.family: Theme.fontUI; font.pixelSize: 13; font.weight: Font.DemiBold }
                                MouseArea {
                                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                    onClicked: audioManager.bgmTrack = "town"
                                }
                            }
                            Rectangle {
                                width: (parent.width - 8) / 2; height: 32; radius: 6
                                color: audioManager.bgmTrack === "forest" ? Theme.btnGreen : Qt.rgba(1,1,1,0.08)
                                border.width: 1
                                border.color: audioManager.bgmTrack === "forest" ? Theme.btnGreenHover : Theme.borderDim
                                Text { anchors.centerIn: parent; text: "森林"; color: audioManager.bgmTrack === "forest" ? "white" : Theme.textDim; font.family: Theme.fontUI; font.pixelSize: 13; font.weight: Font.DemiBold }
                                MouseArea {
                                    anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                                    onClicked: audioManager.bgmTrack = "forest"
                                }
                            }
                        }
                    }

                    // BGM volume
                    Column {
                        width: parent.width; spacing: 8
                        Item {
                            width: parent.width; height: 20
                            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "背景音乐"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14 }
                            Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Math.round(bgmSlider.value) + "%"; color: Theme.textDim; font.family: Theme.fontCode; font.pixelSize: 13 }
                        }
                        SettingSlider {
                            id: bgmSlider
                            width: parent.width; from: 0; to: 100; value: audioManager.bgmVolume
                            enabled: !audioManager.muted
                            onMoved: audioManager.bgmVolume = Math.round(value)
                        }
                    }

                    // SFX volume
                    Column {
                        width: parent.width; spacing: 8
                        Item {
                            width: parent.width; height: 20
                            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "音效"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14 }
                            Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: Math.round(sfxSlider.value) + "%"; color: Theme.textDim; font.family: Theme.fontCode; font.pixelSize: 13 }
                        }
                        SettingSlider {
                            id: sfxSlider
                            width: parent.width; from: 0; to: 100; value: audioManager.sfxVolume
                            enabled: !audioManager.muted
                            onMoved: {
                                audioManager.sfxVolume = Math.round(value)
                                audioManager.playSfx("click")
                            }
                        }
                    }
                }
            }

            // ============ Section: 游戏 ============
            Text { text: "游戏"; color: Theme.btnGreen; font.family: Theme.fontUI; font.pixelSize: 16; font.weight: Font.Bold; bottomPadding: 8 }

            Rectangle {
                width: parent.width
                height: gameCol.height + 32
                radius: 8; color: Qt.rgba(0,0,0,0.3)

                Column {
                    id: gameCol
                    anchors.left: parent.left; anchors.right: parent.right
                    anchors.top: parent.top; anchors.margins: 16
                    spacing: 20

                    // Run speed
                    Column {
                        width: parent.width; spacing: 8
                        Item {
                            width: parent.width; height: 20
                            Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "运行速度"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14 }
                            Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter; text: speedSlider.value.toFixed(1) + "x"; color: Theme.textDim; font.family: Theme.fontCode; font.pixelSize: 13 }
                        }
                        SettingSlider {
                            id: speedSlider
                            width: parent.width; from: 0.1; to: 5.0; stepSize: 0.1; value: appVm.runSpeed; decimals: 1
                            onMoved: appVm.setRunSpeed(value)
                        }
                    }

                    // Auto show hint
                    Item {
                        width: parent.width; height: 28
                        Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "进入关卡自动显示提示"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14 }
                        ToggleSwitch {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            checked: appVm.autoShowHint
                            onToggled: appVm.setAutoShowHint(!appVm.autoShowHint)
                        }
                    }

                    // Background animations (clouds)
                    Item {
                        width: parent.width; height: 28
                        Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter; text: "背景云朵动画"; color: Theme.textLight; font.family: Theme.fontUI; font.pixelSize: 14 }
                        ToggleSwitch {
                            anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
                            checked: appVm.bgAnimations
                            onToggled: appVm.setBgAnimations(!appVm.bgAnimations)
                        }
                    }
                }
            }

            // ============ Bottom actions ============
            Item { width: 1; height: 8 }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 16

                MenuButton {
                    text: "恢复默认设置"
                    implicitW: 160; implicitH: 38
                    onClicked: appVm.resetUiPreferences()
                }
                MenuButton {
                    text: "重置全部进度"
                    implicitW: 160; implicitH: 38
                    onClicked: confirmDialog.show("重置全部进度", "将清空所有关卡的星级和最佳时间记录，且无法恢复。确定继续吗？")
                }
            }

            Item { width: 1; height: 16 }
        }
    }

    ConfirmDialog {
        id: confirmDialog
        anchors.fill: parent
        z: 500
        onConfirmed: appVm.resetAllProgress()
    }
}
