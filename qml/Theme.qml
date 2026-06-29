pragma Singleton
import QtQuick

QtObject {
    readonly property color bgGradientStart: "#EEF8F1"
    readonly property color bgGradientEnd: "#D9F0E4"
    readonly property color horizonGlow: "#FFF7E6"
    readonly property color fieldGreen: "#A6D4A6"
    readonly property color fieldGreenDeep: "#79AF86"
    readonly property color fieldGold: "#EBCB8F"

    readonly property color primaryGreen: "#3E7C60"
    readonly property color primaryGreenLight: "#5C9B79"
    readonly property color primaryGreenHover: "#72B28E"
    readonly property color secondaryBlue: "#85ADC4"
    readonly property color mint: "#A9D9BC"
    readonly property color terracotta: "#D58D72"
    readonly property color terracottaLight: "#E5A58E"

    readonly property color warning: "#D1A24F"
    readonly property color danger: "#C96B59"
    readonly property color locked: "#9DA79C"
    readonly property color starGold: "#EDBC5C"
    readonly property color success: "#428D6B"

    readonly property color textPrimary: "#243C31"
    readonly property color textSecondary: "#648171"
    readonly property color textDisabled: "#9EB1A6"
    readonly property color textOnDark: "#F6FBF7"
    readonly property color textMutedOnDark: "#BFD2C3"

    readonly property color cardBg: "#FFFEFB"
    readonly property color windowBg: "#F5FBF6"
    readonly property color surfaceSoft: "#F2F8F3"
    readonly property color surfaceRaised: "#FFFFFF"
    readonly property color surfaceStrong: "#EEF6F0"
    readonly property color surfaceStrongRaised: "#F9FCFA"
    readonly property color titleBarStart: "#F7FBF8"
    readonly property color titleBarEnd: "#EBF4EE"
    readonly property color codeBg: "#15211B"
    readonly property color codePanel: "#203028"
    readonly property color codeText: "#EDF6EF"
    readonly property color chromeBg: Qt.rgba(0.98, 0.995, 0.985, 0.96)
    readonly property color chromeBorder: Qt.rgba(0.20, 0.28, 0.21, 0.10)
    readonly property color shellBorder: Qt.rgba(0.20, 0.28, 0.21, 0.12)
    readonly property color panelHighlight: Qt.rgba(1.0, 1.0, 1.0, 0.66)

    readonly property color border: Qt.rgba(0.17, 0.23, 0.15, 0.10)
    readonly property color borderHover: Qt.rgba(0.24, 0.46, 0.34, 0.24)
    readonly property color borderFocused: Qt.rgba(0.24, 0.46, 0.34, 0.34)
    readonly property color borderStrong: Qt.rgba(0.17, 0.23, 0.15, 0.16)
    readonly property color panelShadow: Qt.rgba(0.17, 0.22, 0.16, 0.10)
    readonly property color overlayDark: Qt.rgba(0.12, 0.16, 0.12, 0.42)

    readonly property int radius: 18
    readonly property int radiusLarge: 24
    readonly property int radiusPill: 26
    readonly property int radiusSmall: 10

    readonly property int btnWidth: 236
    readonly property int btnHeight: 50

    readonly property string fontUI: "Nunito"
    readonly property string fontCode: "JetBrains Mono"
}
