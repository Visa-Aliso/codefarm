import QtQuick
import CodeFarm
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: settingsRoot
    readonly property var navigator: settingsRoot.StackView ? settingsRoot.StackView.view : null
    property bool introReady: false

    Component.onCompleted: introReady = true

    SceneBackdrop {
        anchors.fill: parent
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 26
        spacing: 18
        opacity: settingsRoot.introReady ? 1 : 0

        transform: Translate {
            id: settingsShift
            y: settingsRoot.introReady ? 0 : 18

            Behavior on y {
                NumberAnimation {
                    duration: 280
                    easing.type: Easing.OutCubic
                }
            }
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 220
                easing.type: Easing.OutQuad
            }
        }

        RowLayout {
            Layout.fillWidth: true

            PillButton {
                text: "← 返回"
                onClicked: {
                    if (settingsRoot.navigator) {
                        settingsRoot.navigator.pop()
                    }
                }
            }
            Item { Layout.fillWidth: true }
            Text {
                text: "设置"
                font.family: Theme.fontUI
                font.pixelSize: 30
                font.weight: Font.Bold
                color: Theme.textPrimary
            }
            Item { Layout.fillWidth: true }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            FloatingPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "当前状态"
                subtitle: "这一版尚未接入持久化设置项"
                accentColor: Theme.warning

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Text {
                        text: "当前版本没有真正需要保存的复杂设置，所以这里先保留为产品说明位。"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Text {
                        text: "后续适合补充：主题切换、字体缩放、音效开关、动画强度、编辑器字号。"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }
            }

            FloatingPanel {
                Layout.preferredWidth: 360
                Layout.fillHeight: true
                title: "Render Notes"
                subtitle: "为什么这次 UI 比之前好"
                accentColor: Theme.secondaryBlue

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Text {
                        text: "1. 主要页面有统一背景、层级和控制台风格。"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: "2. 地图和无人机不再依赖 emoji，而是用 QML/Canvas 自绘。"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        text: "3. 编辑器、日志、提示卡统一成同一套作业台语言。"
                        font.family: Theme.fontUI
                        font.pixelSize: 14
                        color: Theme.textPrimary
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }
}
