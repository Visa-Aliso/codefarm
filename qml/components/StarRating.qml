pragma ComponentBehavior: Bound
import QtQuick
import CodeFarm

Row {
    id: starsRow
    property int count: 0
    spacing: 6

    Repeater {
        model: 3

        Rectangle {
            id: starBadge
            required property int index
            width: 26
            height: 26
            radius: 13
            color: starBadge.index < starsRow.count
                   ? Qt.rgba(Theme.starGold.r, Theme.starGold.g, Theme.starGold.b, 0.20)
                   : Qt.rgba(0.30, 0.34, 0.28, 0.08)
            border.width: 1
            border.color: starBadge.index < starsRow.count
                          ? Qt.rgba(Theme.starGold.r, Theme.starGold.g, Theme.starGold.b, 0.42)
                          : Theme.border

            Text {
                anchors.centerIn: parent
                text: "✦"
                font.pixelSize: 13
                color: starBadge.index < starsRow.count ? Theme.starGold : Theme.textDisabled
            }
        }
    }
}
