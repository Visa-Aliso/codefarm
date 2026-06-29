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
            text: "游戏帮助"
            color: Theme.textLight
            font.family: Theme.fontUI
            font.pixelSize: 18
            font.weight: Font.Bold
        }
    }

    // Scrollable content
    Flickable {
        anchors.top: topBar.bottom
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width - 80, 700)
        contentHeight: helpContent.height + 60
        clip: true
        flickableDirection: Flickable.VerticalFlick

        Column {
            id: helpContent
            width: parent.width
            topPadding: 30
            spacing: 24

            // Section 1: Game Overview
            HelpSection {
                title: "游戏简介"
                content: "Code Farm 是一款编程教育游戏。你将通过编写 Python 代码控制一架无人机，在农场上完成耕地、种植、浇水、施肥和收获等任务。每个关卡有不同的目标，完成目标获得星级评价。"
            }

            // Section 2: How to Play
            HelpSection {
                title: "如何游玩"
                content: "1. 从关卡选择界面选择一个已解锁的关卡\n2. 在代码编辑器中编写 Python 脚本\n3. 点击绿色按钮（▶）运行代码\n4. 观察无人机在农场上执行你的指令\n5. 完成所有目标即可通关\n\n提示：代码每个 tick 执行一次，你可以用变量保存状态。"
            }

            // Section 3: Code Editor
            HelpSection {
                title: "代码编辑器"
                content: "游戏界面中的深色浮动窗口就是代码编辑器。\n\n• 绿色圆点 ▶ — 运行/继续代码\n• 黄色圆点 ⏸ — 暂停执行\n• 红色圆点 ■ — 停止并重置\n• → 按钮 — 单步执行一个 tick\n• ⟲ 按钮 — 重置关卡\n\n编辑器支持 Python 语法高亮，当前执行行会以绿色标记。\n你可以拖拽标题栏移动窗口，拖拽右下角调整大小。"
            }

            // Section 4: API Reference
            HelpSection {
                title: "基础指令"
                content: "move(方向) — 移动无人机\n  方向: \"up\" / \"down\" / \"left\" / \"right\"\n  消耗: 1 能量\n\ntill() — 开垦当前格子\n  消耗: 2 能量\n\nplant(作物) — 种植作物\n  作物: \"wheat\" / \"carrot\" / \"tomato\" / \"corn\" / \"sunflower\"\n  消耗: 1.5 能量\n\nharvest() — 收割成熟作物\n  消耗: 1 能量"
            }

            HelpSection {
                title: "进阶指令"
                content: "water() — 浇水 (关卡3解锁)\n  消耗: 1.5 能量\n\nfertilize() — 施肥 (关卡5解锁)\n  消耗: 2 能量\n\ndebug() — 修复虫害 (关卡2解锁)\n  消耗: 3 能量\n\nget_pos() — 获取无人机坐标\nget_current() — 获取当前格子信息\nget_energy() — 获取剩余能量"
            }

            HelpSection {
                title: "游戏机制"
                content: "• 能量：每次操作消耗能量，每 tick 恢复 0.5\n• 水分：作物需要水分才能生长，每 tick 自然蒸发\n• 虫害：随机出现，不处理会导致作物死亡\n• 施肥：加速作物生长 1.5 倍，持续 30 tick\n• 星级：完成主目标得 1 星，额外目标得 2-3 星"
            }

            Item { width: 1; height: 30 }
        }
    }

    // Inline component for help sections
    component HelpSection: Column {
        property string title: ""
        property string content: ""
        width: parent.width
        spacing: 8

        Text {
            text: title
            color: Theme.btnGreen
            font.family: Theme.fontUI
            font.pixelSize: 16
            font.weight: Font.Bold
        }
        Rectangle {
            width: parent.width; height: contentText.height + 20
            radius: 8; color: Qt.rgba(0,0,0,0.3)
            Text {
                id: contentText
                anchors.left: parent.left; anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 14
                text: content
                color: Theme.textDim
                font.family: Theme.fontCode
                font.pixelSize: 13
                lineHeight: 1.5
                wrapMode: Text.WordWrap
            }
        }
    }
}
