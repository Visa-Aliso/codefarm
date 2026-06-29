import QtQuick
import CodeFarm
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: editorPanel
    property int levelId: 1
    readonly property var engine: gameEngine
    readonly property var scriptsStore: saveManager

    function loadEditorText() {
        const savedCode = editorPanel.scriptsStore.loadScript(editorPanel.levelId)
        codeArea.text = savedCode.length > 0 ? savedCode : editorPanel.engine.tutorialCode
        editorPanel.engine.loadScript(codeArea.text)
    }

    Component.onCompleted: loadEditorText()
    onLevelIdChanged: loadEditorText()

    radius: Theme.radiusLarge
    color: Theme.codePanel
    border.width: 1
    border.color: Qt.rgba(1, 1, 1, 0.08)
    clip: true

    function stateText() {
        switch (editorPanel.engine.state) {
        case 0:
            return "空闲"
        case 1:
            return "运行中"
        case 2:
            return "已暂停"
        case 3:
            return "脚本错误"
        default:
            return ""
        }
    }

    function stateColor() {
        switch (editorPanel.engine.state) {
        case 1:
            return Theme.success
        case 2:
            return Theme.warning
        case 3:
            return Theme.danger
        default:
            return Theme.secondaryBlue
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 94
            gradient: Gradient {
                GradientStop { position: 0.0; color: Theme.titleBarStart }
                GradientStop { position: 1.0; color: Theme.titleBarEnd }
            }
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.06)

            ColumnLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                anchors.topMargin: 12
                anchors.bottomMargin: 12
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    ColumnLayout {
                        spacing: 0

                        Text {
                            text: "FLIGHT SCRIPT"
                            font.family: Theme.fontCode
                            font.pixelSize: 10
                            font.weight: Font.DemiBold
                            color: Theme.textSecondary
                        }

                        Text {
                            text: "level_%1.py".arg(editorPanel.levelId)
                            font.family: Theme.fontUI
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: Theme.textPrimary
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Rectangle {
                        Layout.preferredWidth: 112
                        Layout.preferredHeight: 34
                        radius: 17
                        color: Qt.rgba(editorPanel.stateColor().r,
                                       editorPanel.stateColor().g,
                                       editorPanel.stateColor().b, 0.14)
                        border.width: 1
                        border.color: Qt.rgba(editorPanel.stateColor().r,
                                              editorPanel.stateColor().g,
                                              editorPanel.stateColor().b, 0.30)

                        Text {
                            anchors.centerIn: parent
                            text: editorPanel.stateText()
                            font.family: Theme.fontUI
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            color: editorPanel.stateColor()
                        }
                    }

                    PillButton {
                        Layout.preferredWidth: 88
                        Layout.preferredHeight: 34
                        compact: true
                        primary: editorPanel.engine.state !== 1
                        text: editorPanel.engine.state === 1 ? "暂停" : "运行"
                        onClicked: {
                            if (editorPanel.engine.state === 1) {
                                editorPanel.engine.pause()
                            } else {
                                editorPanel.engine.loadScript(codeArea.text)
                                editorPanel.engine.run()
                            }
                        }
                    }

                    PillButton {
                        Layout.preferredWidth: 76
                        Layout.preferredHeight: 34
                        compact: true
                        text: "单步"
                        onClicked: {
                            editorPanel.engine.loadScript(codeArea.text)
                            editorPanel.engine.stepOnce()
                        }
                    }

                    PillButton {
                        Layout.preferredWidth: 76
                        Layout.preferredHeight: 34
                        compact: true
                        text: "重置"
                        onClicked: editorPanel.engine.reset()
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Text {
                        text: "自动保存已开启"
                        font.family: Theme.fontUI
                        font.pixelSize: 12
                        color: Theme.textSecondary
                    }

                    Rectangle {
                        Layout.preferredWidth: 1
                        Layout.preferredHeight: 16
                        color: Qt.rgba(0, 0, 0, 0.08)
                    }

                    Text {
                        text: "速度"
                        font.family: Theme.fontCode
                        font.pixelSize: 11
                        color: Theme.textSecondary
                    }

                    Slider {
                        id: speedSlider
                        Layout.fillWidth: true
                        from: 0.1
                        to: 5.0
                        value: 1.0
                        onValueChanged: editorPanel.engine.setSpeed(value)

                        background: Rectangle {
                            x: speedSlider.leftPadding
                            y: speedSlider.topPadding + speedSlider.availableHeight / 2 - height / 2
                            width: speedSlider.availableWidth
                            height: 6
                            radius: 3
                            color: Qt.rgba(0.15, 0.18, 0.14, 0.08)

                            Rectangle {
                                width: speedSlider.visualPosition * parent.width
                                height: parent.height
                                radius: parent.radius
                                gradient: Gradient {
                                    orientation: Gradient.Horizontal
                                    GradientStop { position: 0.0; color: Theme.primaryGreenLight }
                                    GradientStop { position: 1.0; color: Theme.secondaryBlue }
                                }
                            }
                        }

                        handle: Rectangle {
                            x: speedSlider.leftPadding + speedSlider.visualPosition * (speedSlider.availableWidth - width)
                            y: speedSlider.topPadding + speedSlider.availableHeight / 2 - height / 2
                            width: 18
                            height: 18
                            radius: 9
                            color: Theme.surfaceRaised
                            border.width: 2
                            border.color: Theme.primaryGreen
                        }
                    }

                    Text {
                        text: speedSlider.value.toFixed(1) + "x"
                        font.family: Theme.fontCode
                        font.pixelSize: 11
                        color: Theme.textPrimary
                    }
                }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            padding: 0

            TextArea {
                id: codeArea
                font.family: Theme.fontCode
                font.pixelSize: 14
                color: Theme.codeText
                wrapMode: TextEdit.NoWrap
                selectByMouse: true
                tabStopDistance: 28
                selectionColor: Qt.rgba(0.44, 0.63, 0.52, 0.45)
                selectedTextColor: Theme.codeText
                leftPadding: 18
                rightPadding: 18
                topPadding: 18
                bottomPadding: 18

                background: Rectangle {
                    gradient: Gradient {
                        orientation: Gradient.Vertical
                        GradientStop { position: 0.0; color: Theme.codeBg }
                        GradientStop { position: 1.0; color: Qt.darker(Theme.codeBg, 1.08) }
                    }
                }

                onTextChanged: {
                    editorPanel.scriptsStore.saveScript(editorPanel.levelId, text)
                    if (editorPanel.engine.currentLevelId === editorPanel.levelId) {
                        editorPanel.engine.loadScript(text)
                    }
                }

                SyntaxHighlighter {
                    document: codeArea.textDocument
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 38
            color: Qt.rgba(1, 1, 1, 0.03)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.04)

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 12

                Text {
                    text: editorPanel.stateText() + " · tick " + editorPanel.engine.tickCount
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    color: Theme.textMutedOnDark
                }

                Item { Layout.fillWidth: true }

                Text {
                    text: "Level " + editorPanel.levelId
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    color: Theme.textMutedOnDark
                }

                Text {
                    text: "elapsed " + editorPanel.engine.timeElapsed + "s"
                    font.family: Theme.fontCode
                    font.pixelSize: 11
                    color: Theme.textMutedOnDark
                }
            }
        }
    }

    Connections {
        target: editorPanel.engine
        function onTutorialCodeChanged() {
            if (editorPanel.engine.currentLevelId === editorPanel.levelId) {
                editorPanel.loadEditorText()
            }
        }
    }
}
